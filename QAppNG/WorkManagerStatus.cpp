//
// C++ Implementation: WorkManagerStatus
//
// Description:
//
//
// Author: Alessandro Della Villa <alessandro.dellavilla@CommProve.com>, 31/10/2008
//
//
//
#include "WorkManagerStatus.h"

// --------------------------------------------------------------------------------------------------------

namespace QAppNG
{
    // --------------------------------------------------------------------------------------------------------

    WorkManagerStatus::WorkManagerStatus( const std::string &status_file_name, fastdelegate::FastDelegate0<std::string> get_status_method )
        : m_get_status_method(get_status_method)
        , m_workmanager_status_fname(status_file_name)
    {
    }

    // --------------------------------------------------------------------------------------------------------

    void WorkManagerStatus::StartTimer( UInt16 status_rate_seconds )
    {
        if (status_rate_seconds > 0)
        {
            m_status_timer.start( status_rate_seconds, fastdelegate::FastDelegate0<void>(this, &WorkManagerStatus::Dump) );
        }
    }

    // --------------------------------------------------------------------------------------------------------

    void WorkManagerStatus::StopTimer()
    {
        m_status_timer.stop();
    }

    // --------------------------------------------------------------------------------------------------------

    void WorkManagerStatus::Dump()
    {
        // Open file in append mode
        if ( !m_status_fp.is_open() )
        {
            m_status_fp.open(m_workmanager_status_fname.c_str(), std::ios::app);
        }

        boost::posix_time::ptime now_time = boost::posix_time::second_clock::local_time();
        boost::posix_time::time_duration time_interval = now_time - m_status_timer.getStartTime();

        UInt16 up_time_days    = UInt16( time_interval.hours() / 24 );
        UInt16 up_time_hours   = UInt16( time_interval.hours() % 24 );
        UInt16 up_time_minutes = UInt16( time_interval.minutes() );
        UInt16 up_time_seconds = UInt16( time_interval.seconds() );

        m_status_fp << "-------------------------------------------------------------" << std::endl;

        m_status_fp << "WorkManager Status: " << now_time << std::endl;

        m_status_fp << "Up since: " 
                    << up_time_days  << "d:"
                    << up_time_hours << "h:"
                    << up_time_minutes << "m:"
                    << up_time_seconds << "s. "
                    << "Started @" << m_status_timer.getStartTime() << std::endl;

        // Get DATA from the GET STATUS METHOD passed to ctor
        m_status_fp << m_get_status_method() << std::endl;

        // Close file
        if ( m_status_fp.is_open() ) m_status_fp.close();
    }
}
// --------------------------------------------------------------------------------------------------------
