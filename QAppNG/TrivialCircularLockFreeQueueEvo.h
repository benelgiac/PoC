#pragma once
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

// Other Includes
#include "core.h"
#include <thread>
#include <chrono>
#include <vector>
#include <tuple>

namespace QAppNG
{
    class TrivialCircularLockFreeQueueBase
    {
    protected:
        // const definition
        static const UInt16 NSEC_TO_SLEEP_IF_FULL;

    public:
        // CTOR
        TrivialCircularLockFreeQueueBase( size_t queue_size )
            : m_queue_size( queue_size )
            , m_queue_last_index( queue_size - 1 )
            , m_queue_end_index( queue_size )
            , m_read_index( 0 )
            , m_write_index( 0 )
        {
            // NB: m_data_array size is QUEUE_SIZE+1 to be able to handle read/write pointers
        }

        // DTOR
        virtual ~TrivialCircularLockFreeQueueBase()
        {
        }

        // Check if queue is EMPTY (redefined in Sorted Queue version)
        virtual bool empty() const
        {
            return m_read_index == m_write_index;
        };

        // Check if queue is FULL
        bool full() const
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

        size_t getUsedQueue() const
        {
            // For thread safety we need to PRECALCULATE and STORE index_diff_plus_queue_size in a local variable
            // since other thread using queue can intervene between "if" and "else if" and modify value of index itself!!!
            size_t index_diff_plus_queue_size = static_cast<size_t>(m_queue_size + m_write_index - m_read_index);

            if ( index_diff_plus_queue_size == m_queue_size )
            {
                return 0;
            }
            else if ( index_diff_plus_queue_size > m_queue_size )
            {
                return static_cast< size_t >(index_diff_plus_queue_size - m_queue_size);
            }
            else if ( index_diff_plus_queue_size < m_queue_size )
            {
                return static_cast< size_t >(index_diff_plus_queue_size);
            }
            else
            {
                //THIS SHOULD NEVER HAPPEN!If it does check concurrency!
                throw "Error in CircularLockFreeQueue: cannot evaluate Queue Usage!";
            }
        }

        size_t getCurrentCapacity() const
        {
            return (m_queue_size - getUsedQueue());
        }

        float getUsedQueuePercent() const
        {
            // For thread safety we need to PRECALCULATE and STORE index_diff_plus_queue_size in a local variable
            // since other thread using queue can intervene between "if" and "else if" and modify value of index itself!!!
            size_t index_diff_plus_queue_size = static_cast< size_t >(m_queue_size + m_write_index - m_read_index);

            if ( index_diff_plus_queue_size == m_queue_size )
            {
                return 0.0;
            }
            else if ( index_diff_plus_queue_size > m_queue_size )
            {
                return (index_diff_plus_queue_size - m_queue_size) * 100 / static_cast< float >(m_queue_size);
            }
            else if ( index_diff_plus_queue_size < m_queue_size )
            {
                return index_diff_plus_queue_size * 100 / static_cast< float >(m_queue_size);
            }
            else
            {
                throw "Error in CircularLockFreeQueue: cannot evaluate Queue Usage Percent!";
            }
        }

        size_t getQueueSize() const
        {
            return m_queue_size;
        }

    protected:
        size_t                                        m_queue_size;
        UInt64                                        m_queue_last_index;
        UInt64                                        m_queue_end_index;
        UInt64                                        m_read_index;
        UInt64                                        m_write_index;

        // Helper Functions
        UInt64      getLastQueueIndex() const
        {
            return m_queue_last_index;
        }

        UInt64      getQueueEndIndex() const
        {
            return m_queue_end_index;
        }

        UInt64      getQueueReadIndex() const
        {
            return m_read_index;
        }

        UInt64      getQueueWriteIndex() const
        {
            return m_write_index;
        }

        UInt64&      getQueueReadIndex()
        {
            return m_read_index;
        }

        UInt64&      getQueueWriteIndex()
        {
            return m_write_index;
        }
    };

    // --------------------------------------------------------------------------------------------------------
    /**
    *  @author Alessandro Della Villa <alessandro.dellavilla@CommProve.com>
    *          Davide Verna <davide.verna@CommProve.com>
    *
    *  @brief Simple circular lock free queue with FIFO policy. It is fast and admit
    *         one thread pushing elements and one thread popping them. Usage:
    *
    *         auto queue = TrivialCircularLockFreeQueue< std::shared_ptr<std::string> >(400);
    *
    *         queue.push( std::shared_ptr<std::string>(new string("Hello Queue!")) );
    *
    *         auto my_string = queue.front();
    *
    *         queue.pop();
    */

    /**  ----------------------------------------------------------------------------------------------
    *  Generic Implementation
    *  ------------------------------------------------------------------------------------------------
    */
    template<typename ENQUEUED_TYPE>
    class TrivialCircularLockFreeQueueEvo
        : public TrivialCircularLockFreeQueueBase
    {
    public:
        // CTOR
        TrivialCircularLockFreeQueueEvo( size_t queue_size )
            : TrivialCircularLockFreeQueueBase( queue_size )
            , m_data_array( queue_size + 1 )
        {
            // NB: m_data_array size is QUEUE_SIZE+1 to be able to handle read/write pointers
        }

        // PUSH in Queue -> a copy is inserted into queue
        void push( ENQUEUED_TYPE& element )
        {
            auto element_copy = element;

            push( std::move( element_copy ) );
        }

        // PUSH in Queue using C++11 MOVABILITY
        void push( ENQUEUED_TYPE&& element )
        {
            // check if the queue is full. If queue is full wait for an empty space
            while ( full() )
            {
                std::this_thread::sleep_for( std::chrono::nanoseconds( NSEC_TO_SLEEP_IF_FULL ) );
            }

            // MOVE element in m_data_array
            m_data_array[static_cast<size_t>( this->getQueueWriteIndex() )] = std::move( element );

            if ( ++this->getQueueWriteIndex() == this->getQueueEndIndex() )
            {
                this->getQueueWriteIndex() = 0;
            }
        }

        // POP from queue
        void pop()
        {
            if ( empty() ) return;

            // release shared pointer
            m_data_array[static_cast< size_t >(this->getQueueReadIndex())].reset();

            // to remove front element it is enough to move read index
            if ( ++this->getQueueReadIndex() == this->getQueueEndIndex() )
            {
                this->getQueueReadIndex() = 0;
            }
        }

        // ref to FRONT element
        ENQUEUED_TYPE& front()
        {
            return m_data_array.at( static_cast< size_t >( this->getQueueReadIndex() ) );
        }

        // ref to BACK element
        ENQUEUED_TYPE& back()
        {
            return m_data_array.at(static_cast<size_t>(this->getQueueWriteIndex()));
        }

    protected:
        // ref to element at index_to_retrieve position
        ENQUEUED_TYPE& at(size_t index_to_retrieve)
        {
            return m_data_array.at( index_to_retrieve );
        }

    private:
        std::vector< ENQUEUED_TYPE > m_data_array;

    }; // END of Generic Implementation

    /**  ----------------------------------------------------------------------------------------------
    *  Specialization for TUPLE containing an ENQUEUED_TYPE ( used in LWS EVO )
    *  ------------------------------------------------------------------------------------------------
    */
    template<class ENQUEUED_TYPE>
    class TrivialCircularLockFreeQueueEvo< std::tuple< UInt64, UInt8, ENQUEUED_TYPE > > : public TrivialCircularLockFreeQueueBase
    {
    public:
        // CTOR
        TrivialCircularLockFreeQueueEvo( size_t queue_size )
            : TrivialCircularLockFreeQueueBase( queue_size )
            , m_data_array( queue_size + 1 )
        {
            // NB: m_data_array size is QUEUE_SIZE+1 to be able to handle read/write pointers
        }

        // PUSH in Queue using C++11 MOVABILITY
        void push( std::tuple< UInt64, UInt8, ENQUEUED_TYPE >&& element )
        {
            // check if the queue is full. If queue is full wait for an empty space
            while ( full() )
            {
                std::this_thread::sleep_for( std::chrono::nanoseconds( NSEC_TO_SLEEP_IF_FULL ) );
            }

            // MOVE element in m_data_array
            m_data_array[static_cast<size_t>(this->getQueueWriteIndex())] = std::move( element );

            if ( ++this->getQueueWriteIndex() == this->getQueueEndIndex() )
            {
                this->getQueueWriteIndex() = 0;
            }
        }

        // PUSH ONLY ENQUEUED_TYPE in Queue using C++11 MOVABILITY
        void push( ENQUEUED_TYPE&& element )
        {
            // check if the queue is full. If queue is full wait for an empty space
            while ( full() )
            {
                std::this_thread::sleep_for( std::chrono::nanoseconds( NSEC_TO_SLEEP_IF_FULL ) );
            }

            // MOVE element in m_data_array
            std::get<2>( m_data_array[static_cast<size_t>(this->getQueueWriteIndex())] ) = std::move( element );

            if ( ++this->getQueueWriteIndex() == this->getQueueEndIndex() )
            {
                this->getQueueWriteIndex() = 0;
            }
        }

        // POP from queue
        void pop()
        {
            if ( empty() ) return;

            // release shared pointer
            std::get<2>( m_data_array[static_cast<size_t>(this->getQueueReadIndex())] ).reset();

            // to remove front element it is enough to move read index
            if ( ++this->getQueueReadIndex() == this->getQueueEndIndex() )
            {
                this->getQueueReadIndex() = 0;
            }
        }

        // ref to FRONT element
        std::tuple< UInt64, UInt8, ENQUEUED_TYPE >& front()
        {
            return m_data_array.at( static_cast< size_t >( this->getQueueReadIndex() ) );
        }

        // ref to BACK element
        std::tuple< UInt64, UInt8, ENQUEUED_TYPE >& back()
        {
            return m_data_array.at(static_cast<size_t>(this->getQueueWriteIndex()));
        }

    protected:
        // ref to element at index_to_retrieve position
        std::tuple< UInt64, UInt8, ENQUEUED_TYPE >& at(size_t index_to_retrieve)
        {
            return m_data_array.at( index_to_retrieve );
        }

    private:
        std::vector< std::tuple< UInt64, UInt8, ENQUEUED_TYPE > > m_data_array;

    }; // END Specialization for TUPLE containing a UNIQUE pointer

} // namespace

// --------------------------------------------------------------------------------------------------------
