#if !defined ( PeriodicTimerImpl_H )
#define PeriodicTimerImpl_H

namespace QAppNG
{
    inline PeriodicTimer::PeriodicTimer()
        : m_queue(comparator)
        , m_nextId(1)
        , m_done(false)
    {
        ScopedLock lock(m_sync);
        m_worker = std::thread(std::bind(&PeriodicTimer::threadStart, this));
    }
    
    inline PeriodicTimer::~PeriodicTimer()
    {
        ScopedLock lock(m_sync);
        m_done = true;
        m_wakeUp.notify_all();
        lock.unlock();
        m_worker.join();
    }
    
    inline PeriodicTimer::timer_id PeriodicTimer::create(uint64_t msFromNow, uint64_t msPeriod, const std::function<void(timer_id)> &handler)
    {
        return createImpl(Timer(0, Clock::now() + Duration(msFromNow), Duration(msPeriod), handler));
    }
    
    inline PeriodicTimer::timer_id PeriodicTimer::create(uint64_t msFromNow, uint64_t msPeriod, std::function<void(timer_id)>&& handler)
    {
        return createImpl(Timer(0, Clock::now() + Duration(msFromNow), Duration(msPeriod), std::move(handler)));
    }
    
    inline PeriodicTimer::timer_id PeriodicTimer::createImpl(Timer&& item)
    {
        ScopedLock lock(m_sync);
        item.id = m_nextId++;
        auto iter = m_active.insert(std::make_pair(item.id, std::move(item)));
        m_queue.insert(iter.first->second);
        m_wakeUp.notify_all();
        return item.id;
    }
    
    inline bool PeriodicTimer::destroy(timer_id id)
    {
        ScopedLock lock(m_sync);
        auto i = m_active.find(id);
        if (i == m_active.end())
        {
            return false;
        }
        else if (i->second.running)
        {
            // A callback is in progress for this Timer,
            // so flag it for deletion in the m_worker
            i->second.running = false;
        }
        else
        {
            m_queue.erase(std::ref(i->second));
            m_active.erase(i);
        }
    
        m_wakeUp.notify_all();
        return true;
    }
    
    inline bool PeriodicTimer::exists(timer_id id)
    {
        ScopedLock lock(m_sync);
        return m_active.find(id) != m_active.end();
    }

} // namespace QAppNG

#endif // PeriodicTimerImpl_H

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  ...End of file */
