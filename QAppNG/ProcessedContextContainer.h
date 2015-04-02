#ifndef __PROCESSED_CONTEXT_CONTAINER_H___NG
#define __PROCESSED_CONTEXT_CONTAINER_H___NG

#include <cal/time/Timestamp.h>
#include "Ticket.h"

namespace QAppNG
{

    template < typename StatusEnum > class ProcessedContextContainer
    {
    public:

        ProcessedContextContainer( std::shared_ptr< Ticket > a_ticket, UInt32 a_secs, UInt32 a_nsecs, StatusEnum a_status, bool a_isFinished = false )
            : m_startTime( a_secs, a_nsecs ),
            m_lastActivityTime( a_secs, a_nsecs)
        {
            m_ticket = a_ticket;
            m_startTimeSecs = a_secs;
            m_startTimeNsecs = a_nsecs;
            m_lastActivityTimeSecs = a_secs;
            m_lastActivityTimeNsecs = a_nsecs;
            m_status = a_status;
            m_isFinished = a_isFinished;
        };

        virtual ~ProcessedContextContainer()
        {
        };

        std::shared_ptr< Ticket > m_ticket;

        //These timestamps are for Paul's MM Code
        cal::Timestamp m_startTime;
        cal::Timestamp m_lastActivityTime;

        //These timestamps are for Faustino's GMM and SM Code
        UInt32 m_startTimeSecs;
        UInt32 m_startTimeNsecs;
        UInt32 m_lastActivityTimeSecs;
        UInt32 m_lastActivityTimeNsecs;

        bool m_isFinished;
        StatusEnum m_status;

    private:

        ProcessedContextContainer();

    };

}
#endif //__PROCESSED_CONTEXT_CONTAINER_H___NG

