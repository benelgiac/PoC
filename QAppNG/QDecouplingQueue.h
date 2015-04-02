/** ===================================================================================================================
  * @file    QDecoplingQueue HEADER FILE
  *
  * @brief   A decuopling lock free queue with one dispatcher thread
  *
  * @copyright
  *
  * @history
  * REF#        Who                                                              When          What
  * #9263       D. Verna                                                        Jan-2014      Original development
  *
  * @endhistory
  * ===================================================================================================================
*/
// ----------------------------------------------------------------------------------------------
#ifndef Q_DECOUPLING_QUEUE_H
#define Q_DECOUPLING_QUEUE_H
// ----------------------------------------------------------------------------------------------

#include <QAppNG/core.h>
#include <QAppNG/TrivialCircularLockFreeQueue.h>

// ----------------------------------------------------------------------------------------------

#include <cal/Thread.h>         // #9263
#include <cal/ThreadManager.h>  // #9263
#include <cal/Delegate.h>       // #9263

// ----------------------------------------------------------------------------------------------

namespace QAppNG
{
    template< typename ELEMENT_TYPE, typename CONSUMER_THREAD_MODEL = cal::Thread >
    class QDecouplingQueue;

    template< typename ELEMENT_TYPE >
    class QDecouplingQueue< ELEMENT_TYPE, cal::Thread > : public TrivialCircularLockFreeQueue< ELEMENT_TYPE >
    {
    public:
        QDecouplingQueue( size_t queue_size, fastdelegate::FastDelegate1< ELEMENT_TYPE&, void > dispatch_delegate
            , fastdelegate::FastDelegate0< void > initialize_delegate
                = fastdelegate::FastDelegate0< void >() 
            , fastdelegate::FastDelegate0< void > termination_delegate
                = fastdelegate::FastDelegate0< void >() )
                : TrivialCircularLockFreeQueue< ELEMENT_TYPE >( queue_size )
                , m_exit(false)
                , m_flushed(false)
                , m_dispatch_delegate(dispatch_delegate)
                , m_initialize_dalegate(initialize_delegate)
                , m_termination_delegate(termination_delegate)
        {
            assert( m_dispatch_delegate );

            // set thread attr
            cal::ThreadAttr attr;
            attr.m_prio = cal::ThreadAttr::Pri_AboveNormal;

            // set thread data (at the moment no thread_data is used)
            void * thread_data(NULL);

            // START Disparcher Thread
            cal::ThreadManager::Instance().StartThread( "QDecoplingQueueDispatcher", attr, static_cast<void*>( thread_data )
                , MakeDelegate( this, &QDecouplingQueue< ELEMENT_TYPE, cal::Thread >::dispatcherLoopThread ) );
        }

        virtual ~QDecouplingQueue()
        {
        }

        void flush()
        {
            m_exit = true;

            while ( !m_flushed )
            {
                cal::Thread::RunOnce( cal::time::Duration< MilliRes >( 1 ) );
            }
        }

    private:
        Int32 dispatcherLoopThread( void* )
        {
            static UInt32   dispatched_elements(0);

            if ( m_initialize_dalegate )
            {
                m_initialize_dalegate();
            }

            while ( !m_exit )
            {
                if ( !TrivialCircularLockFreeQueue< ELEMENT_TYPE >::empty() && dispatched_elements < 9999 )
                {
                    m_dispatch_delegate( TrivialCircularLockFreeQueue< ELEMENT_TYPE >::front() );

                    TrivialCircularLockFreeQueue< ELEMENT_TYPE >::pop();

                    ++dispatched_elements;
                }
                else
                {
                    dispatched_elements = 0;

                    cal::Thread::RunOnce( cal::time::Duration< MilliRes >( 1 ) );
                }
            }

            // Flush queue at shutdown
            while ( !TrivialCircularLockFreeQueue< ELEMENT_TYPE >::empty() )
            {
                m_dispatch_delegate( TrivialCircularLockFreeQueue< ELEMENT_TYPE >::front() );

                TrivialCircularLockFreeQueue< ELEMENT_TYPE >::pop();

                ++dispatched_elements;
            }

            // call termintation delegate if any
            if (m_termination_delegate)
            {
                m_termination_delegate();
            }

            // shutdown complete
            m_flushed = true;

            return 0;
        }

        volatile bool m_exit;
        volatile bool m_flushed;
        fastdelegate::FastDelegate1< ELEMENT_TYPE&, void >       m_dispatch_delegate;
        fastdelegate::FastDelegate0< void >                      m_initialize_dalegate;
        fastdelegate::FastDelegate0< void >                      m_termination_delegate;
    };

    template< typename ELEMENT_TYPE >
    class QDecouplingQueue< ELEMENT_TYPE, std::thread > : public TrivialCircularLockFreeQueue< ELEMENT_TYPE >
    {
    public:
        QDecouplingQueue( size_t queue_size, fastdelegate::FastDelegate1< ELEMENT_TYPE&, void > dispatch_delegate
            , fastdelegate::FastDelegate0< void > initialize_delegate
                = fastdelegate::FastDelegate0< void >()
            , fastdelegate::FastDelegate0< void > termination_delegate
                = fastdelegate::FastDelegate0< void >() )
            : TrivialCircularLockFreeQueue< ELEMENT_TYPE >( queue_size )
            , m_exit(false)
            , m_dispatch_delegate(dispatch_delegate)
            , m_initialize_dalegate(initialize_delegate)
            , m_termination_delegate(termination_delegate)
        {
            assert( m_dispatch_delegate );

            // START Disparcher Thread
            m_dispatcher_thread.reset( new std::thread( [this] { this->dispatcherLoopThread(); } ) );
        }

        virtual ~QDecouplingQueue()
        {
        }

        void flush()
        {
            m_exit = true;

            m_dispatcher_thread->join();
        }

    private:
        void dispatcherLoopThread()
        {
            static UInt32   dispatched_elements(0);

            if ( m_initialize_dalegate )
            {
                m_initialize_dalegate();
            }

            while ( !m_exit )
            {
                if ( !TrivialCircularLockFreeQueue< ELEMENT_TYPE >::empty() && dispatched_elements < 9999 )
                {
                    m_dispatch_delegate( TrivialCircularLockFreeQueue< ELEMENT_TYPE >::front() );

                    TrivialCircularLockFreeQueue< ELEMENT_TYPE >::pop();

                    ++dispatched_elements;
                }
                else
                {
                    dispatched_elements = 0;

                    std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
                }
            }

            // Flush queue at shutdown
            while ( !TrivialCircularLockFreeQueue< ELEMENT_TYPE >::empty() )
            {
                m_dispatch_delegate( TrivialCircularLockFreeQueue< ELEMENT_TYPE >::front() );

                TrivialCircularLockFreeQueue< ELEMENT_TYPE >::pop();

                ++dispatched_elements;
            }

            // call termintation delegate if any
            if ( m_termination_delegate )
            {
                m_termination_delegate();
            }
        }

        volatile bool m_exit;
        std::unique_ptr< std::thread >                          m_dispatcher_thread;
        fastdelegate::FastDelegate1< ELEMENT_TYPE&, void >      m_dispatch_delegate;
        fastdelegate::FastDelegate0< void >                     m_initialize_dalegate;
        fastdelegate::FastDelegate0< void >                     m_termination_delegate;
    };
}

// ----------------------------------------------------------------------------------------------
#endif
// ----------------------------------------------------------------------------------------------
// End of file
