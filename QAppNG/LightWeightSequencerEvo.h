#pragma once
/** ===================================================================================================================
* @file    LightWeightSequencer NEXT GENERATION HEADER FILE
*
* @brief   GsmApp Evolution - code refactoring project
*          This class implement a light weight sequencer (LWS)
*
* @copyright
*
* @history
* REF#        Who                                                              When          What
* #3062       A. Della Villa, A. Manetti, F. Gragnani, R. Buti, S. Luceri      Jan-2009      Code Refactoring
* #3775       Bennett Schneider                                                Nov-2009      Removed dispatcher thread
* #4732       A. Della Villa                                                   Mar-2010      Dynamic add of new queues
* #5466       A. Della Villa                                                   Sep-2010      better handling of out-of-order
*                                                                                            and dynamic queue adding, switched
*                                                                                            to boost shared_lock and boost
*                                                                                            thread specific storage (TSS).
* #5774       Stanislav Timinsky                                               Jan-2011      Added flushing of queues at
*                                                                                            shutdown time
* #6190       A. Della Villa                                                   Jul-2011      waiting for empty queue
*                                                                                            exceptions (for PASS and DROP)
* #6493       A. Della Villa                                                   Feb-2012      detect per queue out of order
* #8060       A. Della Villa                                                   Jan-2013      added "time" limitation to queue waiting
* #8060.......A. Della Villa, D. Verna, F. Lasagni                             Feb-2013      LWS Version 2.0 - added SORTABLE QUEUES
*                                                                                            , CAL removal step A, CAL removal final step
* #11371      A. Della Villa, D. Verna                                         Nov-2014......LWS Version 3.0 - total refactoring using C++11
*
* @endhistory
* ===================================================================================================================
*/

// Include STL & BOOST
#include <string>
#include <array>
#include <tuple>
#include <functional>
#include <memory>
#include <thread>
#include <mutex>
#include <iomanip>
#include <type_traits>

// Include Ecuba
#include "core.h"
#include "TrivialCircularLockFreeQueueEvo.h"
#include "TrivialCircularLockFreeQueueSortedEvo.h"
#include "ThreadCounter.h"

namespace QAppNG
{
    // forward declarations
    template< typename SEQUENCEABLE_CLASS, typename DISPATCH_FUNCTION_TYPE, typename NORM_FUNCTOR_CLASS, typename EVENT_HANDLER_CLASS >
    class LightWeightSequencerEvo;

    // --------------------------------------------------------------------------------------------------------------------

    enum class SequenceableNormProperties : UInt8
    {
        UNDEFINED_NORM,
        ELEMENT_NORM_VALUE,
        ELEMENT_TO_INSTANT_PASS,
        ELEMENT_TO_INSTANT_DROP,
        EVENT_NORM_VALUE,
        EVENT_TO_INSTANT_PASS,
        EVENT_TO_INSTANT_DROP
    };

    // --------------------------------------------------------------------------------------------------------------------

    template< typename SEQUENCEABLE_CLASS >
    struct SequenceableDefaultNormFunctor
    {
        bool operator() ( UInt64& norm_value, UInt8& norm_properties, SEQUENCEABLE_CLASS& element_to_evalutate )
        {
            norm_value = 0;
            norm_properties = static_cast<UInt8>( SequenceableNormProperties::ELEMENT_TO_INSTANT_PASS );
            return true;
        }
    };

    // --------------------------------------------------------------------------------------------------------------------

    struct SequenceableDefaultEventHandler
    {
        template< typename SEQUENCER_CLASS, typename SEQUENCEABLE_EVENT_CLASS >
        bool operator() ( SEQUENCER_CLASS& sequencer, size_t queue_index, SEQUENCEABLE_EVENT_CLASS& event_to_evaluate )
        {
            return sequencer.push( queue_index, event_to_evaluate );
        }
    };

    // --------------------------------------------------------------------------------------------------------------------

    class QueueProperties
    {
    public:
        // Queue Handling Parameters
        enum class QueueType : UInt8 { NORMAL, SORTED, EVENT } m_type;
        volatile bool                                          m_is_active;
        size_t                                                 m_current_capacity;
        UInt64                                                 m_last_pushed_norm_value;
        SequenceableNormProperties                             m_last_pushed_norm_prop;
        UInt64                                                 m_last_popped_norm_value;
        SequenceableNormProperties                             m_last_popped_norm_prop;
        size_t                                                 m_pushing_thread_id;

        // Queue Statistics
        UInt64 m_number_of_pushed_element;
        UInt64 m_number_of_input_dropped_element;
        UInt64 m_number_of_out_of_order_events;

        // CTOR
        QueueProperties( QueueType type = QueueType::NORMAL );
    };

    // --------------------------------------------------------------------------------------------------------------------

    class LightWeightSequencerConfiguration
    {
    public:
        template< typename SEQUENCEABLE_CLASS, typename DISPATCH_FUNCTION_TYPE, typename NORM_FUNCTOR_CLASS, typename EVENT_HANDLER_CLASS >
        friend class LightWeightSequencerEvo;

        template< typename SEQUENCEABLE_CLASS, typename DISPATCH_FUNCTION_TYPE, typename NORM_FUNCTOR_CLASS, typename EVENT_HANDLER_CLASS >
        friend std::ostream& operator<<(std::ostream& output, const LightWeightSequencerEvo< SEQUENCEABLE_CLASS, DISPATCH_FUNCTION_TYPE, NORM_FUNCTOR_CLASS, EVENT_HANDLER_CLASS >& lws_sequencer);

        // LWS Overload Strategy
        enum class LwsInputQueuesOverloadStrategy : UInt8 { eIfOverloadDropInInput, eIfOverloadWaitForFreePlaceInInputQueue };

        // Default values
        static const size_t DEFAULT_INPUT_QUEUE_SIZE;
        static const size_t DEFAULT_MAX_CONSUMABLE_PER_LOOP; // 0 -> disabled
        static const UInt8  DEFAULT_MAX_FILLING_PERCENT_BEFORE_STOP_WAITING_EMPTY_QUEUES;
        static const UInt64 DEFAULT_MAX_NORM_DIFFERENCE_BEFORE_STOP_WAITING_EMPTY_QUEUES;
        static const UInt8  DEFAULT_SORTED_QUEUE_MINIMUN_FILLUP_BEFORE_SORTING_PERCENT;
        static const UInt8  DEFAULT_SORTED_QUEUE_DISORDER_TOLLERANCE_PERCENT;
        static const UInt16 DEFAULT_THREAD_IDLE_SLEEP_TIME;

        // configuration params
        size_t                         m_input_queues_size;
        LwsInputQueuesOverloadStrategy m_input_queues_overload_strategy;
        size_t                         m_max_consumables_per_loop;
        UInt8                          m_max_filling_percent_before_stop_waiting_empty_queues;
        UInt64                         m_max_norm_difference_before_stop_waiting_empty_queues;
        UInt8                          m_sorted_queue_minimum_fillup_before_sorting_percent;
        UInt8                          m_sorted_queue_disorder_tollerance_percent;
        UInt16                         m_thread_idle_sleep_time;

        // CTOR will init all configuration parameters to DEFAULT
        LightWeightSequencerConfiguration();
    private:
        // derived params
        size_t                     m_capacity_limit;

        // LWS last pushed/popped
        UInt64                              m_global_last_pushed_norm_value;
        volatile SequenceableNormProperties m_global_last_pushed_norm_prop;
        UInt64                              m_global_last_pushed_queue_index;
        UInt64                              m_global_last_popped_norm_value;
        SequenceableNormProperties          m_global_last_popped_norm_prop;
        UInt64                              m_global_last_popped_queue_index;
        // LWS stats
        UInt64                     m_global_number_of_out_of_order_events;
        UInt64                     m_global_number_of_wating_for_empty_queues_events;
        UInt64                     m_global_number_of_overwait_events;
        mutable UInt64             m_number_of_processed_elements_before_sleeping;       // Processed Sequenceables Before Sleeping Counter
        mutable UInt64             m_global_number_of_extractor_thread_sleep_events;
        UInt64                     m_global_number_of_sorter_thread_sleep_events;
        UInt64                     m_global_number_of_popped_elements;
        UInt64                     m_global_number_of_output_dropped_elements;
    };
    
    // --------------------------------------------------------------------------------------------------------------------

    template< typename SEQUENCEABLE_CLASS, typename DISPATCH_FUNCTION_TYPE, typename NORM_FUNCTOR_CLASS = SequenceableDefaultNormFunctor<SEQUENCEABLE_CLASS>, typename EVENT_HANDLER_CLASS = SequenceableDefaultEventHandler >
    class LightWeightSequencerEvo
    {
    private:
        // internal structure used in queues, for each element it contains norm and norm props. It is not exposed outside
        typedef std::tuple< UInt64, UInt8, SEQUENCEABLE_CLASS > DECORATED_SEQUENCEABLE_CLASS;

        // queues used by LWS
        typedef TrivialCircularLockFreeQueueEvo< DECORATED_SEQUENCEABLE_CLASS >         LWS_LOCK_FREE_QUEUE;
        typedef TrivialCircularLockFreeQueueSortedEvo< DECORATED_SEQUENCEABLE_CLASS >   LWS_SORTABLE_LOCK_FREE_QUEUE;

        // Special Values
        static const size_t INVALID_QUEUE_INDEX = 0;                                               // = 0 since queue zero is internally used for clock events
        static const size_t MAX_NUMBER_OF_RUNTIME_ADDED_QUEUES = 64;                               // Max Number Of Runtime Added Queues not Reserved for Time Events
        static const size_t MAX_NUMBER_OF_TOTAL_QUEUES   = MAX_NUMBER_OF_RUNTIME_ADDED_QUEUES + 2; // This is the size of each preallocated-vector

        // TODO: This sould be allowed in C++11 but gcc 4.6.2 doesn't support it
        // friend EVENT_HANDLER_CLASS;

    public:

        typedef DISPATCH_FUNCTION_TYPE DispatchFunction;

        /**  ------------------------------------------------------------------------------------------------
        *  called by getStatus
        *  ------------------------------------------------------------------------------------------------
        */
        template< typename _SEQUENCEABLE_CLASS, typename _DISPATCH_FUNCTION_TYPE, typename _NORM_FUNCTOR_CLASS, typename _EVENT_HANDLER_CLASS >
        friend std::ostream& operator<<(std::ostream& output, const LightWeightSequencerEvo< _SEQUENCEABLE_CLASS, _DISPATCH_FUNCTION_TYPE, _NORM_FUNCTOR_CLASS, _EVENT_HANDLER_CLASS >& lws_sequencer);

        /**  ------------------------------------------------------------------------------------------------
        *  CTOR
        *  ------------------------------------------------------------------------------------------------
        */
        LightWeightSequencerEvo( const std::string& lws_name
                               , LightWeightSequencerConfiguration& lws_configuration
                               , DISPATCH_FUNCTION_TYPE& dispatch_function
                               , std::function< void() > idle_function = nullptr )
            : m_lws_name( lws_name )
            , m_lws_state(eLwsIdleState)
            , m_lws_configuration( lws_configuration )
            , m_dispatch_function( dispatch_function )
            , m_idle_function( idle_function )
            // queues handling
            , m_number_of_normal_queues( 0 )
            , m_number_of_sortable_queues( 0 )
            , m_number_of_time_event_queues( 0 )
            , m_total_number_of_queues( 0 )
            // threading
            , m_extractor_thread_id(ThreadCounter::UNDEFINED_THREAD_ID)
            , m_sorter_thread_id(ThreadCounter::UNDEFINED_THREAD_ID)
            , m_stop_extractor_thread( false )
            , m_stop_sorter_thread( false )
            , m_extractor_thread_is_running( false )
            , m_sorter_thread_is_running( false )
        {
            // we resize all to MAX_NUMBER_OF_TOTAL_QUEUES = MAX_NUMBER_OF_RUNTIME_ADDED_QUEUES + 2: we will use queue_0 for TimePulseEvents and queue_1 for PeriodicTimeEvents
            m_input_queues.resize( MAX_NUMBER_OF_TOTAL_QUEUES );
            m_input_queues_properties.resize( MAX_NUMBER_OF_TOTAL_QUEUES );
            m_sortable_queue_indexes.resize( MAX_NUMBER_OF_TOTAL_QUEUES );
            m_thread_ids_queue_ids_mapping_vector.resize( MAX_NUMBER_OF_TOTAL_QUEUES, nullptr );

            // create EVENT dedicated Queues for TimerEvents that are a special category of sequenceable, emitted periodically.
            m_input_queues[0].reset(new LWS_LOCK_FREE_QUEUE(m_lws_configuration.m_input_queues_size));
            m_input_queues_properties[0].reset(new QueueProperties(QueueProperties::QueueType::EVENT));
            ++m_number_of_time_event_queues;
            ++m_total_number_of_queues;

            m_input_queues[1].reset(new LWS_LOCK_FREE_QUEUE(m_lws_configuration.m_input_queues_size));
            m_input_queues_properties[1].reset(new QueueProperties(QueueProperties::QueueType::EVENT));
            ++m_number_of_time_event_queues;
            ++m_total_number_of_queues;

            m_lws_state = eLwsRunningState;
        }

        /**  ------------------------------------------------------------------------------------------------
        *  DTOR
        *  ------------------------------------------------------------------------------------------------
        */
        ~LightWeightSequencerEvo()
        {
            m_lws_state = eLwsIdleState;

            for ( auto per_thread_queue_index : m_thread_ids_queue_ids_mapping_vector )
            {
                delete per_thread_queue_index;
            }
        }

        /**  ------------------------------------------------------------------------------------------------
        *  Add queue
        *  ------------------------------------------------------------------------------------------------
        */
        size_t addQueue()
        {
            // get a unique WRITE LOCK
            std::unique_lock<std::mutex> write_lock(m_queue_adding_removing_mutex);

            // if we are not in the RUNNING  STATE forbid addQueue()
            if (m_lws_state != eLwsRunningState)
            {
                return INVALID_QUEUE_INDEX;
            }

            size_t new_queue_index = getNextQueueIndex();

            if ( new_queue_index != INVALID_QUEUE_INDEX )
            {
                initExtractorThread();

                m_input_queues[new_queue_index].reset( new LWS_LOCK_FREE_QUEUE( m_lws_configuration.m_input_queues_size ) );
                m_input_queues_properties[new_queue_index].reset( new QueueProperties( QueueProperties::QueueType::NORMAL ) );
                m_input_queues_properties[new_queue_index]->m_pushing_thread_id = QAppNG::ThreadCounter::instance().getThreadId();
                ++m_number_of_normal_queues;
            }

            return new_queue_index;
        }

        /**  ------------------------------------------------------------------------------------------------
        *  Add sortable queue
        *  ------------------------------------------------------------------------------------------------
        */
        size_t addSortableQueue() 
        {
            // get a unique WRITE LOCK
            std::unique_lock<std::mutex> write_lock( m_queue_adding_removing_mutex );

            // if we are not in the RUNNING  STATE forbid addQueue()
            if ( m_lws_state != eLwsRunningState)
            {
                return INVALID_QUEUE_INDEX;
            }

            size_t new_queue_index = getNextQueueIndex();

            if ( new_queue_index != INVALID_QUEUE_INDEX )
            {
                initSorterThread();

                initExtractorThread();

                m_input_queues[new_queue_index].reset( new LWS_SORTABLE_LOCK_FREE_QUEUE( m_lws_configuration.m_input_queues_size, m_lws_configuration.m_sorted_queue_disorder_tollerance_percent ) );
                m_input_queues_properties[new_queue_index].reset( new QueueProperties( QueueProperties::QueueType::SORTED ) );
                m_input_queues_properties[new_queue_index]->m_pushing_thread_id = QAppNG::ThreadCounter::instance().getThreadId();
                m_sortable_queue_indexes[ static_cast< size_t >( m_number_of_sortable_queues ) ] = new_queue_index;
                ++m_number_of_sortable_queues;
            }

            return new_queue_index;
        }

        /**  ------------------------------------------------------------------------------------------------
        *  Check if a queue is full
        *  ------------------------------------------------------------------------------------------------
        */
        inline bool full( UInt64 thread_id ) const
        {
            return m_input_queues[static_cast<size_t>(thread_id)]->full();
        }

        /**  ------------------------------------------------------------------------------------------------
        *  Push an element in thread_idth queue
        *  ------------------------------------------------------------------------------------------------
        */
        inline bool push( size_t queue_id, SEQUENCEABLE_CLASS& element )
        {
            // if we are not in RUNNING STATE forbid push()
            if ( m_lws_state == eLwsStoppingState )
            {
                return false;
            }

            // **************** ACTIVATE ****************
            // activate queue when PDU is received
            activateQueue( queue_id );

            // ****************   PUSH   ****************
            // Queues are BLOCKING by default, so we push an element either if Wait policy is enabled or input_queue[n] is not full
            if (  m_lws_configuration.m_input_queues_overload_strategy == LightWeightSequencerConfiguration::LwsInputQueuesOverloadStrategy::eIfOverloadWaitForFreePlaceInInputQueue
               || !m_input_queues[queue_id]->full() )
            {
                // CALCULATE norm value and props
                m_norm_functor( std::get<0>( m_input_queues[queue_id]->back() )
                              , std::get<1>( m_input_queues[queue_id]->back() )
                              , element );

                UInt64 element_norm_value( std::get<0>( m_input_queues[queue_id]->back() ) );
                SequenceableNormProperties element_norm_prop( static_cast<SequenceableNormProperties>( std::get<1>( m_input_queues[queue_id]->back() ) ) );

                // DROP elements having UNDEFINED NORM
                if ( element_norm_prop == SequenceableNormProperties::UNDEFINED_NORM )
                {
                    ++m_input_queues_properties[queue_id]->m_number_of_input_dropped_element;

                    return false;
                }

                // PUSH in input_queue[n] using C++11 Movability
                m_input_queues[queue_id]->push( std::move( element ) );

                // CALCULATE CAPACITY
                m_input_queues_properties[queue_id]->m_current_capacity = m_input_queues[queue_id]->getCurrentCapacity();

                // increment number of pushed elements in input
                ++m_input_queues_properties[queue_id]->m_number_of_pushed_element;

                // CALCULATE per queue last pushed norm value (making it monotone)
                if ( element_norm_prop != SequenceableNormProperties::UNDEFINED_NORM )
                {
                    if ( element_norm_value >= m_input_queues_properties[queue_id]->m_last_pushed_norm_value )
                    {
                        m_input_queues_properties[queue_id]->m_last_pushed_norm_value = element_norm_value;
                        m_input_queues_properties[queue_id]->m_last_pushed_norm_prop = element_norm_prop;
                    }

                    // Store Last Pushed Data if not Reserved Queue
                    if ( queue_id > 1 )
                    {
                        m_lws_configuration.m_global_last_pushed_norm_value = element_norm_value;
                        m_lws_configuration.m_global_last_pushed_norm_prop = element_norm_prop;
                        m_lws_configuration.m_global_last_pushed_queue_index = queue_id;
                    }
                }

                return true;
            }

            // Queue is full and Queue Overload strategy is DROP -> DROP element 
            m_input_queues_properties[queue_id]->m_current_capacity = 0;
            ++m_input_queues_properties[queue_id]->m_number_of_input_dropped_element;

            return false;
        }

        /**  ------------------------------------------------------------------------------------------------
        *  Thread Safe Push of an element in the appropriate queue
        *  ------------------------------------------------------------------------------------------------
        */
        inline bool perThreadPush( SEQUENCEABLE_CLASS& element )
        {
            size_t thread_id = ThreadCounter::Instance().getThreadId();

            if ( m_thread_ids_queue_ids_mapping_vector[thread_id] == nullptr )
            {
                m_thread_ids_queue_ids_mapping_vector[thread_id] = new size_t( addQueue() );
            }

            return push( *m_thread_ids_queue_ids_mapping_vector[thread_id], element );
        }

        /**  ------------------------------------------------------------------------------------------------
        *  Thread Safe Push of an element in the appropriate sortable queue
        *  ------------------------------------------------------------------------------------------------
        */
        inline bool perThreadSortedPush(SEQUENCEABLE_CLASS& element)
        {
            size_t thread_id = ThreadCounter::Instance().getThreadId();

            if ( m_thread_ids_queue_ids_mapping_vector[thread_id] == nullptr )
            {
                m_thread_ids_queue_ids_mapping_vector[thread_id] = new size_t( addSortableQueue() );
            }

            return push( *m_thread_ids_queue_ids_mapping_vector[thread_id], element );
        }

        /**  ------------------------------------------------------------------------------------------------
        *  Push a TimePulseEvent in dedicated CLOCK queue. This method is typically called by QVirtualClock
        *  ------------------------------------------------------------------------------------------------
        */
        inline bool pushTimePulseEvent( SEQUENCEABLE_CLASS& sequenceable_time_event )
        {
            size_t thread_id = ThreadCounter::Instance().getThreadId();

            // Discard any Time Pulse Events Not emitted by QVirtualClock Thread
            if ( m_time_pulse_thread_id != ThreadCounter::UNDEFINED_THREAD_ID
                    && m_time_pulse_thread_id != thread_id )
            {
                return false;
            }

            if ( m_thread_ids_queue_ids_mapping_vector[thread_id] == nullptr )
            {
                initExtractorThread();

                m_thread_ids_queue_ids_mapping_vector[thread_id] = new size_t( 0 );

                m_input_queues_properties[0]->m_pushing_thread_id = thread_id;

                // First Time Call Storing Of Time Pulse Event QvirtualClock Thread Id
                if ( m_time_pulse_thread_id == ThreadCounter::UNDEFINED_THREAD_ID )
                {
                    m_time_pulse_thread_id = thread_id;
                }
            }

            return m_event_handler( *this, *m_thread_ids_queue_ids_mapping_vector[thread_id], sequenceable_time_event );
        }

        /**  ------------------------------------------------------------------------------------------------
        *  Push a PeriodicTimerEvent in dedicated CLOCK queue. This method is typically called by PeriodicTimer
        *  ------------------------------------------------------------------------------------------------
        */
        inline bool pushPeriodicTimerEvent( SEQUENCEABLE_CLASS& sequenceable_time_event )
        {
            size_t thread_id = ThreadCounter::Instance().getThreadId();

            if ( m_thread_ids_queue_ids_mapping_vector[thread_id] == nullptr )
            {
                initExtractorThread();

                m_thread_ids_queue_ids_mapping_vector[thread_id] = new size_t(1);

                m_input_queues_properties[1]->m_pushing_thread_id = thread_id;
            }

            return m_event_handler( *this, *m_thread_ids_queue_ids_mapping_vector[thread_id], sequenceable_time_event );
        }

        /**  ------------------------------------------------------------------------------------------------
        *  Get number of enabled queues
        *  ------------------------------------------------------------------------------------------------
        */
        size_t getNumberOfActiveQueues() const
        {
            size_t number_of_active_queues(0);

            for (size_t i = 0; i < m_total_number_of_queues; i++)
            {
                if ( m_input_queues_properties[i]->m_is_active )
                {
                    ++number_of_active_queues;
                }
            }

            return number_of_active_queues;
        }

        /**  ------------------------------------------------------------------------------------------------
        *  Get status text
        *  ------------------------------------------------------------------------------------------------
        */
        std::string getStatus() const
        {
            // get a unique WRITE LOCK
            std::unique_lock<std::mutex> write_lock(m_queue_adding_removing_mutex);

            std::ostringstream output;

            output << *this << std::endl;

            return output.str();
        }

        /**  ------------------------------------------------------------------------------------------------
        *  Shutdown: method is called by another thread (different from the extractor thread).
        *  We have to wait for thread completion before exiting.
        *  ------------------------------------------------------------------------------------------------
        */
        void shutdown()
        {
            if ( m_lws_state == eLwsStoppingState )
            {
                return;
            }

            std::cout << m_lws_name << " Flushing..." << std::endl;

            // Stop sorter thread
            if ( m_sorter_thread_is_running )
            {
                m_stop_sorter_thread = true;

                m_sorter_thread.join();
            }

            // Go In flushing State
            m_lws_state = eLwsFlushingState;

            if ( m_extractor_thread_is_running )
            {
                m_stop_extractor_thread = true;

                m_extractor_thread.join();
            }

            // Go In Stopping State
            m_lws_state = eLwsStoppingState;

            std::cout << m_lws_name << " Completed Shutdown!" << std::endl;
        }

        /**  ------------------------------------------------------------------------------------------------
        *  Get Per Queue Sorter THREAD id
        *  ------------------------------------------------------------------------------------------------
        */
        size_t getPerQueueSorterThreadId() const
        {
            return m_sorter_thread_id;
        }

        /**  ------------------------------------------------------------------------------------------------
        *  Get Extractor THREAD id
        *  ------------------------------------------------------------------------------------------------
        */
        size_t getExtractorThreadId() const
        {
            return m_extractor_thread_id;
        }

        /**  ------------------------------------------------------------------------------------------------
        *  Change LWS State To Flushing
        *  ------------------------------------------------------------------------------------------------
        */
        // Since gcc 4.6.2 doesn't support friend template-type parameters
        // EVENT_HNDLER_CLASS need this method to change LWS state at shutdown.
        // This method should be removed when gcc allows friendship of template-type parameters
        // and be replaced with direct access to m_lws_state from EVENT_HNDLER_CLASS
        void setFlushingMode()
        {
            m_lws_state = eLwsFlushingState;
        }

        /**  ----------------------------------------------------------------------------------------------
        *  Private Members
        *  ------------------------------------------------------------------------------------------------
        */
    private:
        // Main LWS parameters
        std::string                                                                                         m_lws_name;
        volatile enum LwsState { eLwsIdleState, eLwsRunningState, eLwsFlushingState, eLwsStoppingState }    m_lws_state;
        LightWeightSequencerConfiguration                                                                   m_lws_configuration;

        // Norm Functor
        NORM_FUNCTOR_CLASS  m_norm_functor;

        // Event Handler
        EVENT_HANDLER_CLASS m_event_handler;

        // LWS connecting functions
        DISPATCH_FUNCTION_TYPE                      m_dispatch_function;
        std::function< void() >                     m_idle_function;
        std::function< void(SEQUENCEABLE_CLASS&&) > m_extractor_threrad_init_function;

        // LWS Queues and Queues Handling
        std::vector< std::shared_ptr< LWS_LOCK_FREE_QUEUE > >                                           m_input_queues;
        std::vector< std::unique_ptr< QueueProperties > >                                               m_input_queues_properties;
        std::vector< size_t* >                                                                          m_thread_ids_queue_ids_mapping_vector; // LWS Pushing Thread Ids / Queue Ids mapping vector
        std::vector<size_t>                                                                             m_sortable_queue_indexes;              // contains list of sortable queue indexes to find them in m_input_queues vector
        mutable std::mutex                                                                              m_queue_adding_removing_mutex;
        UInt64                                                                                          m_number_of_normal_queues;
        UInt64                                                                                          m_number_of_sortable_queues;
        UInt64                                                                                          m_number_of_time_event_queues;
        UInt64                                                                                          m_total_number_of_queues;

        // Threads handling
        std::thread   m_extractor_thread;
        std::thread   m_sorter_thread;
        mutable size_t        m_extractor_thread_id;
        mutable size_t        m_sorter_thread_id;
        static  size_t        m_time_pulse_thread_id;   // id of Qvirtual Clock Thread inserting time pulse events: this is the same for all LWS istances;
        volatile bool m_stop_extractor_thread;
        volatile bool m_stop_sorter_thread;
        volatile bool m_extractor_thread_is_running;
        volatile bool m_sorter_thread_is_running;

        /**  ----------------------------------------------------------------------------------------------
        *  Private Methods
        *  ------------------------------------------------------------------------------------------------
        */
    private:
        /// Get next queue index, used as index in preallocated data vectors
        inline size_t getNextQueueIndex()
        {
            if ( static_cast< size_t >( m_total_number_of_queues ) < MAX_NUMBER_OF_TOTAL_QUEUES )
            {
                return static_cast< size_t >( m_total_number_of_queues++ );
            }
            else
            {
                return INVALID_QUEUE_INDEX;
            }
        }

        inline size_t getNumberOfSequeceableQueues() const
        {
            return static_cast< size_t >( m_number_of_normal_queues + m_number_of_sortable_queues );
        }

        inline void doShortSleep() const
        {
            std::this_thread::sleep_for( std::chrono::microseconds( LightWeightSequencerConfiguration::DEFAULT_THREAD_IDLE_SLEEP_TIME ) );
        }

        inline void doLongSleep() const
        {
            std::this_thread::sleep_for( std::chrono::milliseconds( LightWeightSequencerConfiguration::DEFAULT_THREAD_IDLE_SLEEP_TIME ) );
        }

        /// init Extractor Thread Once
        inline void initExtractorThread()
        {
            if ( !m_extractor_thread_is_running )
            {
                // derive capacity limit in terms of percentage
                m_lws_configuration.m_capacity_limit = static_cast<size_t>((m_lws_configuration.m_input_queues_size / 100) * (100 - m_lws_configuration.m_max_filling_percent_before_stop_waiting_empty_queues));

                m_extractor_thread = std::thread( [this] () { this->extractorThreadMainLoop(); } );

                while ( !m_extractor_thread_is_running ) 
                {
                    doShortSleep();
                }
            }
        }

        /// init Sorter Thread Once
        inline void initSorterThread()
        {
            if ( !m_sorter_thread_is_running )
            {
                m_sorter_thread = std::thread( [this] () { this->sorterThreadMainLoop(); } );
                while ( !m_sorter_thread_is_running )
                {
                    doShortSleep();
                }
            }
        }

        // Check If Waiting For Slow Queue is enabled
        inline bool isWatingForSlowQueuesEnabled() const
        {
            return m_lws_configuration.m_capacity_limit != m_lws_configuration.m_input_queues_size;
        }

        // Check If we have to Wait For Any Queue, calculating Minimum Capacity and Checking if it is > capacity_limit
        inline bool waitForAnyQueue() const
        {
            if ( !isWatingForSlowQueuesEnabled() || m_lws_state == eLwsFlushingState ) return false;

            // Start from m_number_of_time_event_queues cause first queues are reserved for Time Events
            size_t queue_scan_cycle_start( static_cast< size_t>( m_number_of_time_event_queues ) );
            size_t queue_scan_cycle_end( queue_scan_cycle_start + getNumberOfSequeceableQueues() );

            size_t minimum_capacity_among_all_queues( m_input_queues_properties[queue_scan_cycle_start]->m_current_capacity );

            for ( size_t queue_index = queue_scan_cycle_start + 1; queue_index < queue_scan_cycle_end; ++queue_index )
            {
                if ( m_input_queues_properties[queue_index]->m_current_capacity < minimum_capacity_among_all_queues )
                {
                    minimum_capacity_among_all_queues = m_input_queues_properties[queue_index]->m_current_capacity;
                }
            }

            return minimum_capacity_among_all_queues > m_lws_configuration.m_capacity_limit;
        }

        // Check if any queue has reached age limit
        inline bool isReachedAgeLimitForQueue( size_t queue_index ) const
        {
            if ( m_lws_configuration.m_max_norm_difference_before_stop_waiting_empty_queues == 0 ) return false;

            if ( m_input_queues_properties[queue_index]->m_last_pushed_norm_value
                + m_lws_configuration.m_max_norm_difference_before_stop_waiting_empty_queues < m_lws_configuration.m_global_last_popped_norm_value )
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        /**  ------------------------------------------------------------------------------------------------
        *  Queue State functions
        *  ------------------------------------------------------------------------------------------------
        */
        inline void activateQueue( size_t queue_index )
        {
            m_input_queues_properties[queue_index]->m_is_active = true;
        }

        inline void deActivateQueue( size_t queue_index )
        {
            m_input_queues_properties[queue_index]->m_is_active = false;
        }

        inline bool isQueueActive( size_t queue_index ) const
        {
            return m_input_queues_properties[queue_index]->m_is_active == true;
        }

        /**  ------------------------------------------------------------------------------------------------
        *  Per Queue Sorter THREAD function: order unordered feeds
        *  ------------------------------------------------------------------------------------------------
        */
        void sorterThreadMainLoop()
        {
            // set TID
            m_sorter_thread_id = ThreadCounter::Instance().getThreadId();
            m_sorter_thread_is_running = true;

            while ( !m_stop_sorter_thread )
            {
                bool at_least_one_sorting_done(false);

                // Scan all sortable queues
                for ( size_t i = 0; i < m_number_of_sortable_queues; ++i )
                {
                    std::shared_ptr< LWS_SORTABLE_LOCK_FREE_QUEUE > sortable_queue
                        = std::static_pointer_cast< LWS_SORTABLE_LOCK_FREE_QUEUE >( m_input_queues[m_sortable_queue_indexes[i]] );

                    if (static_cast<size_t>(sortable_queue->getUnSortedQueuePercent()) > m_lws_configuration.m_sorted_queue_minimum_fillup_before_sorting_percent)
                    {
                        sortable_queue->partial_sort();
                        at_least_one_sorting_done = true;
                    }
                } // End of Queue Scan Cycle

                if (!at_least_one_sorting_done)
                {
                    ++m_lws_configuration.m_global_number_of_sorter_thread_sleep_events;
                    doLongSleep();
                }
            } // End of Bib While Loop

            // Sort All Elements At Shutdown
            for (size_t i = 0; i < m_number_of_sortable_queues; ++i)
            {
                std::shared_ptr< LWS_SORTABLE_LOCK_FREE_QUEUE > sortable_queue
                    = std::static_pointer_cast< LWS_SORTABLE_LOCK_FREE_QUEUE >(m_input_queues[m_sortable_queue_indexes[i]]);

                if (sortable_queue->getUnSortedQueue())
                {
                    sortable_queue->sort();
                }
            }

            m_sorter_thread_is_running = false;
        }

        /**  ------------------------------------------------------------------------------------------------
        *  Extractor THREAD function: order and dispatch
        *  ------------------------------------------------------------------------------------------------
        */
        void extractorThreadMainLoop()
        {
            // set TID
            m_extractor_thread_id = ThreadCounter::Instance().getThreadId();

            // Extractor Thread is running
            m_extractor_thread_is_running = true;

            // vector to store indexes of queues that have front with minimum norm value
            std::vector<size_t> minimum_value_queue_indexes;
            minimum_value_queue_indexes.reserve( MAX_NUMBER_OF_RUNTIME_ADDED_QUEUES );

            UInt64 minimum_norm_value( 0xFFFFFFFFFFFFFFFF );

            // if we have empty queues that are active, we have to wait for them!!!
            bool any_active_sequenceable_queue_is_empty(false);

            // Flag to check if all queues are empty, if at the end of queues scanning cycle it's true then extractor thread sleeps
            bool all_queues_are_empty(true);

            // Queue Scan Cycle Limits
            size_t queue_scan_cycle_start( static_cast< size_t>( m_number_of_time_event_queues ) );
            size_t queue_scan_cycle_end( queue_scan_cycle_start + getNumberOfSequeceableQueues() );

            // Thread Loop
            while ( !m_stop_extractor_thread )
            {
                // INIT VARs
                minimum_value_queue_indexes.clear();
                any_active_sequenceable_queue_is_empty = false;
                all_queues_are_empty = true;

                minimum_norm_value = 0xFFFFFFFFFFFFFFFF;

                // Check TimePulseEvent queue
                // If in flushing state, Pause Event Extraction: They wiil be popped out in flushAllQueues in order to perform shutdown correctly
                if ( !m_input_queues[0]->empty() && m_lws_state != eLwsFlushingState )
                {
                    popElementFromTimePulseEventQueue();

                    all_queues_are_empty = false;
                }

                // Check PeriodicTimerEvent queue
                // If in flushing state, Pause Event Extraction: They wiil be popped out in flushAllQueues in order to perform shutdown correctly
                // TODO
                //if ( !m_input_queues[1]->empty() && m_lws_state != eLwsFlushingState  )
                //{
                //    popElementFromPeriodicTimerEventQueue();
                //
                //    all_queues_are_empty = false;
                //}

                // Start from m_number_of_time_event_queues cause first queues are reserved for Time Events
                queue_scan_cycle_start = static_cast< size_t>( m_number_of_time_event_queues );
                queue_scan_cycle_end = queue_scan_cycle_start + getNumberOfSequeceableQueues();

                // Queues Scanning Cycle
                for (size_t queue_index = queue_scan_cycle_start; queue_index < queue_scan_cycle_end; ++queue_index)
                {
                    if ( !m_input_queues[queue_index]->empty() )
                    {
                        // Check Norm Property Head Elements
                        switch ( std::get<1>( m_input_queues[queue_index]->front() ) )
                        {
                            // INSTANT PASS
                            case static_cast<UInt8>( SequenceableNormProperties::ELEMENT_TO_INSTANT_PASS ):
                            case static_cast<UInt8>( SequenceableNormProperties::EVENT_TO_INSTANT_PASS ):
                            {
                                popElementFromInputQueue( queue_index );
                            }
                            break;

                            // DROP
                            case static_cast<UInt8>( SequenceableNormProperties::ELEMENT_TO_INSTANT_DROP ):
                            case static_cast<UInt8>( SequenceableNormProperties::EVENT_TO_INSTANT_DROP ):
                            {
                                dropElementFromInputQueue( queue_index );
                            }
                            break;

                            // NORMED
                            case static_cast<UInt8>( SequenceableNormProperties::ELEMENT_NORM_VALUE ):
                            case static_cast<UInt8>( SequenceableNormProperties::EVENT_NORM_VALUE ):
                            {
                                // Calculate minimum norm
                                if ( std::get<0>( m_input_queues[queue_index]->front() ) < minimum_norm_value )
                                {
                                    minimum_value_queue_indexes.clear();

                                    minimum_value_queue_indexes.push_back( queue_index );
                                    minimum_norm_value = std::get<0>( m_input_queues[queue_index]->front() );
                                }
                                else if( std::get<0>( m_input_queues[queue_index]->front() ) == minimum_norm_value )
                                {
                                    minimum_value_queue_indexes.push_back(queue_index);
                                    minimum_norm_value = std::get<0>( m_input_queues[queue_index]->front() );
                                }
                            }
                            break;

                            default:
                            {
                                assert( 0 );
                            }
                            break;
                        }

                        all_queues_are_empty = false;
                    }
                    else // current queue is empty
                    {
                        // Check queue age and if it is too old DEACTIVATE queue
                        if ( isReachedAgeLimitForQueue( queue_index ) )
                        {
                            deActivateQueue( queue_index );
                        }

                        // Check any active empty queues
                        if ( isQueueActive( queue_index ) )
                        {
                            any_active_sequenceable_queue_is_empty = true;
                        }
                    }
                } // END queue scan cycle

                // Element extraction
                extractElementsOrWaitEmptyQueues( minimum_value_queue_indexes, any_active_sequenceable_queue_is_empty );

                // Sleep if We Reached Upper Limit of Processed Elements Per Cycle Or All Queues Are Empty
                checkNumberOfProcessedElementsAndSleep( all_queues_are_empty );

            }// End Extractor Cycle

            flushAllQueues();

            m_extractor_thread_is_running = false;
        }

        /**  ------------------------------------------------------------------------------------------------
        *  Called by Extractor THREAD to Extract Elements or Wait For Empty Queues
        *  ------------------------------------------------------------------------------------------------
        */
        void extractElementsOrWaitEmptyQueues( std::vector<size_t>& indexes_of_queues_to_extract_from, bool check_empty_queues_for_waiting )
        {
            // No Queues To Pop From
            if ( indexes_of_queues_to_extract_from.empty() )
            {
                return;   // no element extracted
            }

            // Some Empty Queues are present -> Check Them For Waiting
            if ( check_empty_queues_for_waiting )
            {
                // Check if We have to Wait For Any Queue
                bool wait_for_any_queue = waitForAnyQueue();

                // We have To Wait -> Go To Sleep
                if ( wait_for_any_queue )
                {
                    doShortSleep();

                    ++m_lws_configuration.m_global_number_of_extractor_thread_sleep_events;
                    ++m_lws_configuration.m_global_number_of_wating_for_empty_queues_events;

                    // Reset Processed Elements Before Sleeping Counter
                    m_lws_configuration.m_number_of_processed_elements_before_sleeping = 0;

                    return;   // no element extracted
                }
                // Don't wait
                else
                {
                    // Extract any Element
                    for ( auto &i : indexes_of_queues_to_extract_from )
                    {
                        popElementFromInputQueueWithOrderCheck( i );
                    }

                    // Count Overwait Events if Waiting policy is enabled and not in flushing state
                    if ( isWatingForSlowQueuesEnabled() && m_lws_state != eLwsFlushingState )
                    {
                        m_lws_configuration.m_global_number_of_overwait_events += indexes_of_queues_to_extract_from.size();
                    }
                }
            }
            // No Empty Queues are present
            else
            {
                // Extract any Element
                for ( auto &i : indexes_of_queues_to_extract_from )
                {
                    popElementFromInputQueueWithOrderCheck(i);
                }
            }
        }

        /**  ------------------------------------------------------------------------------------------------
        *  Called by Extractor THREAD to pop out elements
        *  ------------------------------------------------------------------------------------------------
        */
        inline bool popElementFromInputQueue( size_t queue_index )
        {
            // dispatch element
            // Move it From the Queue Using C++11 Movability

            auto element_to_be_dispatched = std::move( std::get<2>( m_input_queues[queue_index]->front() ) );

            m_dispatch_function( element_to_be_dispatched );

            // pop element
            m_input_queues[queue_index]->pop();

            // update stats
            ++m_lws_configuration.m_global_number_of_popped_elements;

            // Update Processed Elements Before Sleeping Counter
            ++m_lws_configuration.m_number_of_processed_elements_before_sleeping;

            return true;
        }

        /**  ------------------------------------------------------------------------------------------------
        *  Called by Extractor THREAD to pop out elements: in this case a per queue OUT OF ORDER check will be done
        *  ------------------------------------------------------------------------------------------------
        */
        inline bool popElementFromInputQueueWithOrderCheck( size_t queue_index )
        {
            // Check per-queue out-of-order
            if ( m_input_queues_properties[queue_index]->m_last_popped_norm_value > std::get<0>( m_input_queues[queue_index]->front() ) )
            {
                ++m_input_queues_properties[queue_index]->m_number_of_out_of_order_events;
            }

            // Check global out-of-order
            if ( m_lws_configuration.m_global_last_popped_norm_value > std::get<0>( m_input_queues[queue_index]->front() ) )
            {
                ++m_lws_configuration.m_global_number_of_out_of_order_events;
            }

            // Update
            m_input_queues_properties[queue_index]->m_last_popped_norm_value = std::get<0>( m_input_queues[queue_index]->front() );
            m_input_queues_properties[queue_index]->m_last_popped_norm_prop  = static_cast<SequenceableNormProperties>( std::get<1>( m_input_queues[queue_index]->front() ) );

            m_lws_configuration.m_global_last_popped_norm_value = m_input_queues_properties[queue_index]->m_last_popped_norm_value;
            m_lws_configuration.m_global_last_popped_norm_prop  = m_input_queues_properties[queue_index]->m_last_popped_norm_prop;
            m_lws_configuration.m_global_last_popped_queue_index = queue_index;

            return popElementFromInputQueue( queue_index );
        }

        /**  ------------------------------------------------------------------------------------------------
        *  Called by Extractor THREAD to drop element from queue
        *  ------------------------------------------------------------------------------------------------
        */
        inline bool dropElementFromInputQueue( size_t queue_index )
        {
            // pop element
            m_input_queues[queue_index]->pop();

            // update stats
            ++m_lws_configuration.m_global_number_of_output_dropped_elements;

            // Update Processed Elements Before Sleeping Counter
            ++m_lws_configuration.m_number_of_processed_elements_before_sleeping;

            return true;
        }

        /**  ------------------------------------------------------------------------------------------------
        *  Called by Extractor THREAD to pop out elements from TimePulseEvent queue
        *  ------------------------------------------------------------------------------------------------
        */
        inline bool popElementFromTimePulseEventQueue()
        {
            // Update
            m_input_queues_properties[0]->m_last_popped_norm_value = std::get< 0 >( m_input_queues[0]->front() );
            m_input_queues_properties[0]->m_last_popped_norm_prop = static_cast< SequenceableNormProperties >( std::get< 1 >( m_input_queues[0]->front() ) );

            popElementFromInputQueue( 0 );

            return true;
        }

        /**  ------------------------------------------------------------------------------------------------
        *  Called by Extractor THREAD to pop out elements from PeriodicTimerEvent queue
        *  ------------------------------------------------------------------------------------------------
        */
        inline bool popElementFromPeriodicTimerEventQueue()
        {
            // Update
            m_input_queues_properties[1]->m_last_popped_norm_value = std::get< 0 >( m_input_queues[1]->front() );
            m_input_queues_properties[1]->m_last_popped_norm_prop = static_cast< SequenceableNormProperties >( std::get< 1 >( m_input_queues[1]->front() ) );

            popElementFromInputQueue( 1 );

            return true;
        }

        /**  ------------------------------------------------------------------------------------------------
        *  Flush queues at shutdown
        *  ------------------------------------------------------------------------------------------------
        */
        void flushAllQueues()
        {
            UInt64 minimum_norm_value(0xFFFFFFFFFFFFFFFF);
            size_t number_of_empty_queues(0);

            // vector to store indexes of queues that have front with minimum norm value
            std::vector<size_t> minimum_value_queue_indexes;
            minimum_value_queue_indexes.reserve( MAX_NUMBER_OF_RUNTIME_ADDED_QUEUES );

            // Queue Scan Cycle Limits
            size_t queue_scan_cycle_start( static_cast< size_t>( m_number_of_time_event_queues ) );
            size_t queue_scan_cycle_end( queue_scan_cycle_start + getNumberOfSequeceableQueues() );

            do
            {
                // INIT VARs
                minimum_value_queue_indexes.clear();
                minimum_norm_value = 0xFFFFFFFFFFFFFFFF;
                number_of_empty_queues = 0;

                // Scan All queues
                // Start from m_number_of_time_event_queues cause first queues are reserved for Time Events
                queue_scan_cycle_start = static_cast< size_t>( m_number_of_time_event_queues );
                queue_scan_cycle_end = queue_scan_cycle_start + getNumberOfSequeceableQueues();

                for (size_t queue_index = queue_scan_cycle_start; queue_index < queue_scan_cycle_end; ++queue_index)
                {
                    if ( !m_input_queues[queue_index]->empty() )
                    {
                        switch ( std::get<1>( m_input_queues[queue_index]->front() ) )
                        {
                            case static_cast<UInt8>( SequenceableNormProperties::ELEMENT_TO_INSTANT_PASS ):
                            case static_cast<UInt8>( SequenceableNormProperties::EVENT_TO_INSTANT_PASS ):
                            {
                                popElementFromInputQueue( queue_index );
                            }
                            break;
            
                            case static_cast<UInt8>( SequenceableNormProperties::ELEMENT_TO_INSTANT_DROP ):
                            case static_cast<UInt8>( SequenceableNormProperties::EVENT_TO_INSTANT_DROP ):
                            {
                                dropElementFromInputQueue( queue_index );
                            }
                            break;
            
                            case static_cast<UInt8>( SequenceableNormProperties::ELEMENT_NORM_VALUE ):
                            case static_cast<UInt8>( SequenceableNormProperties::EVENT_NORM_VALUE ):
                            {
                                if ( std::get<0>( m_input_queues[queue_index]->front() ) <= minimum_norm_value )
                                {
                                    minimum_value_queue_indexes.push_back( queue_index );
                                    minimum_norm_value = std::get<0>( m_input_queues[queue_index]->front() );
                                }
                            }
                            break;
            
                            default:
                            {
                                assert( 0 );
                            }
                            break;
                        }
                    }
                    else // current queue is empty
                    {
                        ++number_of_empty_queues;
                    }

                    extractElementsOrWaitEmptyQueues( minimum_value_queue_indexes, false );

                } // END queue scan cycle

            } while ( number_of_empty_queues < getNumberOfSequeceableQueues() );

            // Check PeriodicTimerEvent queue
            // TODO
            //while (!m_input_queues[1]->empty())
            //{
            //    popElementFromPeriodicTimerEventQueue();
            //}

            // Check TimePulseEvent queue
            while ( !m_input_queues[0]->empty() )
            {
                popElementFromTimePulseEventQueue();
            }

            // Call Idle Function Before Exiting
            if (m_idle_function)
            {
                m_idle_function();
            }
        }

        /**  ------------------------------------------------------------------------------------------------
        *  Called by Extractor THREAD to Check If Max Number of Consumables per Loop is reached and Sleep
        *  ------------------------------------------------------------------------------------------------
        */
        inline void checkNumberOfProcessedElementsAndSleep( bool all_queues_are_empty ) const
        {
            // Never Sleeps In Flushing State
            if ( m_lws_state == eLwsFlushingState )
            {
                // Reset Processed Elements Before Sleeping Counter
                m_lws_configuration.m_number_of_processed_elements_before_sleeping = 0;

                return;
            }

            // All queues are empty so go to sleep
            if ( all_queues_are_empty )
            {
                doLongSleep();

                if (m_idle_function)
                {
                    m_idle_function();
                }

                ++m_lws_configuration.m_global_number_of_extractor_thread_sleep_events;

                // Reset Processed Elements Before Sleeping Counter
                m_lws_configuration.m_number_of_processed_elements_before_sleeping = 0;

                return;
            }

            // Never Sleeps if upper limit is 0
            if ( m_lws_configuration.m_max_consumables_per_loop == 0 )
            {
                if ( m_idle_function )
                {
                    m_idle_function();
                }

                // Reset Processed Elements Before Sleeping Counter
                m_lws_configuration.m_number_of_processed_elements_before_sleeping = 0;
            }
            // Max Number of consumables extracted
            else if ( m_lws_configuration.m_number_of_processed_elements_before_sleeping >= m_lws_configuration.m_max_consumables_per_loop )
            {
                doShortSleep();

                if ( m_idle_function )
                {
                    m_idle_function();
                }

                ++m_lws_configuration.m_global_number_of_extractor_thread_sleep_events;

                // Reset Processed Elements Before Sleeping Counter
                m_lws_configuration.m_number_of_processed_elements_before_sleeping = 0;
            }
        }
    };

    // --------------------------------------------------------------------------------------------------------------------

    template< typename SEQUENCEABLE_CLASS, typename DISPATCH_FUNCTION_TYPE, typename NORM_FUNCTOR_CLASS, typename EVENT_HANDLER_CLASS >
    size_t LightWeightSequencerEvo< SEQUENCEABLE_CLASS, DISPATCH_FUNCTION_TYPE, NORM_FUNCTOR_CLASS, EVENT_HANDLER_CLASS >::m_time_pulse_thread_id = ThreadCounter::UNDEFINED_THREAD_ID;

    // --------------------------------------------------------------------------------------------------------------------

    template< typename SEQUENCEABLE_CLASS, typename DISPATCH_FUNCTION_TYPE, typename NORM_FUNCTOR_CLASS, typename EVENT_HANDLER_CLASS >
    std::ostream& operator<<(std::ostream& output, const LightWeightSequencerEvo< SEQUENCEABLE_CLASS, DISPATCH_FUNCTION_TYPE, NORM_FUNCTOR_CLASS, EVENT_HANDLER_CLASS >& lws_sequencer)
    {

        //*****************************
        // Formatting Lambada Functions
        //*****************************

        static auto string_formatter( [] ( const std::string& string_to_format )
            {
                std::ostringstream output;

                output << std::setfill(' ') << std::setiosflags(std::ios::left) << std::setw(44);

                output << string_to_format;

                return output.str();
            } );

        static auto format_as_percent([]( float number_to_format )
            {
                std::ostringstream output;

                output << std::fixed << std::setprecision(2);

                output << number_to_format << "%";

                return output.str();
            } );

        static auto enclose_in_brackets([]( const std::string& string_to_ecose )
            {
                std::ostringstream output;

                output << "(" << string_to_ecose << ")";

                return output.str();
            } ) ;

        static auto format_as_list_entry([]( const std::string& string_to_format )
            {
                static std::string empty_list_entry("--------------------, ");

                if ( string_to_format.empty() )
                {
                    return empty_list_entry;
                }

                std::ostringstream output;

                output << std::setfill(' ') << std::setiosflags(std::ios::left) << std::setw(20);

                output << string_to_format;

                output << std::resetiosflags(std::ios::adjustfield);

                output << ", ";

                return output.str();
            } );

        //**********************
        // Global Section Status
        //**********************

        std::string lws_status;

        switch ( lws_sequencer.m_lws_state )
        {
            case LightWeightSequencerEvo< SEQUENCEABLE_CLASS, DISPATCH_FUNCTION_TYPE, NORM_FUNCTOR_CLASS, EVENT_HANDLER_CLASS >::eLwsIdleState:
                lws_status = "Idle";
                break;
            case LightWeightSequencerEvo< SEQUENCEABLE_CLASS, DISPATCH_FUNCTION_TYPE, NORM_FUNCTOR_CLASS, EVENT_HANDLER_CLASS >::eLwsRunningState:
                lws_status = "Running";
                break;
            case LightWeightSequencerEvo< SEQUENCEABLE_CLASS, DISPATCH_FUNCTION_TYPE, NORM_FUNCTOR_CLASS, EVENT_HANDLER_CLASS >::eLwsFlushingState:
                lws_status = "Flushing";
                break;
            case LightWeightSequencerEvo< SEQUENCEABLE_CLASS, DISPATCH_FUNCTION_TYPE, NORM_FUNCTOR_CLASS, EVENT_HANDLER_CLASS >::eLwsStoppingState:
                lws_status = "Stopping";
                break;
        }

        std::string lws_input_queues_overload_strategy_string( "Drop" );

        if ( lws_sequencer.m_lws_configuration.m_input_queues_overload_strategy == QAppNG::LightWeightSequencerConfiguration::LwsInputQueuesOverloadStrategy::eIfOverloadWaitForFreePlaceInInputQueue )
        {
            lws_input_queues_overload_strategy_string = "Wait";
        }

        std::string max_consumable_limit_string("----------");

        if ( lws_sequencer.m_lws_configuration.m_max_consumables_per_loop > 0 )
        {
            max_consumable_limit_string = std::to_string( lws_sequencer.m_lws_configuration.m_max_consumables_per_loop );
        }

        std::string extractor_thread_id_string("Not Any thread");

        if (lws_sequencer.m_extractor_thread_is_running)
        {
            extractor_thread_id_string = std::to_string(lws_sequencer.m_extractor_thread_id);
        }

        std::string sorter_thread_id_string("Not Any thread");

        if (lws_sequencer.m_sorter_thread_is_running)
        {
            sorter_thread_id_string = std::to_string(lws_sequencer.m_sorter_thread_id);
        }

        output << "-------------------------------------------------------------"   << std::endl;
        output << "LIGHT WEIGHT SEQUENCER (LWS)                                 "   << std::endl;
        output << "-------------------------------------------------------------"   << std::endl;
        output << string_formatter("LWS name:" )                                    << lws_sequencer.m_lws_name                                 << std::endl;
        output << string_formatter("LWS Status:")                                   << lws_status                                               << std::endl;
        output << string_formatter("Total number of INPUT queues:")                 << lws_sequencer.m_total_number_of_queues                   << std::endl;
        output << string_formatter("Number of SORTABLE queues:")                    << lws_sequencer.m_number_of_sortable_queues                << std::endl;
        output << string_formatter("Input queues size:")                            << lws_sequencer.m_lws_configuration.m_input_queues_size    << std::endl;
        output << string_formatter("Input overload strategy:")                      << lws_input_queues_overload_strategy_string                << std::endl;
        output << string_formatter("Max Consumable per Extraction-Loop:")           << max_consumable_limit_string                              << std::endl;
        output << string_formatter("EXTRACTOR Thread ID:")                          << extractor_thread_id_string                               << std::endl;
        output << string_formatter("SORTER Thread ID:")                             << sorter_thread_id_string                                  << std::endl;

        //**********************
        // Aggregated Statistics
        //**********************
        UInt64  total_number_of_pushed_elements(0);
        UInt64  total_number_of_input_dropped_elements(0);

        for (size_t i = 0; i < lws_sequencer.m_total_number_of_queues; ++i)
        {
            total_number_of_pushed_elements += lws_sequencer.m_input_queues_properties[i]->m_number_of_pushed_element;
            total_number_of_input_dropped_elements += lws_sequencer.m_input_queues_properties[i]->m_number_of_input_dropped_element;
        }

        UInt64 total_number_of_input_processed_elements =
            total_number_of_pushed_elements
            + total_number_of_input_dropped_elements;

        UInt64 total_number_of_output_processed_elements =
            lws_sequencer.m_lws_configuration.m_global_number_of_popped_elements +
            lws_sequencer.m_lws_configuration.m_global_number_of_output_dropped_elements;

        UInt64 total_number_of_enqueued_element =
            total_number_of_pushed_elements -
            total_number_of_output_processed_elements;

        UInt64 aggregated_size = lws_sequencer.m_total_number_of_queues * lws_sequencer.m_lws_configuration.m_input_queues_size;

        float input_drop_percentage = total_number_of_input_dropped_elements > 0 ?
            total_number_of_input_dropped_elements * 100.00F / total_number_of_input_processed_elements : 0;

        float output_drop_percentage = lws_sequencer.m_lws_configuration.m_global_number_of_output_dropped_elements > 0 ?
            lws_sequencer.m_lws_configuration.m_global_number_of_output_dropped_elements * 100.00F / total_number_of_output_processed_elements : 0;

        float average_queue_filling_percentage = total_number_of_enqueued_element > 0 ?
            total_number_of_enqueued_element * 100.00F / aggregated_size : 0;

        float out_of_order_percentage = lws_sequencer.m_lws_configuration.m_global_number_of_out_of_order_events > 0 ?
            lws_sequencer.m_lws_configuration.m_global_number_of_out_of_order_events * 100.00F / lws_sequencer.m_lws_configuration.m_global_number_of_popped_elements : 0;

        output << string_formatter("Total PUSHED in Input:")                    << total_number_of_pushed_elements                                                      << std::endl;
        output << string_formatter("Total POPPED in Output:")                   << lws_sequencer.m_lws_configuration.m_global_number_of_popped_elements                 << std::endl;

        output << string_formatter("Total DROPPED in Input:")                   << total_number_of_input_dropped_elements
                                                                                << enclose_in_brackets( format_as_percent(input_drop_percentage) )                      << std::endl;

        output << string_formatter("Total DROPPED in Output:")                  << lws_sequencer.m_lws_configuration.m_global_number_of_output_dropped_elements
                                                                                << enclose_in_brackets( format_as_percent(output_drop_percentage) )                     << std::endl;

        output << string_formatter("INPUT AVERGAGE queues filling:")            << total_number_of_enqueued_element
                                                                                << enclose_in_brackets( format_as_percent(average_queue_filling_percentage) )           << std::endl;

        output << string_formatter("EXTRACTOR out-of-order events")             << lws_sequencer.m_lws_configuration.m_global_number_of_out_of_order_events
                                                                                << enclose_in_brackets( format_as_percent(out_of_order_percentage) )                    << std::endl;

        output << string_formatter("EXTRACTOR queue wait events:")              << lws_sequencer.m_lws_configuration.m_global_number_of_wating_for_empty_queues_events  << std::endl;
        output << string_formatter("EXTRACTOR overwait events:")                << lws_sequencer.m_lws_configuration.m_global_number_of_overwait_events                 << std::endl;
        output << string_formatter("EXTRACTOR sleep events:")                   << lws_sequencer.m_lws_configuration.m_global_number_of_extractor_thread_sleep_events   << std::endl;
        output << string_formatter("SORTER   sleep events:")                    << lws_sequencer.m_lws_configuration.m_global_number_of_sorter_thread_sleep_events      << std::endl;

        //**********************
        // Per Queue Statistics
        //**********************

        // pushing thread ids
        std::ostringstream pushing_thread_id_str_stream;;

        //number of current enqueued elements
        std::ostringstream current_enqueued_elements_str_stream;

        // last pushed Norm Values
        std::ostringstream last_pushed_norm_values_str_stream;

        // last popped Norm Values
        std::ostringstream last_popped_norm_values_str_stream;

        // per queue elements distribution
        std::ostringstream per_queue_elements_distribution;

        // per queue out of orders
        std::ostringstream per_queue_out_of_orders;

        // get Per Queue Statistics
        for (size_t i = 0; i < lws_sequencer.m_total_number_of_queues; ++i)
        {
            if ( lws_sequencer.m_input_queues_properties[i]->m_pushing_thread_id != QAppNG::ThreadCounter::UNDEFINED_THREAD_ID )
            {
                pushing_thread_id_str_stream << format_as_list_entry( std::to_string( lws_sequencer.m_input_queues_properties[i]->m_pushing_thread_id ) ); // pushing Thread IDs
            }
            else
            {
                pushing_thread_id_str_stream << format_as_list_entry( "Na" );
            }

            current_enqueued_elements_str_stream << format_as_list_entry ( std::to_string( lws_sequencer.m_input_queues[i]->getUsedQueue() )
                    + enclose_in_brackets( format_as_percent( lws_sequencer.m_input_queues[i]->getUsedQueuePercent() ) )
                    + enclose_in_brackets( ( lws_sequencer.isQueueActive(i) ? "A" : "I" ) ) );

            if ( lws_sequencer.m_input_queues_properties[i]->m_last_pushed_norm_value == 0)
            {
                last_pushed_norm_values_str_stream << format_as_list_entry( "" );
            }
            else
            {
                UInt32  sec = static_cast<UInt32>( lws_sequencer.m_input_queues_properties[i]->m_last_pushed_norm_value >> 32 );
                UInt32 nsec = static_cast<UInt32>( lws_sequencer.m_input_queues_properties[i]->m_last_pushed_norm_value & 0xFFFFFFFF );

                last_pushed_norm_values_str_stream << format_as_list_entry( std::to_string(sec) + "." + std::to_string(nsec) );
            }

            if ( lws_sequencer.m_input_queues_properties[i]->m_last_popped_norm_value == 0 )
            {
                last_popped_norm_values_str_stream << format_as_list_entry( "" );
            }
            else
            {
                UInt32  sec = static_cast<UInt32>( lws_sequencer.m_input_queues_properties[i]->m_last_popped_norm_value >> 32 );
                UInt32 nsec = static_cast<UInt32>( lws_sequencer.m_input_queues_properties[i]->m_last_popped_norm_value & 0xFFFFFFFF );

                last_popped_norm_values_str_stream << format_as_list_entry( std::to_string(sec) + "." + std::to_string(nsec) );
            }

            UInt64 per_queue_input_processed_elements = lws_sequencer.m_input_queues_properties[i]->m_number_of_pushed_element +
                lws_sequencer.m_input_queues_properties[i]->m_number_of_input_dropped_element;

            float queue_load_balancing_percentage = per_queue_input_processed_elements > 0 ?
                per_queue_input_processed_elements * 100.00F / total_number_of_input_processed_elements : 0;

            std::string queue_type_string("-");

            switch (lws_sequencer.m_input_queues_properties[i]->m_type)
            {
                case QueueProperties::QueueType::NORMAL:
                    queue_type_string = "N";
                    break;
                case QueueProperties::QueueType::SORTED:
                    queue_type_string = "S";
                    break;
                case QueueProperties::QueueType::EVENT:
                    queue_type_string = "E";
                    break;
                default:
                    break;
            }

            per_queue_elements_distribution << format_as_list_entry( std::to_string( per_queue_input_processed_elements )
                + enclose_in_brackets( format_as_percent( queue_load_balancing_percentage ) )
                + enclose_in_brackets( queue_type_string ) );

            per_queue_out_of_orders << format_as_list_entry( std::to_string( lws_sequencer.m_input_queues_properties[i]->m_number_of_out_of_order_events ) );
        }

        output << string_formatter("PUSHING Thread IDs:")                   << pushing_thread_id_str_stream.str()           << std::endl;
        output << string_formatter("ENQUEUED Elements:")                    << current_enqueued_elements_str_stream.str()   << std::endl;
        output << string_formatter("Per Queue Elements DISTRIBUTION:")      << per_queue_elements_distribution.str()        << std::endl;
        output << string_formatter("Per Queue OUT OF ORDERS:")              << per_queue_out_of_orders.str()                << std::endl;
        output << string_formatter("Per Queue LAST PUSHED Elements Norms:") << last_pushed_norm_values_str_stream.str()     << std::endl;
        output << string_formatter("Per Queue LAST POPPED Elements Norms:") << last_popped_norm_values_str_stream.str()     << std::endl;

        //***************************
        // Global LAST PUSHED/POPPED
        //***************************

        std::string global_last_pushed_string("------------------------");

        if ( lws_sequencer.m_lws_configuration.m_global_last_pushed_norm_value != 0 )
        {
            UInt32  sec = static_cast<UInt32>(lws_sequencer.m_lws_configuration.m_global_last_pushed_norm_value >> 32);
            UInt32 nsec = static_cast<UInt32>(lws_sequencer.m_lws_configuration.m_global_last_pushed_norm_value & 0xFFFFFFFF);

            global_last_pushed_string = std::to_string(sec) + "." + std::to_string(nsec)
                + enclose_in_brackets(std::to_string(lws_sequencer.m_lws_configuration.m_global_last_pushed_queue_index));
        }

        std::string global_last_popped_string("------------------------");

        if ( lws_sequencer.m_lws_configuration.m_global_last_popped_norm_value != 0 )
        {
            UInt32  sec = static_cast< UInt32 >( lws_sequencer.m_lws_configuration.m_global_last_popped_norm_value >> 32 );
            UInt32 nsec = static_cast< UInt32 >( lws_sequencer.m_lws_configuration.m_global_last_popped_norm_value & 0xFFFFFFFF );

            global_last_popped_string = std::to_string(sec) + "." + std::to_string(nsec)
                + enclose_in_brackets( std::to_string( lws_sequencer.m_lws_configuration.m_global_last_popped_queue_index ) );
        }

        output << string_formatter("LAST PUSHED Element Norm:") << global_last_pushed_string << std::endl;
        output << string_formatter("LAST POPPED Element Norm:") << global_last_popped_string << std::endl;


        return output;
    }
} // namespace NG


// --------------------------------------------------------------------------------------------------------------------
// End of file
// --------------------------------------------------------------------------------------------------------------------
