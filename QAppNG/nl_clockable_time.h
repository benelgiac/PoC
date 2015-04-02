#ifndef __NL_CLOCKABLE_TIME_H___NG
#define __NL_CLOCKABLE_TIME_H___NG

#include <QAppNG/core.h>

namespace QAppNG
{

    class CNlClockableTime
    {
    public:
        CNlClockableTime();
        ~CNlClockableTime();
        time_t GetCurrentSeconds();
        void UpdateCurrentSeconds(time_t cur_time);
    protected:
        time_t m_cur_sec;
    };

}
#endif

