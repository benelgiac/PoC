/** =================================================================================================================
* @file    TrivialCircularLockFreeQueue.h
*
* @brief   lock free, wait free circular queue that can be used by 1 producer thread and 1 consumer thread
*
* @copyright
*
* @history
* REF#        Who                                                              When          What
* #8060       A. Della Villa, D. Verna, F. Lasagni                             Feb-2013      Original development
*
* @endhistory
* ===================================================================================================================
*/
#ifndef INCLUDE_TRIVIALCIRCULARLOCKFREEQUEUESORTED_NG
#define INCLUDE_TRIVIALCIRCULARLOCKFREEQUEUESORTED_NG

// Other Includes
#include "core.h"
#include "TrivialCircularLockFreeQueue.h"

#include <memory>

#include <iterator>
#include <algorithm>
#include <functional>

namespace QAppNG
{
    // forward declaration
    template<class ELEMENT_CLASS> class TrivialCircularLockFreeQueueSorted;

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
    template<class ELEMENT_CLASS>
    class TrivialCircularLockFreeQueueSorted< std::shared_ptr< ELEMENT_CLASS > > : public QAppNG::TrivialCircularLockFreeQueue< std::shared_ptr< ELEMENT_CLASS > >
    {
    public:

        // iterator
        class iterator;
        friend class iterator;
        iterator begin() { return iterator( *this, 0 ); }
        iterator end() { return iterator( *this, this->m_queue_size ); }

    public:
        // CTOR
        TrivialCircularLockFreeQueueSorted( size_t queue_size, UInt8 max_disorder_percent)
                : QAppNG::TrivialCircularLockFreeQueue< std::shared_ptr< ELEMENT_CLASS > >(queue_size)
                , m_sort_index(0)
                , m_max_disorder_size( static_cast< size_t >( max_disorder_percent / 100.0 * queue_size ) )
        {}

        // DTOR
        virtual ~TrivialCircularLockFreeQueueSorted()
        {
        }

        // Check if queue is EMPTY
        virtual bool empty() { return this->m_read_index == m_sort_index; }

        // partial sorting of queue from m_sort_index to m_write_index of max_sorting_size elements
        void partial_sort()
        {

            size_t max_sorting_size = getUnSortedQueue() - m_max_disorder_size;

            // any element to sort
            if ( max_sorting_size > 0 )
            {

                // calculate sorting bounds

                iterator start_iterator = begin();
                iterator start_sorting_iterator = start_iterator + static_cast<size_t>( m_sort_index );
                iterator middle_sorting_iterator = start_sorting_iterator + max_sorting_size;
                iterator end_sorting_iterator = start_iterator + static_cast<size_t>( this->m_write_index );

                // partial sort
                std::partial_sort( start_sorting_iterator, middle_sorting_iterator, end_sorting_iterator, &compareElementClass );

                // update sort index taking care of rotate it if needed
                if ( m_sort_index + max_sorting_size >= this->m_queue_size )
                {
                    // rotate
                    m_sort_index = m_sort_index + max_sorting_size - this->m_queue_size;
                }
                else
                {
                    // increment
                    m_sort_index += max_sorting_size;
                }

            }
        }

        // total sort of queue from m_sort_index to m_write_index
        void sort()
        {
            // any element to sort
            if ( getUnSortedQueue() > 0 )
            {
                // calculate sorting bounds
                iterator start_iterator = begin();
                iterator start_sorting_iterator = start_iterator + static_cast<size_t>( m_sort_index );
                iterator end_sorting_iterator = start_iterator + static_cast<size_t>( this->m_write_index );

                // partial sort
                std::sort( start_sorting_iterator, end_sorting_iterator, compareElementClass );

                // update sort index
                m_sort_index = this->m_write_index;
            }
        }

        size_t getUnSortedQueue()
        {
            if ( m_sort_index == this->m_write_index)
            {
                return 0;
            }
            else if ( this->m_write_index > m_sort_index  )
            {
                return static_cast< size_t >( this->m_write_index - m_sort_index );
            }
            else if ( this->m_write_index < m_sort_index )
            {
                return static_cast< size_t >( this->m_queue_size + this->m_write_index - m_sort_index );
            }
            else
            {
                throw "Error in CircularLockFreeQueue: cannot evaluate Queue UnSorted!";
            }
        }

        size_t getSortedQueue()
        {   
            if ( m_sort_index == this->m_read_index )
            {
                return 0;
            }
            else if ( this->m_read_index < m_sort_index  )
            {
                return static_cast< size_t >( m_sort_index - this->m_read_index );
            }
            else if ( this->m_read_index > m_sort_index )
            {
                return static_cast< size_t >( this->m_queue_size + m_sort_index - this->m_read_index );
            }
            else
            {
            throw "Error in CircularLockFreeQueue: cannot evaluate Queue UnSorted!";
            }
        }

        float getSortedQueuePercent()
        {
            return ( getSortedQueue() * 100 / static_cast< float >( this->getQueueSize() ) );
        }

        float getUnSortedQueuePercent()
        {
            return ( getUnSortedQueue() * 100 / static_cast< float >( this->getQueueSize() ) );
        }

    protected:

        static bool compareElementClass( const std::shared_ptr<ELEMENT_CLASS>& element_1
                                       , const std::shared_ptr<ELEMENT_CLASS>& element_2 )
        {
            return *element_1 < *element_2;
        }

        UInt64                      m_sort_index;
        size_t                      m_max_disorder_size;
    };

    // --------------------------------------------------------------------------------------------------------

    template<typename ELEMENT_CLASS>
    class TrivialCircularLockFreeQueueSorted< std::shared_ptr< ELEMENT_CLASS > >::iterator 
        : public std::iterator< std::random_access_iterator_tag, std::shared_ptr< ELEMENT_CLASS >
                              , size_t, std::shared_ptr< ELEMENT_CLASS >*, std::shared_ptr< ELEMENT_CLASS >& >
    {
    public:
        // CTOR
        iterator( TrivialCircularLockFreeQueueSorted< std::shared_ptr< ELEMENT_CLASS > >& circular_lock_free_queue, size_t index_position = 0 )
            : m_circular_lock_free_queue( circular_lock_free_queue ) 
            , m_index_position(index_position)
            , m_last_valid_index_position( static_cast< size_t >( circular_lock_free_queue.m_queue_last_index ) )
            , m_container_size( circular_lock_free_queue.m_queue_size )
            , m_container_sort_index( static_cast< size_t >( circular_lock_free_queue.m_sort_index ) )
        {}

        // COPY CTOR
        iterator( const iterator & source_iterator )
            : m_circular_lock_free_queue( source_iterator.m_circular_lock_free_queue )
            , m_index_position( source_iterator.m_index_position )
            , m_last_valid_index_position( source_iterator.m_last_valid_index_position )
            , m_container_size( source_iterator.m_container_size )
            , m_container_sort_index( source_iterator.m_container_sort_index )
        {}

        inline iterator& operator=( const iterator& source_iterator )
        {
            if ( &source_iterator == this )
            {
                return *this;
            }

            m_circular_lock_free_queue = source_iterator.m_circular_lock_free_queue;
            m_index_position = source_iterator.m_index_position;
            m_last_valid_index_position = source_iterator.m_last_valid_index_position;
            m_container_size = source_iterator.m_container_size;
            m_container_sort_index = source_iterator.m_container_sort_index;

            return *this;
        }

        // dereferencing and arrow operators
        inline std::shared_ptr< ELEMENT_CLASS >& operator*() const { return m_circular_lock_free_queue.m_data_array[m_index_position]; }
        inline std::shared_ptr< ELEMENT_CLASS >* operator->() const { return &(m_circular_lock_free_queue.m_data_array[m_index_position]); }

        // arithmetic operators: increment & decrement
        inline iterator& operator++() { incrementIteratorIndex(); return *this; }
        inline iterator& operator--() { decrementIteratorIndex(); return *this; }
        inline iterator operator++(int) { iterator temp_iterator(*this); incrementIteratorIndex(); return temp_iterator; }
        inline iterator operator--(int) { iterator temp_iterator(*this); decrementIteratorIndex(); return temp_iterator; }

        typedef typename TrivialCircularLockFreeQueueSorted< std::shared_ptr< ELEMENT_CLASS > >::iterator TrivialCircularLockFreeQueueSortedIterator;

        // comparison operators
        friend inline bool operator==( const TrivialCircularLockFreeQueueSortedIterator& compared_iterator_1
                                     , const TrivialCircularLockFreeQueueSortedIterator& compared_iterator_2 )
        {
            return compared_iterator_1.m_index_position == compared_iterator_2.m_index_position;
        }

        friend inline bool operator!=( const TrivialCircularLockFreeQueueSortedIterator& compared_iterator_1
                                     , const TrivialCircularLockFreeQueueSortedIterator& compared_iterator_2 )
        {
            return compared_iterator_1.m_index_position != compared_iterator_2.m_index_position;
        }

        friend inline bool operator<( const TrivialCircularLockFreeQueueSortedIterator& compared_iterator_1
                                    , const TrivialCircularLockFreeQueueSortedIterator& compared_iterator_2 )
        {
            assert(compared_iterator_1.m_container_sort_index == compared_iterator_2.m_container_sort_index);

            if ( compared_iterator_1.m_index_position >= compared_iterator_1.m_container_sort_index && compared_iterator_2.m_index_position >= compared_iterator_1.m_container_sort_index )
            {
                return compared_iterator_1.m_index_position < compared_iterator_2.m_index_position;
            }
            else if ( compared_iterator_1.m_index_position >= compared_iterator_1.m_container_sort_index && compared_iterator_2.m_index_position < compared_iterator_1.m_container_sort_index )
            {
                return true;
            }
            else if ( compared_iterator_1.m_index_position < compared_iterator_1.m_container_sort_index && compared_iterator_2.m_index_position >= compared_iterator_1.m_container_sort_index )
            {
                return false;
            }
            else
            {
                return compared_iterator_1.m_index_position < compared_iterator_2.m_index_position;
            }
        }

        friend inline bool operator>( const TrivialCircularLockFreeQueueSortedIterator& compared_iterator_1
                                    , const TrivialCircularLockFreeQueueSortedIterator& compared_iterator_2 )
        {
            return ( compared_iterator_1 != compared_iterator_2 )
                    && !( compared_iterator_1 < compared_iterator_2 );
        }

        friend inline bool operator>=( const TrivialCircularLockFreeQueueSortedIterator& compared_iterator_1
                                     , const TrivialCircularLockFreeQueueSortedIterator& compared_iterator_2 )
        {
            return !( compared_iterator_1 < compared_iterator_2 );
        }
        
        friend inline bool operator<=( const TrivialCircularLockFreeQueueSortedIterator& compared_iterator_1
                                     , const TrivialCircularLockFreeQueueSortedIterator& compared_iterator_2 )
        {
            return !( compared_iterator_1 > compared_iterator_2 );
        }

        // arithmetic operators: add & subtract numeric values
        friend inline TrivialCircularLockFreeQueueSortedIterator operator+( const TrivialCircularLockFreeQueueSortedIterator & source_iterator, const size_t& numeric_value )
        {
            size_t new_index_position = source_iterator.m_index_position + numeric_value;

            while ( new_index_position >= source_iterator.m_container_size )
            {
                new_index_position -= source_iterator.m_container_size;
            }

            return TrivialCircularLockFreeQueueSortedIterator( source_iterator.m_circular_lock_free_queue, new_index_position );
        }

        friend inline TrivialCircularLockFreeQueueSortedIterator operator-( const TrivialCircularLockFreeQueueSortedIterator & source_iterator, const size_t& numeric_value )
        {
            int new_index_position = static_cast<int>( source_iterator.m_index_position ) - static_cast<int>( numeric_value );

            while ( new_index_position < 0 )
            {
                new_index_position += source_iterator.m_container_size;
            }

            return TrivialCircularLockFreeQueueSortedIterator( source_iterator.m_circular_lock_free_queue, static_cast<size_t>( new_index_position ) );
        }

        friend inline TrivialCircularLockFreeQueueSortedIterator operator+( const size_t& numeric_value, const TrivialCircularLockFreeQueueSortedIterator & source_iterator )
        {
            return source_iterator + numeric_value ;
        }

        friend inline TrivialCircularLockFreeQueueSortedIterator operator-( const size_t& numeric_value, const TrivialCircularLockFreeQueueSortedIterator & source_iterator )
        {
            return source_iterator - numeric_value;
        }

        // difference functions
        friend size_t operator+( const TrivialCircularLockFreeQueueSortedIterator& operand_iterator_1
                               , const TrivialCircularLockFreeQueueSortedIterator& operand_iterator_2 )
        {
            TrivialCircularLockFreeQueueSortedIterator tmp = operand_iterator_1 + operand_iterator_2.m_index_position;
            return tmp.m_index_position;
        }

        friend size_t operator-( const TrivialCircularLockFreeQueueSortedIterator& operand_iterator_1
                               , const TrivialCircularLockFreeQueueSortedIterator& operand_iterator_2 )
        {
            TrivialCircularLockFreeQueueSortedIterator tmp = operand_iterator_1 - operand_iterator_2.m_index_position;
            return tmp.m_index_position;
        }

    private:
        void incrementIteratorIndex()
        {
            if ( m_index_position == m_last_valid_index_position )
            {
                m_index_position = 0;
            }
            else
            {
                ++m_index_position;
            }
        }

        void decrementIteratorIndex()
        {
            if ( m_index_position == 0 )
            {
                m_index_position = m_last_valid_index_position;
            }
            else
            {
                --m_index_position;
            }
        }

    protected:
        // reference to target container
        TrivialCircularLockFreeQueueSorted< std::shared_ptr< ELEMENT_CLASS > >& m_circular_lock_free_queue;

        // indexes
        size_t m_index_position;
        size_t m_last_valid_index_position;
        size_t m_container_size;
        size_t m_container_sort_index;
    };

} // namespace

// --------------------------------------------------------------------------------------------------------
#endif // INCLUDE_TRIVIALCIRCULARLOCKFREEQUEUE_SORTED_NG


