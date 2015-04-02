#ifndef QVIRTUALCLOCK_EVO_NG
#define QVIRTUALCLOCK_EVO_NG
/** ===================================================================================================================
  * @file    QVirtualClock HEADER FILE
  *
  * @brief   GsmApp Evolution - code refactoring project
  *          Virtual Clock
  *
  * @copyright
  *
  * @history
  * REF#        Who                                                              When          What
  * #3062       A. Della Villa, A. Manetti, F. Gragnani, R. Buti, S. Luceri      May-2009      Original development
  * #5466       A. Della Villa                                                   Sep-2010      handle out-of-order, time resolution etc.
  * #8060       A. Della Villa, D. Verna, F. Lasagni                             Jan-2013      remove CAL, introduce QVIRTUALCLOCK_TIME_PULSE events
  *
  * @endhistory
  * ===================================================================================================================
  */

// Include STL & BOOST
#include <string>
#include <list>
#include <limits>
#include <chrono>
#include <functional>
#include <memory>
#include <exception>
#include <algorithm>

// Include Ecuba
#include <QAppNG/core.h>
#include <QAppNG/Singleton.h>
#include <QAppNG/SimplePeriodicTimer.h>
#include <QAppNG/QObservable.h>
#include <QAppNG/MultithreadProcessingEntity.h>

namespace QAppNG
{

    // --------------------------------------------------------------------------------------------------------------------

    class QVirtualClockTimePulseEvent : public QObservable
    {
        friend class QVirtualClock;
    public:
        QVirtualClockTimePulseEvent( const UInt32& total_elapsed_seconds )
            : QObservable(QVIRTUALCLOCK_TIME_PULSE)
            , m_trigger_1_second(true) // at least 1 second is passed every time an event is generated!!!
            , m_trigger_5_seconds(false)
            , m_trigger_15_seconds(false)
            , m_trigger_30_seconds(false)
            , m_trigger_1_minute(false)
            , m_trigger_5_minutes(false)
            , m_trigger_15_minutes(false)
            , m_trigger_30_minutes(false)
            , m_trigger_1_hour(false)
            , m_trigger_end_of_time(false)
            , m_total_elapsed_seconds(total_elapsed_seconds)
        {
        }

        // numeric values in seconds
        static const UInt32 TIME_INTERVAL_1_SECOND = 1;
        static const UInt32 TIME_INTERVAL_5_SECONDS = 5;
        static const UInt32 TIME_INTERVAL_15_SECONDS = 15;
        static const UInt32 TIME_INTERVAL_30_SECONDS = 30;
        static const UInt32 TIME_INTERVAL_1_MINUTE = 60;
        static const UInt32 TIME_INTERVAL_5_MINUTES = 300;
        static const UInt32 TIME_INTERVAL_15_MINUTES = 900;
        static const UInt32 TIME_INTERVAL_30_MINUTES = 1800;
        static const UInt32 TIME_INTERVAL_1_HOUR = 3600;

        bool has1SecondTrigger() { return m_trigger_1_second; }
        bool has5SecondsTrigger() { return m_trigger_5_seconds; }
        bool has15SecondsTrigger() { return m_trigger_15_seconds; }
        bool has30SecondsTrigger() { return m_trigger_30_seconds; }
        bool has1MinuteTrigger() { return m_trigger_1_minute; }
        bool has5MinutesTrigger() { return m_trigger_5_minutes; }
        bool has15MinutesTrigger() { return m_trigger_15_minutes; }
        bool has30MinutesTrigger() { return m_trigger_30_minutes; }
        bool has1HourTrigger() { return m_trigger_1_hour; }
        bool hasEndOfTimeTrigger() { return m_trigger_end_of_time; }

        const UInt32 getTotalElapsedSeconds() { return m_total_elapsed_seconds; }

    private:
        bool m_trigger_1_second;
        bool m_trigger_5_seconds;
        bool m_trigger_15_seconds;
        bool m_trigger_30_seconds;
        bool m_trigger_1_minute;
        bool m_trigger_5_minutes;
        bool m_trigger_15_minutes;
        bool m_trigger_30_minutes;
        bool m_trigger_1_hour;
        bool m_trigger_end_of_time;

        // current time
        UInt32 m_total_elapsed_seconds;
    };

    // --------------------------------------------------------------------------------------------------------------------

    class QVirtualClockShutdownEvent : public QObservable
    {
        friend class QVirtualClock;
    public:
        QVirtualClockShutdownEvent()
            : QObservable( QVIRTUALCLOCK_APPLICATION_SHUTDOWN )
        {}
    };

    // --------------------------------------------------------------------------------------------------------------------

    class QVirtualClockStartEvent : public QObservable
    {
        friend class QVirtualClock;
    public:
        QVirtualClockStartEvent()
            : QObservable( QVIRTUALCLOCK_APPLICATION_START )
        {}
    };

    // --------------------------------------------------------------------------------------------------------------------

    class QVirtualClock : public QAppNG::Singleton<QVirtualClock>
    {
    private:
        // define the tolerance used to detect PDUs mis ordering. It depends on the primary sequencer tolerance
        static const UInt32 TIMESTAMP_RESOLUTION_NANOSECONDS = 1;

        // define numeric value for 1 second to be able to sum it to UInt64 timestamp
        static const UInt64 ONE_SECOND = 0x0000000100000000;

    public:
        /**
          *  Update virtual time clock to the value of passed sec and nsec,
          *  the update is done only if the passed timestamp (sec+nsec) is
          *  in the future with respect than the current virtual time value.
          *  Also detect OUT OF ORDER EVENTS using a tolerance.
          */
        inline bool updateTime( const UInt64& virtual_time )
        {
            // #8549: we assume Virtual clock is called from a single thread
            // std::unique_lock<std::recursive_mutex> modify_time_lock(m_modify_time_mutex);

            bool update_time_is_in_the_future( virtual_time > m_virtual_time );
            
            m_update_time_invoked = true;
            
            if ( update_time_is_in_the_future )
            {
                // on first VALID time update we SET m_first_update_time
                if ( m_first_update_time == 0 && m_virtual_time != 0 )
                {
                    m_first_update_time = virtual_time;
                }

                // UPDATE VIRTUAL CLOCK
                m_virtual_time = virtual_time;

                // UPDATE last update time received
                m_last_update_time = virtual_time;

                // OK: update time is in the future
                return true;
            }
            else if ( !update_time_is_in_the_future && m_last_update_time == virtual_time )
            {
                // OK: we received more PDUs with the same timestamp
                return true;
            }
            else if ( !update_time_is_in_the_future && m_last_update_time - virtual_time < TIMESTAMP_RESOLUTION_NANOSECONDS ) 
            {
                // UPDATE last update time received
                m_last_update_time = virtual_time;

                // OK: update time not in the future but in the tolerance interval
                return true;
            }
            else if ( !update_time_is_in_the_future && virtual_time > m_last_update_time )
            {
                // UPDATE last update time received
                m_last_update_time = virtual_time;

                // OK: out of sequence with respect to virtual clock but it is in sequence with respect last pdu: SO WE ALREADY REPORTED THE OUT OF SEQUENCE EVENT
                return true;
            }
            else
            {
                // UPDATE last update time received from virtual clock
                m_last_update_time = virtual_time;

                // OUT OF SEQUENCE EVENT
                return false;
            }
        }

        /**
          *  Set virtual time clock to the given value (no check is done)
          */
        void setTime( const UInt32& sec, const UInt32& nanosec )
        {
            // #8549: we assume Virtual clock is called from a single thread
            // std::unique_lock<std::recursive_mutex> modify_time_lock(m_modify_time_mutex);

            m_virtual_time = convertTimestampToUInt64(sec, nanosec);
        }

        //  if a dispatch function is provided, QVirtualClock will feed it with time pulse events
        void start( std::function< bool( std::unique_ptr<QAppNG::QObservable>& ) > event_target_dispatch_function = std::function< bool( std::unique_ptr<QAppNG::QObservable>& ) >() )
        {

            if ( event_target_dispatch_function )
            {
                registerTimePulseEventsReceivingFunction( "QVirtualClockStart",event_target_dispatch_function );
            }

            if ( !m_timer_thread_running )
            {
                m_exit = false;

            // START timer thread
            m_timer_thread.reset( new std::thread( [this] { this->timerThreadLoop(); } ) );

            // WAIT until timer thread has been started (it is useful to be sure that INIT event has been dispatched)
            while ( !m_timer_thread_running )
            {
                std::this_thread::sleep_for(std::chrono::microseconds(1));
            }
        }

        }

        UInt32 getTotalElapsedSeconds()
        {
            return m_total_elapsed_seconds;
        }
    
        UInt64 getTime()
        {
            return m_virtual_time;
        }

        UInt32 getTimeSec()
        {
            return static_cast<UInt32>( m_virtual_time >> 32 );
        }

        UInt32 getTimeNanoSec()
        {
            return static_cast<UInt32>( m_virtual_time );
        }

        std::string getTimeString()
        {
            return convertTimestampToHumanString( m_virtual_time );
        }

        UInt32 getTimeDifferenceSec( const UInt32& sec )
        {
            UInt32 virtual_time_sec = getTimeSec();

            if ( sec > virtual_time_sec )
            {
                return (sec - virtual_time_sec);
            }
            else
            {
                return (virtual_time_sec - sec);
            }
        }

        UInt32 getTimeDifferenceNanoSec( const UInt32& nanosec )
        {
            UInt32 virtual_time_nanosec = getTimeNanoSec();

            if ( nanosec > virtual_time_nanosec )
            {
                return (nanosec - virtual_time_nanosec);
            }
            else
            {
                return (virtual_time_nanosec - nanosec);
            }
        }

        UInt64 convertTimestampToUInt64( const UInt32& time_sec, const UInt32& time_nanosec )
        {
            return ( static_cast<UInt64>(time_sec) << 32 ) | ( static_cast<UInt64>(time_nanosec) );
        }

        std::string convertTimestampToHumanString( const UInt32& time_sec, const UInt32& time_nanosec )
        {
            if (time_sec == 0) return "-";

            boost::gregorian::date initial_date(1970, 1, 1); // all was born in 70s...
            boost::posix_time::ptime current_time( initial_date, boost::posix_time::seconds( time_sec ) + boost::posix_time::microsec(  static_cast<UInt32>(time_nanosec / 1000) ) );

            /* #8338
            // iso_extended_string has format "2013-03-12T12:27:24" but we want this format: "2013/03/12 12:27:24" so we need to change '-' and 'T'
            std::string timestampString = boost::posix_time::to_iso_extended_string(current_time);
            timestampString[4] = '/'; timestampString[7] = '/'; timestampString[10] = ' ';  // change characters

            return timestampString;
            //oldver return boost::posix_time::to_simple_string(current_time);
            */

            // #8338 we decided to use directly the ISO extended string
            return boost::posix_time::to_iso_extended_string(current_time);
        }

        std::string convertTimestampToHumanString( const UInt64& unix_time_stamp )
        {
            UInt32 time_sec = static_cast<UInt32>( unix_time_stamp >> 32 );
            UInt32 time_nanosec = static_cast<UInt32>( unix_time_stamp & 0x00000000FFFFFFFF );

            return convertTimestampToHumanString( time_sec, time_nanosec );
        }

        // CTOR
        QVirtualClock()
            : QAppNG::Singleton<QVirtualClock>()
            , m_virtual_time(0)
            , m_last_update_time(0)
            , m_update_time_invoked(false)
            , m_first_update_time(0)
            , m_delta_between_calculated_virtual_time_and_feeding(0)
            , m_calculated_virtual_time(0)
            , m_exit(true)
            , m_timer_thread_running(false)
            , m_total_elapsed_seconds(0)
        {
            // TimerLoopThread is started by "start" method
        }

        // DTOR
        ~QVirtualClock()
        {
            shutdown();
        }

        void shutdown();

        // These methods are used to register processing function of emitted events.
        // Registration should be done starting from last chain ring in order to propagate
        // events, including shutdown, from the first to the last one
        void registerTimePulseEventsReceivingFunction( const std::string& function_string_id, std::function< bool( std::shared_ptr< QAppNG::QObservable >& ) > time_pulse_event_receiving_function );
        void registerTimePulseEventsReceivingFunction( const std::string& function_string_id, std::function< bool( std::unique_ptr<QAppNG::QObservable>& ) > time_pulse_event_receiving_function );

        // Unregister processing events function
        void unRegisterTimePulseEventsReceivingFunction( const std::string& function_string_id );
    private:
        // virtual time
        UInt64 m_virtual_time;

        // last time used to update m_virtual_time
        UInt64 m_last_update_time;
        
        // bool that is set to true by each updateTime invocation
        volatile bool m_update_time_invoked;

        // used to sync time calculated by QVirtualClock with time of feeding AND to be able to pulse also with no feeding
        UInt64 m_first_update_time;

        // used to sync
        UInt64 m_delta_between_calculated_virtual_time_and_feeding;

        // Virtual time calculated from elapsed seconds
        UInt64 m_calculated_virtual_time;

        // map of registered functions to propagate emitted events
        // We have two versions of them, using unique_ptr to time events for RxSequencer and std::shared_ptr for othe LWSs
        std::list< std::pair< std::string, std::function<bool( std::shared_ptr<QAppNG::QObservable>& )> > >   m_registered_shared_ptr_time_pulse_events_receiving_functions;
        std::list< std::pair< std::string, std::function<bool( std::unique_ptr<QAppNG::QObservable>& )> > >   m_registered_unique_ptr_time_pulse_events_receiving_functions;

        // used when adding receivers for PULSE EVENTS and when casting PULSE EVENTS
        std::recursive_mutex m_time_pulse_events_mutex;

        // used on updateTime() and setTime()
        //std::recursive_mutex m_modify_time_mutex;

        // thread used as timer to count elapsed seconds
        volatile bool m_exit;
        volatile bool m_timer_thread_running;
        std::unique_ptr<std::thread> m_timer_thread;

        // count number of timer pulse (second passed since QVirtualClock has been started)
        UInt32 m_total_elapsed_seconds;

        void timerThreadLoop()
        {
            m_timer_thread_running = true;
            std::chrono::high_resolution_clock::time_point sleep_time;
            std::chrono::duration<double> sleep_duration_seconds;

            // enter INFINITE thread loop
            while ( !m_exit )
            {
                sleep_time = std::chrono::high_resolution_clock::now( );
                std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
                sleep_duration_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now( ) - sleep_time);
                timerCallBack( sleep_duration_seconds );
            }

            propagateShutdownEventsToRegistered();

            m_timer_thread_running = false;
        }

        void timerCallBack( std::chrono::duration<double>& elapsed_sleep_seconds );

        // Propagate events passing them to registered functions, starting from the last added one
        void propagateTimePulseEventToRegistered();
        void propagateShutdownEventsToRegistered();
    };

} // namespace

#endif //QVIRTUALCLOCK
// --------------------------------------------------------------------------------------------------------------------
// End of file


