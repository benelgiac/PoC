#include "LightWeightSequencerEvo.h"

namespace QAppNG
{
    // Static Members Definition
    const size_t LightWeightSequencerConfiguration::DEFAULT_INPUT_QUEUE_SIZE = 10000;
    const size_t LightWeightSequencerConfiguration::DEFAULT_MAX_CONSUMABLE_PER_LOOP = 9999;
    const UInt8  LightWeightSequencerConfiguration::DEFAULT_MAX_FILLING_PERCENT_BEFORE_STOP_WAITING_EMPTY_QUEUES = 50;
    const UInt64 LightWeightSequencerConfiguration::DEFAULT_MAX_NORM_DIFFERENCE_BEFORE_STOP_WAITING_EMPTY_QUEUES = 5000000000;
    const UInt8  LightWeightSequencerConfiguration::DEFAULT_SORTED_QUEUE_MINIMUN_FILLUP_BEFORE_SORTING_PERCENT = 10;
    const UInt8  LightWeightSequencerConfiguration::DEFAULT_SORTED_QUEUE_DISORDER_TOLLERANCE_PERCENT = 5;
    const UInt16 LightWeightSequencerConfiguration::DEFAULT_THREAD_IDLE_SLEEP_TIME = 1;

    LightWeightSequencerConfiguration::LightWeightSequencerConfiguration()
        : m_input_queues_size( DEFAULT_INPUT_QUEUE_SIZE )
        , m_input_queues_overload_strategy( LwsInputQueuesOverloadStrategy::eIfOverloadDropInInput )
        , m_max_consumables_per_loop( DEFAULT_MAX_CONSUMABLE_PER_LOOP )
        , m_max_filling_percent_before_stop_waiting_empty_queues( DEFAULT_MAX_FILLING_PERCENT_BEFORE_STOP_WAITING_EMPTY_QUEUES )
        , m_max_norm_difference_before_stop_waiting_empty_queues( DEFAULT_MAX_NORM_DIFFERENCE_BEFORE_STOP_WAITING_EMPTY_QUEUES )
        , m_sorted_queue_minimum_fillup_before_sorting_percent( DEFAULT_SORTED_QUEUE_MINIMUN_FILLUP_BEFORE_SORTING_PERCENT )
        , m_sorted_queue_disorder_tollerance_percent( DEFAULT_SORTED_QUEUE_DISORDER_TOLLERANCE_PERCENT )
        , m_thread_idle_sleep_time( DEFAULT_THREAD_IDLE_SLEEP_TIME )
        , m_global_last_pushed_norm_value( 0 )
        , m_global_last_pushed_norm_prop( SequenceableNormProperties::UNDEFINED_NORM )
        , m_global_last_pushed_queue_index( 0 )
        , m_global_last_popped_norm_value( 0 )
        , m_global_last_popped_norm_prop( SequenceableNormProperties::UNDEFINED_NORM )
        , m_global_last_popped_queue_index( 0 )
        , m_global_number_of_out_of_order_events(0)
        , m_global_number_of_wating_for_empty_queues_events(0)
        , m_global_number_of_overwait_events(0)
        , m_number_of_processed_elements_before_sleeping(0)
        , m_global_number_of_extractor_thread_sleep_events(0)
        , m_global_number_of_sorter_thread_sleep_events(0)
        , m_global_number_of_popped_elements(0)
        , m_global_number_of_output_dropped_elements(0)
    {
        // sanity check parameters
        assert(m_sorted_queue_minimum_fillup_before_sorting_percent <= 100);
        assert(m_sorted_queue_minimum_fillup_before_sorting_percent > m_sorted_queue_disorder_tollerance_percent);
    }

    // --------------------------------------------------------------------------------------------------------------------

    QueueProperties::QueueProperties( QueueType type )
        : m_type( type )
        , m_is_active( false )
        , m_last_pushed_norm_value( 0 )
        , m_last_pushed_norm_prop( SequenceableNormProperties::UNDEFINED_NORM )
        , m_last_popped_norm_value( 0 )
        , m_last_popped_norm_prop( SequenceableNormProperties::UNDEFINED_NORM )
        , m_pushing_thread_id( ThreadCounter::UNDEFINED_THREAD_ID )
        , m_number_of_pushed_element( 0 )
        , m_number_of_input_dropped_element( 0 )
        , m_number_of_out_of_order_events( 0 )
    {
    }

    // --------------------------------------------------------------------------------------------------------------------
    //  Free functions
    // --------------------------------------------------------------------------------------------------------------------

    std::ostream& operator<<(std::ostream& output, const SequenceableNormProperties& norm_property)
    {
        switch (norm_property)
        {
            case QAppNG::SequenceableNormProperties::UNDEFINED_NORM:
                output << "-";
                break;
            case QAppNG::SequenceableNormProperties::ELEMENT_NORM_VALUE:
                output << "NormedElem";
                break;
            case QAppNG::SequenceableNormProperties::EVENT_NORM_VALUE:
                output << "NormedEvent";
                break;
            case QAppNG::SequenceableNormProperties::ELEMENT_TO_INSTANT_PASS:
            case QAppNG::SequenceableNormProperties::EVENT_TO_INSTANT_PASS:
                output << "InstantPass";
                break;
            case QAppNG::SequenceableNormProperties::ELEMENT_TO_INSTANT_DROP:
            case QAppNG::SequenceableNormProperties::EVENT_TO_INSTANT_DROP:
                output << "ToBeDropped";
                break;
            default:
                assert(0);
                break;
        }

        return output;
    }

} // namespace QAppNG

// --------------------------------------------------------------------------------------------------------------------
// End of file
// --------------------------------------------------------------------------------------------------------------------
