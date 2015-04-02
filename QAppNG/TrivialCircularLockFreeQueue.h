/** =================================================================================================================
* @file    TrivialCircularLockFreeQueue.h
*
* @brief   lock free, wait free circular queue that can be used by 1 producer thread and 1 consumer thread
*
* @copyright
*
* @history
* REF#        Who                                                              When          What
* #????       A. Della Villa                                                   31/10/2008    Original development
* #8060       A. Della Villa, D. Verna, F. Lasagni                             Feb-2013      minor improvements
*
* @endhistory
* ===================================================================================================================
*/
#ifndef INCLUDE_TRIVIALCIRCULARLOCKFREEQUEUE_NG
#define INCLUDE_TRIVIALCIRCULARLOCKFREEQUEUE_NG

// Other Includes
#include <QAppNG/core.h>


#include <memory>

namespace QAppNG
{
    // forward declaration
    template<class ELEMENT_CLASS> class TrivialCircularLockFreeQueue;

    // --------------------------------------------------------------------------------------------------------
    /**
    *  @author Alessandro Della Villa <alessandro.dellavilla@CommProve.com>
    *
    *  @brief Simple circular lock free queue with FIFO policy. It is fast and it is possible to have
    *         one thread pushing elements and one thread popping them. Usage:
    *
    *         TrivialCircularLockFreeQueue< std::shared_ptr<std::string> > queue(400);
    *
    *         queue.push( std::shared_ptr<std::string>(new string("Hello Queue!")) );
    *
    *         std::shared_ptr<std::string> got_string = queue.pop();
    */
    // shared pointer specialization
    template<class ELEMENT_CLASS >
    class TrivialCircularLockFreeQueue< std::shared_ptr< ELEMENT_CLASS > >
    {
    public:
        // const definition
        static const UInt16 USEC_TO_SLEEP_IF_FULL = 1;

    public:
        // CTOR
        TrivialCircularLockFreeQueue( size_t queue_size ) 
            : m_queue_size( queue_size)
            , m_queue_last_index( queue_size - 1 )
            , m_queue_end_index( queue_size )
        {
            // init read/write cursors
            m_read_index = m_write_index = 0;

            // create data array: we create a m_queue_size+1 long array to be able to handle read/write pointers
            m_data_array = std::vector< std::shared_ptr< ELEMENT_CLASS > >( m_queue_size + 1, std::shared_ptr< ELEMENT_CLASS >() );
        }

        // DTOR
        virtual ~TrivialCircularLockFreeQueue()
        {
        }

        // Check if queue is EMPTY
        // TICKET #8060: redefined in Sorted Queue version
        virtual bool empty() { return m_read_index == m_write_index; };

        // Check if queue is FULL
        bool full()
        {
            if ( m_read_index == 0 && m_write_index == m_queue_last_index )
            {
                return true;
            }
            else
            {
                return m_write_index == m_read_index - 1;
            }
        }

        size_t getUsedQueue()
        {
            //For thread safety we need to store index_diff_plus_queue_size in a local variable and refer to that for further comparison
            size_t index_diff_plus_queue_size = static_cast<size_t>( m_queue_size + m_write_index - m_read_index );

            if ( index_diff_plus_queue_size == m_queue_size )
            {
                return 0;
            }
            else if (index_diff_plus_queue_size > m_queue_size)
            {
                return static_cast< size_t >(index_diff_plus_queue_size - m_queue_size);
            }
            else if (index_diff_plus_queue_size < m_queue_size)
            {
                return static_cast< size_t >(index_diff_plus_queue_size);
            }
            /*
            THESE COMPARISON ARE NOT GUARANTEED ATOMIC
            one thread could modify a index while a second one is using it
            
            if ( m_read_index == m_write_index )
            {
                return 0;
            }
            else if ( m_write_index > m_read_index )
            {
                return static_cast< size_t >(m_write_index - m_read_index);
            }
            else if ( m_write_index < m_read_index )
            {
                return static_cast< size_t >(m_queue_size + m_write_index - m_read_index);
            }

            */
            else
            {
                //THIS SHOULD NEVER HAPPEN!If it does check concurrency!
                throw "Error in CircularLockFreeQueue: cannot evaluate Queue Usage!";
            }
        }

        size_t getCurrentCapacity()
        {
            return ( m_queue_size - getUsedQueue() );
        }

        float getUsedQueuePercent()
        {
            //For thread safety we need to store index_diff_plus_queue_size in a local variable and refer to that for further comparison
            size_t index_diff_plus_queue_size = static_cast< size_t >( m_queue_size + m_write_index - m_read_index );

            if ( index_diff_plus_queue_size == m_queue_size )
            {
                return 0.0;
            }
            else if ( index_diff_plus_queue_size > m_queue_size )
            {
                return ( index_diff_plus_queue_size - m_queue_size ) * 100 / static_cast< float >( m_queue_size );
            }
            else if ( index_diff_plus_queue_size < m_queue_size )
            {
                return index_diff_plus_queue_size * 100 / static_cast< float >( m_queue_size );
            }
            /*
            THESE COMPARISON ARE NOT GUARANTEED ATOMIC
            one thread could modify a index while a second one is using it
            if ( m_read_index == m_write_index )
            {
                return 0.0;
            }
            else if ( m_write_index > m_read_index )
            {
                return ( m_write_index - m_read_index ) * 100 / static_cast< float >( m_queue_size );
            }
            else if ( m_write_index < m_read_index )
            {
                return ( m_queue_size + m_write_index - m_read_index ) * 100 / static_cast< float >( m_queue_size );
            }
            */
            else
            {
                throw "Error in CircularLockFreeQueue: cannot evaluate Queue Usage Percent!";
            }
        }

        size_t getQueueSize() { return m_queue_size; }

        // PUSH in Queue
        void push(const std::shared_ptr< ELEMENT_CLASS >& element)
        {
            // check if the queue is full. If queue is full wait for an empty space
            while ( full() )
            {
                std::this_thread::sleep_for(std::chrono::nanoseconds(static_cast<int>(USEC_TO_SLEEP_IF_FULL)));
            }

            // COPY element in m_data_array
            m_data_array[ static_cast< size_t >( m_write_index ) ] = element;

            if ( ++m_write_index == m_queue_end_index )
            {
                m_write_index = 0;
            }
        }

        // POP from queue
        void pop()
        {
            if ( empty() ) return;

            // release shared pointer
            m_data_array[ static_cast< size_t >( m_read_index) ].reset();

            // to remove front element it is enough to move read index
            if ( ++m_read_index == m_queue_end_index )
            {
                m_read_index = 0;
            }
        }

        std::shared_ptr< ELEMENT_CLASS >& front()
        {
            return m_data_array[ static_cast< size_t >( m_read_index) ];
        }

    protected:
        std::vector< std::shared_ptr< ELEMENT_CLASS > >  m_data_array;
        size_t                      m_queue_size;
        UInt64                      m_read_index;
        UInt64                      m_write_index;
        UInt64                      m_queue_last_index;
        UInt64                      m_queue_end_index;
    };

} // namespace

// --------------------------------------------------------------------------------------------------------
#endif // INCLUDE_TRIVIALCIRCULARLOCKFREEQUEUE_NG


