#ifndef SIMPLEPERIODICTIMER_INCLUDE_NG
#define SIMPLEPERIODICTIMER_INCLUDE_NG
/** ===================================================================================================================
  * @file    SimplePeriodicTimer HEADER FILE
  *
  * @brief   Create a timer running in a new boost Thread
  *
  * @copyright
  *
  * @history
  * REF#        Who                                                              When          What
  * #8060       A. Della Villa, D. Verna, F. Lasagni                             Jan-2013      Original Development
  *
  * @endhistory
  * ===================================================================================================================
  */

// Include STL & BOOST
#include <QAppNG/core.h>

#ifdef __GNUC__
#   pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#   pragma GCC diagnostic ignored "-Wmissing-braces"
#endif

#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#ifdef __GNUC__
#   pragma GCC diagnostic warning "-Wdeprecated-declarations"
#   pragma GCC diagnostic warning "-Wmissing-braces"
#endif


// --------------------------------------------------------------------------------------------------------

namespace QAppNG
{
    class SimplePeriodicTimer
    {
    public:
        SimplePeriodicTimer() : m_timer_is_running(false), m_time_interval_seconds(0) {}
        ~SimplePeriodicTimer()
        {
            stop();
        }

        bool start( UInt16 time_interval_seconds, fastdelegate::FastDelegate0<void> callback_function )
        {
            if ( m_timer_is_running == false && time_interval_seconds > 0 )
            {
                // set interval
                m_time_interval_seconds = time_interval_seconds;

                // set delegate
                m_callback_function = callback_function;

                m_io_service.reset( new boost::asio::io_service );

                // create deadline timer
                m_deadline_timer.reset( new boost::asio::deadline_timer(*m_io_service, boost::posix_time::seconds(time_interval_seconds)) );

                m_start_time = boost::posix_time::second_clock::local_time();
                m_timer_is_running  =   true;

                m_deadline_timer->async_wait( boost::bind(&SimplePeriodicTimer::timerTrigger, this) );

                // call m_io_service.run() from another thread (that will be blocked)
                m_asio_thread.reset( new std::thread( [this] { this->m_io_service->run(); } ) );

                //m_io_service.run();
                return true;
            }

            return false;
        }

        bool stop()
        {
            if (m_timer_is_running) 
            {
                m_timer_is_running = false;

                m_io_service->stop();
                m_deadline_timer.reset();
                m_asio_thread->join();

                // call timer action last time before exiting
                timerTrigger();
            }

            return true;
        }

        const boost::posix_time::ptime& getStartTime() { return m_start_time; }

    private:
        boost::posix_time::ptime m_start_time;
        volatile bool m_timer_is_running;
        UInt16 m_time_interval_seconds;

        fastdelegate::FastDelegate0<void>               m_callback_function;
        std::unique_ptr<boost::asio::io_service>      m_io_service;
        std::unique_ptr<boost::asio::deadline_timer>  m_deadline_timer;

        std::unique_ptr< std::thread > m_asio_thread;

        void timerTrigger()
        {
            // call function
            if (m_callback_function)
            {
                m_callback_function();
            }

            // rearm timer
            if ( m_timer_is_running )
            {
                m_deadline_timer->expires_at(m_deadline_timer->expires_at() + boost::posix_time::seconds(m_time_interval_seconds));
                m_deadline_timer->async_wait( boost::bind(&SimplePeriodicTimer::timerTrigger, this) );
            }
        }
    };
}
#endif // SIMPLEPERIODICTIMER_INCLUDE_NG
// --------------------------------------------------------------------------------------------------------

