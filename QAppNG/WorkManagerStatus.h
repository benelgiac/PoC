//
// C++ Interface: WorkManagerStatus
//
// Description:
//
//
// Author: Alessandro Della Villa <alessandro.dellavilla@CommProve.com>, 31/10/2008
//
//
//
#ifndef WORKMANAGERSTATUS_INCLUDE_NG
#define WORKMANAGERSTATUS_INCLUDE_NG

// --------------------------------------------------------------------------------------------------------

// Include STL & BOOST
#include <string>
#include <fstream>

// other Includes
#include "core.h"
#include "SimplePeriodicTimer.h"

// --------------------------------------------------------------------------------------------------------

namespace QAppNG
{
    class WorkManagerStatus
    {
    public:
        WorkManagerStatus( const std::string &status_file_name, fastdelegate::FastDelegate0<std::string> get_status_method );

        void StartTimer( UInt16 status_rate_seconds = 5 );
        void StopTimer();

    private:
        // dump function called by the Timer
        void Dump();

        // external status method to call
        fastdelegate::FastDelegate0<std::string> m_get_status_method;

        std::ofstream m_status_fp;
        std::string   m_workmanager_status_fname;

        // timer to control status dump
        SimplePeriodicTimer m_status_timer;
    };
}

// --------------------------------------------------------------------------------------------------------
#endif //WORKMANAGERSTATUS_INCLUDE_NG
