#include <QAppNG/nl_clockable_time.h>

namespace QAppNG
{


    CNlClockableTime::CNlClockableTime()
    {
       m_cur_sec = 0;
    }

    CNlClockableTime::~CNlClockableTime()
    {
    }

    time_t CNlClockableTime::GetCurrentSeconds()
    {
       return m_cur_sec;
    }

    void CNlClockableTime::UpdateCurrentSeconds(time_t cur_time)
    {
       m_cur_sec = cur_time;
    }


}
