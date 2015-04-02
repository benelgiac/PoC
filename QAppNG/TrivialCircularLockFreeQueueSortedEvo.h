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
#ifndef INCLUDE_TRIVIALCIRCULARLOCKFREEQUEUESORTED_EVO_NG
#define INCLUDE_TRIVIALCIRCULARLOCKFREEQUEUESORTED_EVO_NG

// Other Includes
#include "core.h"
#include "TrivialCircularLockFreeQueueEvo.h"

#include <boost/shared_ptr.hpp>

#include <iterator>
#include <algorithm>
#include <functional>

namespace QAppNG
{
    //-----------------------------------------------------------------------------------------------------------------------------

    /**  --------------------------------------------------------------------------------------------------------------------------
    *  Base class TrivialCircularLockFreeQueueSortedBase
    *  ----------------------------------------------------------------------------------------------------------------------------
    */
    template< typename SORTABLE_TYPE >
    class TrivialCircularLockFreeQueueSortedBase : public TrivialCircularLockFreeQueueEvo< SORTABLE_TYPE >
    {
    public:

        TrivialCircularLockFreeQueueSortedBase( size_t queue_size, UInt8 max_disorder_percent )
            : TrivialCircularLockFreeQueueEvo< SORTABLE_TYPE >( queue_size )
            , m_sort_index(0)
            , m_max_disorder_size(static_cast<size_t>( max_disorder_percent / 100.0 * queue_size ) )
        {
        }

        virtual ~TrivialCircularLockFreeQueueSortedBase()
        {
        }

        // Check if queue is EMPTY
        virtual bool empty() const
        {
            return this->getQueueReadIndex() == m_sort_index;
        }

        // Queue State Query Functions
        size_t getUnSortedQueue() const
        {
            if (m_sort_index == this->getQueueWriteIndex())
            {
                return 0;
            }
            else if (this->getQueueWriteIndex() > m_sort_index)
            {
                return static_cast<size_t>(this->getQueueWriteIndex() - m_sort_index);
            }
            else if (this->getQueueWriteIndex() < m_sort_index)
            {
                return static_cast<size_t>(this->getQueueSize() + this->getQueueWriteIndex() - m_sort_index);
            }
            else
            {
                throw "Error in CircularLockFreeQueue: cannot evaluate Queue UnSorted!";
            }
        }

        size_t getSortedQueue() const
        {
            if (m_sort_index == this->getQueueReadIndex())
            {
                return 0;
            }
            else if (this->getQueueReadIndex() < m_sort_index)
            {
                return static_cast<size_t>(m_sort_index - this->getQueueReadIndex());
            }
            else if (this->getQueueReadIndex() > m_sort_index)
            {
                return static_cast<size_t>(this->m_queue_size + m_sort_index - this->getQueueReadIndex());
            }
            else
            {
                throw "Error in CircularLockFreeQueue: cannot evaluate Queue UnSorted!";
            }
        }

        float getSortedQueuePercent() const
        {
            return (getSortedQueue() * 100 / static_cast<float>(this->getQueueSize()));
        }

        float getUnSortedQueuePercent()
        {
            return (getUnSortedQueue() * 100 / static_cast<float>(this->getQueueSize()));
        }

        // Sorting Functions
        // partial sorting of queue from m_sort_index to m_write_index of max_sorting_size elements
        virtual void partial_sort() 
        {
            size_t max_sorting_size = getUnSortedQueue() - m_max_disorder_size;

            // any element to sort
            if (max_sorting_size > 0)
            {
                // calculate sorting bounds
                iterator start_iterator = begin();
                iterator start_sorting_iterator = start_iterator + static_cast<size_t>(m_sort_index);
                iterator middle_sorting_iterator = start_sorting_iterator + max_sorting_size;
                iterator end_sorting_iterator = start_iterator + static_cast<size_t>(this->getQueueWriteIndex());

                // partial sort
                std::partial_sort(start_sorting_iterator, middle_sorting_iterator, end_sorting_iterator);

                // update sort index taking care of rotate it if needed
                if (m_sort_index + max_sorting_size >= this->getQueueSize())
                {
                    // rotate
                    m_sort_index = m_sort_index + max_sorting_size - this->getQueueSize();
                }
                else
                {
                    // increment
                    m_sort_index += max_sorting_size;
                }
            }
        }

        // total sort of queue from m_sort_index to m_write_index
        virtual void sort()
        {
            // any element to sort
            if (this->getUnSortedQueue() > 0)
            {
                // calculate sorting bounds
                iterator start_iterator = begin();
                iterator start_sorting_iterator = start_iterator + static_cast<size_t>(m_sort_index);
                iterator end_sorting_iterator = start_iterator + static_cast<size_t>(this->getQueueWriteIndex());

                //// sort
                std::sort(start_sorting_iterator, end_sorting_iterator);

                // update sort index
                this->m_sort_index = this->getQueueWriteIndex();
            }
        }

    protected:
        // Custom Version of sorting functions
        template< typename COMPARE_FUNCTION >
        void partial_sort( const COMPARE_FUNCTION& compare_function )
        {
            size_t max_sorting_size = getUnSortedQueue() - m_max_disorder_size;

            // any element to sort
            if (max_sorting_size > 0)
            {
                // calculate sorting bounds
                iterator start_iterator = begin();
                iterator start_sorting_iterator = start_iterator + static_cast<size_t>(m_sort_index);
                iterator middle_sorting_iterator = start_sorting_iterator + max_sorting_size;
                iterator end_sorting_iterator = start_iterator + static_cast<size_t>(this->getQueueWriteIndex());

                // partial sort
                std::partial_sort(start_sorting_iterator, middle_sorting_iterator, end_sorting_iterator, compare_function);

                // update sort index taking care of rotate it if needed
                if (m_sort_index + max_sorting_size >= this->getQueueSize())
                {
                    // rotate
                    m_sort_index = m_sort_index + max_sorting_size - this->getQueueSize();
                }
                else
                {
                    // increment
                    m_sort_index += max_sorting_size;
                }
            }
        }

        template< typename COMPARE_FUNCTION >
        void sort(const COMPARE_FUNCTION& compare_function)
        {
            // any element to sort
            if (this->getUnSortedQueue() > 0)
            {
                // calculate sorting bounds
                iterator start_iterator = begin();
                iterator start_sorting_iterator = start_iterator + static_cast<size_t>(m_sort_index);
                iterator end_sorting_iterator = start_iterator + static_cast<size_t>(this->getQueueWriteIndex());

                // sort
                std::sort(start_sorting_iterator, end_sorting_iterator, compare_function);

                // update sort index
                this->m_sort_index = this->getQueueWriteIndex();
            }
        }

        SORTABLE_TYPE& at( size_t index_to_retrive )
        {
            return TrivialCircularLockFreeQueueEvo< SORTABLE_TYPE >::at( index_to_retrive );
        }

        UInt64                      m_sort_index;
        size_t                      m_max_disorder_size;

    public:

        typedef SORTABLE_TYPE       value_type;

        // iterator
        class iterator;
        iterator begin() { return iterator(this, 0); }
        iterator end() { return iterator(this, this->m_queue_size); }
    };
    //-------------------------------------------------------------------------------------------------

    //-------------------------------------------------------------------------------------------------
    // End of  Base class TrivialCircularLockFreeQueueSortedBase
    //-------------------------------------------------------------------------------------------------

    //-------------------------------------------------------------------------------------------------

    /**  --------------------------------------------------------------------------------------------------------------------------
    *  Random Access Iterator to Make TrivialCircularLockFreeQueueSortedBase sortable by std::sort and std::partial_sort algorithms
    *  ----------------------------------------------------------------------------------------------------------------------------
    */
    template< typename SORTABLE_TYPE >
    class TrivialCircularLockFreeQueueSortedBase<SORTABLE_TYPE>::iterator
        : public std::iterator< std::random_access_iterator_tag, SORTABLE_TYPE, size_t >
    {
    public:
        // CTOR
        iterator( TrivialCircularLockFreeQueueSortedBase<SORTABLE_TYPE>* circular_lock_free_queue, size_t index_position = 0 )
            : m_circular_lock_free_queue( circular_lock_free_queue )
            , m_index_position(index_position)
            , m_last_valid_index_position( static_cast< size_t >( circular_lock_free_queue->m_queue_last_index ) )
            , m_container_size( circular_lock_free_queue->m_queue_size )
            , m_container_sort_index( static_cast< size_t >( circular_lock_free_queue->m_sort_index ) )
        {
        }

        // COPY CTOR
        iterator( const iterator& source_iterator )
            : m_circular_lock_free_queue( source_iterator.m_circular_lock_free_queue )
            , m_index_position( source_iterator.m_index_position )
            , m_last_valid_index_position( source_iterator.m_last_valid_index_position )
            , m_container_size( source_iterator.m_container_size )
            , m_container_sort_index( source_iterator.m_container_sort_index )
        {
        }


        // DTOR
        virtual ~iterator()
        {
        }

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
        inline SORTABLE_TYPE& operator*() const { return m_circular_lock_free_queue->at(m_index_position); }
        inline SORTABLE_TYPE* operator->() const { return &(m_circular_lock_free_queue->at( m_index_position ) ); }

        // arithmetic operators: increment & decrement
        inline iterator& operator++() { incrementIteratorIndex(); return *this; }
        inline iterator& operator--() { decrementIteratorIndex(); return *this; }
        inline iterator operator++(int) { iterator temp_iterator(*this); incrementIteratorIndex(); return temp_iterator; }
        inline iterator operator--(int) { iterator temp_iterator(*this); decrementIteratorIndex(); return temp_iterator; }

        typedef typename TrivialCircularLockFreeQueueSortedBase<SORTABLE_TYPE>::iterator TrivialCircularLockFreeQueueSortedIterator;

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
        friend inline TrivialCircularLockFreeQueueSortedIterator operator+( const TrivialCircularLockFreeQueueSortedIterator& source_iterator, const size_t& numeric_value )
        {
            size_t new_index_position = source_iterator.m_index_position + numeric_value;

            while ( new_index_position >= source_iterator.m_container_size )
            {
                new_index_position -= source_iterator.m_container_size;
            }

            return TrivialCircularLockFreeQueueSortedIterator( source_iterator.m_circular_lock_free_queue, new_index_position );
        }

        friend inline TrivialCircularLockFreeQueueSortedIterator operator-( const TrivialCircularLockFreeQueueSortedIterator& source_iterator, const size_t& numeric_value )
        {
            int new_index_position = static_cast<int>( source_iterator.m_index_position ) - static_cast<int>( numeric_value );

            while ( new_index_position < 0 )
            {
                new_index_position += source_iterator.m_container_size;
            }

            return TrivialCircularLockFreeQueueSortedIterator( source_iterator.m_circular_lock_free_queue, static_cast<size_t>( new_index_position ) );
        }

        friend inline TrivialCircularLockFreeQueueSortedIterator operator+( const size_t& numeric_value, const TrivialCircularLockFreeQueueSortedIterator& source_iterator )
        {
            return source_iterator + numeric_value ;
        }

        friend inline TrivialCircularLockFreeQueueSortedIterator operator-( const size_t& numeric_value, const TrivialCircularLockFreeQueueSortedIterator& source_iterator )
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
        TrivialCircularLockFreeQueueSortedBase<SORTABLE_TYPE>* m_circular_lock_free_queue;

        // indexes
        size_t m_index_position;
        size_t m_last_valid_index_position;
        size_t m_container_size;
        size_t m_container_sort_index;
    };
    //-------------------------------------------------------------------------------------------------

    //-------------------------------------------------------------------------------------------------
    // End of Random Access Iterator
    //-------------------------------------------------------------------------------------------------

    //-------------------------------------------------------------------------------------------------

    // ------------------------------------------------------------------------------------------------
    // forward declaration
    // ------------------------------------------------------------------------------------------------
    template<typename SORTABLE_TYPE> 
    class TrivialCircularLockFreeQueueSortedEvo;

    //-------------------------------------------------------------------------------------------------

    // ------------------------------------------------------------------------------------------------
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

    // ------------------------------------------------------------------------------------------------
    // Comparing Functor for POINTER. It has to be passed to std::sort and std::partial_sort as last argumente
    // ------------------------------------------------------------------------------------------------
    template<typename POINTER_TYPE>
    class SequenceablePoniterComparator
    {
    public:
        bool operator()(const POINTER_TYPE& comparable1
                      , const POINTER_TYPE& comparable2) const
        {
            return *comparable1 < *comparable2;
        }
    };
    //-------------------------------------------------------------------------------------------------

    /**  ----------------------------------------------------------------------------------------------
    *  Specialization for SHARED pointer
    *  ------------------------------------------------------------------------------------------------
    */
    template< typename SORTABLE_TYPE >
    class TrivialCircularLockFreeQueueSortedEvo< std::shared_ptr< SORTABLE_TYPE > >
        : public TrivialCircularLockFreeQueueSortedBase< std::shared_ptr< SORTABLE_TYPE > >
    {
    public:
        // CTOR
        TrivialCircularLockFreeQueueSortedEvo(size_t queue_size, UInt8 max_disorder_percent)
            : TrivialCircularLockFreeQueueSortedBase< std::shared_ptr< SORTABLE_TYPE > >( queue_size, max_disorder_percent )
        {
        }

        // DTOR
        virtual ~TrivialCircularLockFreeQueueSortedEvo()
        {
        }

        // partial sorting of queue from m_sort_index to m_write_index of max_sorting_size elements
        void partial_sort()
        {
            TrivialCircularLockFreeQueueSortedBase< std::shared_ptr< SORTABLE_TYPE > >
                ::partial_sort( SequenceablePoniterComparator< std::shared_ptr< SORTABLE_TYPE > >() );
        }

        // total sort of queue from m_sort_index to m_write_index
        void sort()
        {
            TrivialCircularLockFreeQueueSortedBase< std::shared_ptr< SORTABLE_TYPE > >
                ::sort( SequenceablePoniterComparator< std::shared_ptr< SORTABLE_TYPE > >() );
        }
    };
    //-------------------------------------------------------------------------------------------------

    //-------------------------------------------------------------------------------------------------
    // End of Specialization for SHARED pointer
    //-------------------------------------------------------------------------------------------------

    //-------------------------------------------------------------------------------------------------

    /**  ----------------------------------------------------------------------------------------------
    *  Specialization for UNIQUE pointer
    *  ------------------------------------------------------------------------------------------------
    */
    template< typename SORTABLE_TYPE >
    class TrivialCircularLockFreeQueueSortedEvo< std::unique_ptr< SORTABLE_TYPE > >
        : public TrivialCircularLockFreeQueueSortedBase< std::unique_ptr< SORTABLE_TYPE > >
    {
    public:
        // CTOR
        TrivialCircularLockFreeQueueSortedEvo(size_t queue_size, UInt8 max_disorder_percent)
            : TrivialCircularLockFreeQueueSortedBase< std::unique_ptr< SORTABLE_TYPE > >( queue_size, max_disorder_percent )
        {
        }

        // DTOR
        virtual ~TrivialCircularLockFreeQueueSortedEvo()
        {
        }

        // partial sorting of queue from m_sort_index to m_write_index of max_sorting_size elements
        void partial_sort()
        {
            TrivialCircularLockFreeQueueSortedBase< std::unique_ptr< SORTABLE_TYPE > >
                ::partial_sort( SequenceablePoniterComparator< std::unique_ptr< SORTABLE_TYPE > >() );
        }

        // total sort of queue from m_sort_index to m_write_index
        void sort()
        {
            TrivialCircularLockFreeQueueSortedBase< std::unique_ptr< SORTABLE_TYPE > >
                ::sort( SequenceablePoniterComparator< std::unique_ptr< SORTABLE_TYPE > >() );
        }
    };
    //-------------------------------------------------------------------------------------------------

    //-------------------------------------------------------------------------------------------------
    // End of Specialization for SHARED pointer
    //-------------------------------------------------------------------------------------------------

    //-------------------------------------------------------------------------------------------------

    // ------------------------------------------------------------------------------------------------
    // Comparing Functor for std::tuple, having norm in <0>. It has to be passed to std::sort and std::partial_sort as last argumente
    // ------------------------------------------------------------------------------------------------
    template<typename ELEMENT_TYPE>
    class SequenceableTupleComparator
    {
    public:
        bool operator() (const std::tuple< UInt64, UInt8, ELEMENT_TYPE >& comparable1
                       , const std::tuple< UInt64, UInt8, ELEMENT_TYPE >& comparable2) const
        {
            return std::get<0>(comparable1) < std::get<0>(comparable2);
        }
    };
    //-------------------------------------------------------------------------------------------------

    //-------------------------------------------------------------------------------------------------

    /**  ----------------------------------------------------------------------------------------------
    *  Specialization for TUPLE containing a SHARED pointer
    *  ------------------------------------------------------------------------------------------------
    */
    template< typename SORTABLE_TYPE >
    class TrivialCircularLockFreeQueueSortedEvo< std::tuple< UInt64, UInt8, std::shared_ptr<SORTABLE_TYPE> > >
        : public TrivialCircularLockFreeQueueSortedBase< std::tuple< UInt64, UInt8, std::shared_ptr<SORTABLE_TYPE> > >
    {
    public:
        // CTOR
        TrivialCircularLockFreeQueueSortedEvo(size_t queue_size, UInt8 max_disorder_percent)
            : TrivialCircularLockFreeQueueSortedBase< std::tuple< UInt64, UInt8, std::shared_ptr<SORTABLE_TYPE> > >( queue_size, max_disorder_percent )
        {
        }

        // DTOR
        virtual ~TrivialCircularLockFreeQueueSortedEvo()
        {
        }

        // partial sorting of queue from m_sort_index to m_write_index of max_sorting_size elements
        void partial_sort()
        {
            TrivialCircularLockFreeQueueSortedBase< std::tuple< UInt64, UInt8, std::shared_ptr<SORTABLE_TYPE> > >
                ::partial_sort( SequenceableTupleComparator< std::shared_ptr<SORTABLE_TYPE> >() );
        }

        // total sort of queue from m_sort_index to m_write_index
        void sort()
        {
            TrivialCircularLockFreeQueueSortedBase< std::tuple< UInt64, UInt8, std::shared_ptr<SORTABLE_TYPE> > >
                ::sort( SequenceableTupleComparator< std::shared_ptr<SORTABLE_TYPE> >() );
        }
    };
    //-------------------------------------------------------------------------------------------------

    //-------------------------------------------------------------------------------------------------
    // End of Specialization for TUPLE containing a SHARED pointer
    //-------------------------------------------------------------------------------------------------

    //-------------------------------------------------------------------------------------------------

   /**  ----------------------------------------------------------------------------------------------
    *  Specialization for TUPLE containing a UNIQUE pointer
    *  ------------------------------------------------------------------------------------------------
    */
    template<typename SORTABLE_TYPE>
    class TrivialCircularLockFreeQueueSortedEvo< std::tuple< UInt64, UInt8, std::unique_ptr<SORTABLE_TYPE> > >
        : public TrivialCircularLockFreeQueueSortedBase< std::tuple< UInt64, UInt8, std::unique_ptr<SORTABLE_TYPE> > >
    {
    public:
        // CTOR
        TrivialCircularLockFreeQueueSortedEvo(size_t queue_size, UInt8 max_disorder_percent)
            : TrivialCircularLockFreeQueueSortedBase< std::tuple< UInt64, UInt8, std::unique_ptr<SORTABLE_TYPE> > >( queue_size, max_disorder_percent )
        {
        }

        // DTOR
        virtual ~TrivialCircularLockFreeQueueSortedEvo()
        {
        }

        // partial sorting of queue from m_sort_index to m_write_index of max_sorting_size elements
        void partial_sort()
        {
            TrivialCircularLockFreeQueueSortedBase< std::tuple< UInt64, UInt8, std::unique_ptr<SORTABLE_TYPE> > >
                ::partial_sort( SequenceableTupleComparator< std::unique_ptr<SORTABLE_TYPE> >() );
        }

        // total sort of queue from m_sort_index to m_write_index
        void sort()
        {
            TrivialCircularLockFreeQueueSortedBase< std::tuple< UInt64, UInt8, std::unique_ptr<SORTABLE_TYPE> > >
                ::sort( SequenceableTupleComparator< std::unique_ptr<SORTABLE_TYPE> >() );
        }
    };
    //-------------------------------------------------------------------------------------------------

    //-------------------------------------------------------------------------------------------------
    // End of Specialization for TUPLE containing a UNIQUE pointer
    //-------------------------------------------------------------------------------------------------

    //-------------------------------------------------------------------------------------------------

    /**  ----------------------------------------------------------------------------------------------
    *  Specialization for TUPLE
    *  ------------------------------------------------------------------------------------------------
    */
    template<typename SORTABLE_TYPE>
    class TrivialCircularLockFreeQueueSortedEvo< std::tuple< UInt64, UInt8, SORTABLE_TYPE > >
        : public TrivialCircularLockFreeQueueSortedBase< std::tuple< UInt64, UInt8, SORTABLE_TYPE > >
    {
    public:
        // CTOR
        TrivialCircularLockFreeQueueSortedEvo(size_t queue_size, UInt8 max_disorder_percent)
            : TrivialCircularLockFreeQueueSortedBase< std::tuple< UInt64, UInt8, SORTABLE_TYPE > >( queue_size, max_disorder_percent )
        {
        }

        // DTOR
        virtual ~TrivialCircularLockFreeQueueSortedEvo()
        {
        }

        // partial sorting of queue from m_sort_index to m_write_index of max_sorting_size elements
        void partial_sort()
        {
            TrivialCircularLockFreeQueueSortedBase< std::tuple< UInt64, UInt8, SORTABLE_TYPE > >
                ::partial_sort( SequenceableTupleComparator< SORTABLE_TYPE >() );
        }

        // total sort of queue from m_sort_index to m_write_index
        void sort()
        {
            TrivialCircularLockFreeQueueSortedBase< std::tuple< UInt64, UInt8, SORTABLE_TYPE > >
                ::sort( SequenceableTupleComparator< SORTABLE_TYPE >() );
        }
    };
    //-------------------------------------------------------------------------------------------------

    //-------------------------------------------------------------------------------------------------
    // End of Specialization for TUPLE
    //-------------------------------------------------------------------------------------------------

    //-------------------------------------------------------------------------------------------------


    //-------------------------------------------------------------------------------------------------

} // namespace

// --------------------------------------------------------------------------------------------------------
#endif // INCLUDE_TRIVIALCIRCULARLOCKFREEQUEUE_SORTED_EVO_NG
