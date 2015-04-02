#ifndef PeriodicTimer_H
#define PeriodicTimer_H

#include "core.h"
#include "QAppNG/Singleton.h"

#include <thread>
#include <mutex>
#include <condition_variable>
#include <algorithm>
#include <functional>
#include <chrono>
#include <unordered_map>
#include <set>
#include <cstdint>

#ifndef _MSC_VER
#define NOEXCEPT noexcept
#else
#define NOEXCEPT
#endif

namespace QAppNG
{
    class PeriodicTimer : public QAppNG::Singleton<PeriodicTimer>
    {
    public:
        typedef UInt64                      timer_id;
        typedef std::function<void(timer_id)> handler_type;
    
        PeriodicTimer();
        ~PeriodicTimer();
    
        timer_id create(uint64_t when, uint64_t period, const handler_type& handler);
        timer_id create(uint64_t when, uint64_t period, handler_type&& handler);
    
        bool destroy(timer_id id);
        bool exists(timer_id id);
        
    private:
        
        typedef std::chrono::system_clock       Clock;
        typedef std::chrono::time_point<Clock>  Timestamp;
        typedef std::chrono::milliseconds       Duration;
        
        struct Timer
        {
            Timer(timer_id id = 0)
                : id(id)
                , running(false)
            {
            }
    
            template<typename Tfunction>
            Timer(timer_id id, Timestamp next, Duration period, Tfunction&& handler) NOEXCEPT
                : id(id)
                , next(next)
                , period(period)
                , handler(std::forward<Tfunction>(handler))
                , running(false)
            {
            }
    
            Timer(Timer const& r) = delete;
    
            Timer(Timer&& r) NOEXCEPT
                : id(r.id)
                , next(r.next)
                , period(r.period)
                , handler(std::move(r.handler))
                , running(r.running)
            {
            }
    
            Timer& operator=(Timer const& r) = delete;
    
            Timer& operator=(Timer&& r)
            {
                if (this != &r)
                {
                    id = r.id;
                    next = r.next;
                    period = r.period;
                    handler = std::move(r.handler);
                    running = r.running;
                }
                return *this;
            }
    
            timer_id        id;
            Timestamp       next;
            Duration        period;
            handler_type    handler;
            bool            running;
        };
        typedef std::unordered_map<timer_id, Timer> TimerMap;
        
        // Comparison functor to sort the timer "queue" by Timer::next
        struct NextActiveComparator
        {
            bool operator()(const Timer &a, const Timer &b) const
            {
                return a.next < b.next;
            }
        };
        NextActiveComparator comparator;
    
        // Queue is a set of references to Timer objects, sorted by next
        typedef std::reference_wrapper<Timer>                   QueueValue;
        typedef std::multiset<QueueValue, NextActiveComparator> Queue;
        Queue                                                   m_queue;
        
        TimerMap    m_active;
        timer_id    m_nextId;
        
        std::mutex                              m_sync;
        typedef std::unique_lock<std::mutex>    ScopedLock;
        std::condition_variable                 m_wakeUp;
    
        // Thread and exit flag
        std::thread m_worker;
        bool        m_done;

	private:
        
        void        threadStart();
        timer_id    createImpl(Timer&& item);

    }; // class PeriodicTimer

} // namespace QAppNG

#include "detail/PeriodicTimerImpl.h"

#endif // PeriodicTimer_H
