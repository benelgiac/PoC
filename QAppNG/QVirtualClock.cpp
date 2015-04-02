#include "QVirtualClock.h"
#include <QAppNG/core.h>

using namespace QAppNG;

void QVirtualClock::timerCallBack( std::chrono::duration<double>& elapsed_sleep_seconds )
{
    UInt32 elapsed_sec = static_cast<UInt32>(elapsed_sleep_seconds.count( ));
    
    if (elapsed_sec < 1)
    {
        return; //this should never happen because boost::sleep guarantees it.
    }
    
    if (!m_update_time_invoked)
    {
        //This can happen if pdu reception is not yet started, or if for some reason pdus have not
        // been received from one timerCallBack invocation to another, or if pdus have stopped completely.
        //The solution is to keep virtual time alive
        UInt32 elapsed_msec_fraction   = (static_cast<UInt32>(elapsed_sleep_seconds.count( ) *1000000)) % 1000000;
        UInt64 virtual_time_correction = (static_cast<UInt64>(elapsed_sec) << 32 | elapsed_msec_fraction);
        
        m_virtual_time+=virtual_time_correction;
    }

    for (UInt32 i = 0; i < elapsed_sec; i++)
    {
        ++m_total_elapsed_seconds;

        propagateTimePulseEventToRegistered();
    }

    m_update_time_invoked = false;
}

// --------------------------------------------------------------------------------------------------------------------

void QVirtualClock::shutdown()
{
    if (m_exit == true)
    {
        return;
    }

    m_exit = true;

    m_timer_thread->join();
}

// --------------------------------------------------------------------------------------------------------------------

void QVirtualClock::registerTimePulseEventsReceivingFunction(  const std::string& function_string_id, std::function<bool( std::shared_ptr< QAppNG::QObservable >& )> time_pulse_receiving_function_legacy )
{
    std::unique_lock<std::recursive_mutex> time_pulse_events_lock( m_time_pulse_events_mutex );

    auto begin_itr = m_registered_shared_ptr_time_pulse_events_receiving_functions.begin();

    auto end_itr   = m_registered_shared_ptr_time_pulse_events_receiving_functions.end();

    for ( auto it = begin_itr; it != end_itr; ++it )
    {
        if ( it->first == function_string_id )
        {
            throw std::runtime_error( "QVirtualClock - Cannot register two Time Events Propagation Function With the Same Name: " + function_string_id );
        }
    }

    m_registered_shared_ptr_time_pulse_events_receiving_functions.push_front( std::make_pair( function_string_id, time_pulse_receiving_function_legacy ) );
}

// --------------------------------------------------------------------------------------------------------------------

void QVirtualClock::registerTimePulseEventsReceivingFunction(  const std::string& function_string_id, std::function< bool( std::unique_ptr< QAppNG::QObservable >& ) > time_pulse_receiving_function )
{
    std::unique_lock<std::recursive_mutex> time_pulse_events_lock(m_time_pulse_events_mutex);

    auto begin_itr = m_registered_unique_ptr_time_pulse_events_receiving_functions.begin();

    auto end_itr = m_registered_unique_ptr_time_pulse_events_receiving_functions.end();

    for (auto it = begin_itr; it != end_itr; ++it)
    {
        if (it->first == function_string_id)
        {
            throw std::runtime_error("QVirtualClock - Cannot register two Time Events Propagation Function With the Same Name: " + function_string_id);
        }
    }

    m_registered_unique_ptr_time_pulse_events_receiving_functions.push_front( std::make_pair( function_string_id, time_pulse_receiving_function ) );
}

// --------------------------------------------------------------------------------------------------------------------

void QVirtualClock::unRegisterTimePulseEventsReceivingFunction( const std::string& function_string_id )
{
    std::unique_lock<std::recursive_mutex> time_pulse_events_lock( m_time_pulse_events_mutex );

    auto begin_itr_1 = m_registered_unique_ptr_time_pulse_events_receiving_functions.begin();

    auto end_itr_1 = m_registered_unique_ptr_time_pulse_events_receiving_functions.end();

    for (auto it = begin_itr_1; it != end_itr_1; ++it)
    {
        if (it->first == function_string_id)
        {
            m_registered_unique_ptr_time_pulse_events_receiving_functions.erase(it);

            return;
        }
    }

    auto begin_itr_2 = m_registered_shared_ptr_time_pulse_events_receiving_functions.begin();

    auto end_itr_2 = m_registered_shared_ptr_time_pulse_events_receiving_functions.end();

    for (auto it = begin_itr_2; it != end_itr_2; ++it)
    {
        if (it->first == function_string_id)
        {
            m_registered_shared_ptr_time_pulse_events_receiving_functions.erase(it);

            return;
        }
    }
}

// --------------------------------------------------------------------------------------------------------------------

void QVirtualClock::propagateTimePulseEventToRegistered()
{
    std::unique_lock<std::recursive_mutex> time_pulse_events_lock(m_time_pulse_events_mutex);

    for ( auto& registered_function : m_registered_unique_ptr_time_pulse_events_receiving_functions )
    {
        // CREATE event and set timer_pulses
        std::unique_ptr<QVirtualClockTimePulseEvent> time_pulse_event( new QVirtualClockTimePulseEvent( m_total_elapsed_seconds ) );
        time_pulse_event->setQObservableTimestamp(m_virtual_time);
        time_pulse_event->setQObservableRoutingKey(QAppNG::BROADCAST_ROUTING_KEY_VALUE);

        if (m_total_elapsed_seconds % QVirtualClockTimePulseEvent::TIME_INTERVAL_5_SECONDS == 0) time_pulse_event->m_trigger_5_seconds = true;
        if (m_total_elapsed_seconds % QVirtualClockTimePulseEvent::TIME_INTERVAL_15_SECONDS == 0) time_pulse_event->m_trigger_15_seconds = true;
        if (m_total_elapsed_seconds % QVirtualClockTimePulseEvent::TIME_INTERVAL_30_SECONDS == 0) time_pulse_event->m_trigger_30_seconds = true;
        if (m_total_elapsed_seconds % QVirtualClockTimePulseEvent::TIME_INTERVAL_1_MINUTE == 0) time_pulse_event->m_trigger_1_minute = true;
        if (m_total_elapsed_seconds % QVirtualClockTimePulseEvent::TIME_INTERVAL_5_MINUTES == 0) time_pulse_event->m_trigger_5_minutes = true;
        if (m_total_elapsed_seconds % QVirtualClockTimePulseEvent::TIME_INTERVAL_15_MINUTES == 0) time_pulse_event->m_trigger_15_minutes = true;
        if (m_total_elapsed_seconds % QVirtualClockTimePulseEvent::TIME_INTERVAL_30_MINUTES == 0) time_pulse_event->m_trigger_30_minutes = true;
        if (m_total_elapsed_seconds % QVirtualClockTimePulseEvent::TIME_INTERVAL_1_HOUR == 0) time_pulse_event->m_trigger_1_hour = true;

        // cast to observable
        std::unique_ptr<QAppNG::QObservable> qobservable( std::move( time_pulse_event ) );

        // DISPATCH event
        registered_function.second(qobservable);
    }

    for (auto& registered_function : m_registered_shared_ptr_time_pulse_events_receiving_functions)
    {
        // CREATE event and set timer_pulses
        std::shared_ptr<QVirtualClockTimePulseEvent> time_pulse_event(new QVirtualClockTimePulseEvent(m_total_elapsed_seconds));
        time_pulse_event->setQObservableTimestamp(m_virtual_time);
        time_pulse_event->setQObservableRoutingKey(QAppNG::BROADCAST_ROUTING_KEY_VALUE);

        if (m_total_elapsed_seconds % QVirtualClockTimePulseEvent::TIME_INTERVAL_5_SECONDS == 0) time_pulse_event->m_trigger_5_seconds = true;
        if (m_total_elapsed_seconds % QVirtualClockTimePulseEvent::TIME_INTERVAL_15_SECONDS == 0) time_pulse_event->m_trigger_15_seconds = true;
        if (m_total_elapsed_seconds % QVirtualClockTimePulseEvent::TIME_INTERVAL_30_SECONDS == 0) time_pulse_event->m_trigger_30_seconds = true;
        if (m_total_elapsed_seconds % QVirtualClockTimePulseEvent::TIME_INTERVAL_1_MINUTE == 0) time_pulse_event->m_trigger_1_minute = true;
        if (m_total_elapsed_seconds % QVirtualClockTimePulseEvent::TIME_INTERVAL_5_MINUTES == 0) time_pulse_event->m_trigger_5_minutes = true;
        if (m_total_elapsed_seconds % QVirtualClockTimePulseEvent::TIME_INTERVAL_15_MINUTES == 0) time_pulse_event->m_trigger_15_minutes = true;
        if (m_total_elapsed_seconds % QVirtualClockTimePulseEvent::TIME_INTERVAL_30_MINUTES == 0) time_pulse_event->m_trigger_30_minutes = true;
        if (m_total_elapsed_seconds % QVirtualClockTimePulseEvent::TIME_INTERVAL_1_HOUR == 0) time_pulse_event->m_trigger_1_hour = true;

        // cast to observable
        std::shared_ptr<QAppNG::QObservable> qobservable = std::static_pointer_cast<QAppNG::QObservable>(time_pulse_event);

        // DISPATCH event
        registered_function.second(qobservable);
    }
}

// --------------------------------------------------------------------------------------------------------------------

void QVirtualClock::propagateShutdownEventsToRegistered()
{
    // Propagate event through registered Functions
    std::unique_lock<std::recursive_mutex> time_pulse_events_lock(m_time_pulse_events_mutex);

    for ( auto& registered_function : m_registered_unique_ptr_time_pulse_events_receiving_functions )
    {
        // CREATE event and set all time flags to true in order to trigger All time driven activities
        std::unique_ptr< QVirtualClockTimePulseEvent > time_pulse_event( new QVirtualClockTimePulseEvent( m_total_elapsed_seconds ) );

        time_pulse_event->m_trigger_5_seconds = true;
        time_pulse_event->m_trigger_15_seconds = true;
        time_pulse_event->m_trigger_30_seconds = true;
        time_pulse_event->m_trigger_1_minute = true;
        time_pulse_event->m_trigger_5_minutes = true;
        time_pulse_event->m_trigger_15_minutes = true;
        time_pulse_event->m_trigger_30_minutes = true;
        time_pulse_event->m_trigger_1_hour = true;
        time_pulse_event->m_trigger_end_of_time = true;

        time_pulse_event->setQObservableRoutingKey(QAppNG::BROADCAST_ROUTING_KEY_VALUE);

        // SET TIMESTAMP TO MAXIMUM SECONDS VALUE minus 2 sec
        UInt64 max_timestamp_seconds_value = std::numeric_limits< UInt32 >::max() - 1;
        max_timestamp_seconds_value <<= 32;

        time_pulse_event->setQObservableTimestamp(max_timestamp_seconds_value);

        // cast to observable
        std::unique_ptr< QAppNG::QObservable > qobservable( std::move( time_pulse_event ) );

        // DISPATCH event
        registered_function.second(qobservable);
    }

    for ( auto& registered_function : m_registered_unique_ptr_time_pulse_events_receiving_functions )
    {
        // CREATE shutdown event
        std::unique_ptr< QVirtualClockShutdownEvent > shutdown_event( new QVirtualClockShutdownEvent );

        // SET BROADCAST routing key
        shutdown_event->setQObservableRoutingKey(QAppNG::BROADCAST_ROUTING_KEY_VALUE);

        // SET TIMESTAMP TO MAXIMUM SECONDS VALUE
        UInt64 max_timestamp_seconds_value = std::numeric_limits< UInt32 >::max();
        max_timestamp_seconds_value <<= 32;

        shutdown_event->setQObservableTimestamp(max_timestamp_seconds_value);

        // cast to observable
        std::unique_ptr< QAppNG::QObservable > qobservable( std::move( shutdown_event ) );

        // DISPATCH event
        registered_function.second(qobservable);
    }

    for (auto& registered_function : m_registered_shared_ptr_time_pulse_events_receiving_functions )
    {
        // CREATE event and set all time flags to true in order to trigger All time driven activities
        std::shared_ptr< QVirtualClockTimePulseEvent > time_pulse_event(new QVirtualClockTimePulseEvent(m_total_elapsed_seconds));

        time_pulse_event->m_trigger_5_seconds = true;
        time_pulse_event->m_trigger_15_seconds = true;
        time_pulse_event->m_trigger_30_seconds = true;
        time_pulse_event->m_trigger_1_minute = true;
        time_pulse_event->m_trigger_5_minutes = true;
        time_pulse_event->m_trigger_15_minutes = true;
        time_pulse_event->m_trigger_30_minutes = true;
        time_pulse_event->m_trigger_1_hour = true;
        time_pulse_event->m_trigger_end_of_time = true;

        time_pulse_event->setQObservableRoutingKey(QAppNG::BROADCAST_ROUTING_KEY_VALUE);

        // SET TIMESTAMP TO MAXIMUM SECONDS VALUE minus 2 sec
        UInt64 max_timestamp_seconds_value = std::numeric_limits< UInt32 >::max() - 1;
        max_timestamp_seconds_value <<= 32;

        time_pulse_event->setQObservableTimestamp(max_timestamp_seconds_value);

        // cast to observable
        std::shared_ptr< QAppNG::QObservable > qobservable = std::static_pointer_cast<QAppNG::QObservable>(time_pulse_event);

        // DISPATCH event
        registered_function.second(qobservable);
    }

    for (auto& registered_function : m_registered_shared_ptr_time_pulse_events_receiving_functions )
    {
        // CREATE shutdown event
        std::shared_ptr< QVirtualClockShutdownEvent >
            shutdown_event(new QVirtualClockShutdownEvent);

        // SET BROADCAST routing key
        shutdown_event->setQObservableRoutingKey(QAppNG::BROADCAST_ROUTING_KEY_VALUE);

        // SET TIMESTAMP TO MAXIMUM SECONDS VALUE
        UInt64 max_timestamp_seconds_value = std::numeric_limits< UInt32 >::max();
        max_timestamp_seconds_value <<= 32;

        shutdown_event->setQObservableTimestamp(max_timestamp_seconds_value);

        // cast to observable
        std::shared_ptr< QAppNG::QObservable > qobservable = std::static_pointer_cast<QAppNG::QObservable>(shutdown_event);

        // DISPATCH event
        registered_function.second(qobservable);
    }
}

// --------------------------------------------------------------------------------------------------------------------
