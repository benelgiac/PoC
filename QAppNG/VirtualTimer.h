// ===========================================================================
/// @file
/// @brief This file contains the interface for CVirtualTimer class.
///
/// @copyright
/// @history
/// REF#        Who               When        What
/// 1957       Faustino Frechilla 07-May-08   Original development
/// @endhistory
///
// ===========================================================================

#ifndef __VIRTUAL_TIMER_H___NG
#define __VIRTUAL_TIMER_H___NG

#include "assert.h"

namespace QAppNG
{

    class CVirtualTimer
    {
    public:
        /**
         * creates a Virtual Timer object. expireSec and expireNsec indicate how often
         * the delegate will be called
         */
        CVirtualTimer(UInt32 expireSec, UInt32 expireNsec = 0, void * arg = NULL):   
            m_initialised(false),
            m_expireSec(expireSec),
            m_expireNsec(expireNsec),
            m_startSec(0),
            m_startNsec(0),
            m_current_sec(0),
            m_current_nsec(0),
            m_arg(arg)
        {
        };

        ~CVirtualTimer()
        {
        };

        /**
         * assign the delegate which will be called when the virtual timer expires
         */
    	inline void AssignDelegate(fastdelegate::FastDelegate1< void*, void > a_delegate)
        {
            m_delegate = a_delegate;
        };

        /**
         * Update the current time of the virtual timer
         * if the expire time is reached the delegate function is called
         * using the argument passed as parameter in the construction of the object
         */
        inline void Update(UInt32 sec, UInt32 nsec = 0)
        {
            if (m_initialised == false)
            {
                m_startSec  = sec;
                m_startNsec = nsec;
                m_current_sec = sec;
                m_current_nsec = nsec;
                m_initialised = true;
                return;
            }

            //update current time
            if ( (sec > m_current_sec) || 
                 ((sec == m_current_sec) && nsec > m_current_nsec)
                )
            {
                m_current_sec = sec;
                m_current_nsec = nsec;
            }

            if ( (sec > m_startSec) &&
                 ( ( (sec - m_startSec) > m_expireSec ) || 
                   ( ( (sec - m_startSec) == m_expireSec ) && ( (nsec - m_startNsec) >= m_expireNsec ) ) ) )
            {
                m_startSec  = sec;
                m_startNsec = nsec;

                assert(m_delegate);

                m_delegate(m_arg);
            }
        };

        inline UInt32 GetCurrentSec(){return m_current_sec;};
        inline UInt32 GetCurrentNSec(){return m_current_nsec;};

    protected:
        // false if the Start time has not been set yet
        bool m_initialised;

        // expire time
        UInt32 m_expireSec;
        UInt32 m_expireNsec;

        // last time the timer expired
        UInt32 m_startSec;
        UInt32 m_startNsec;

        // current time in term of pdus (can be useful to have a monotonic Pdu timestamp in case of 
        //  out of order pdus as it is in case of Load balancing where you mix local and remote pdus )
        UInt32 m_current_sec;
        UInt32 m_current_nsec;

        // the delegate
        fastdelegate::FastDelegate1< void *, void > m_delegate;

        void * m_arg;

    };

}
#endif
