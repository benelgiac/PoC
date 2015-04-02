#include "PeriodicTimer.h"

namespace QAppNG
{    
    void PeriodicTimer::threadStart()
    {
        ScopedLock lock(m_sync);
    
        while (!m_done)
        {
            if (m_queue.empty())
            {
                // Wait (forever) for work
                m_wakeUp.wait(lock);
            }
            else
            {
                auto firstTimer = m_queue.begin();
                Timer& timer = *firstTimer;
                auto now = Clock::now();
                if (now >= timer.next)
                {
                    m_queue.erase(firstTimer);
    
                    // Mark it as running to handle racing destroy
                    timer.running = true;
    
                    // Call the handler
                    lock.unlock();
                    timer.handler(timer.id);
                    lock.lock();
    
                    if (m_done)
                    {
                        break;
                    }
                    else if (!timer.running)
                    {
                        // Running was set to false, destroy was called
                        // for this Timer while the callback was in progress
                        // (this thread was not holding the lock during the callback)
                        m_active.erase(timer.id);
                    }
                    else
                    {
                        timer.running = false;
    
                        // If it is periodic, schedule a new one
                        if (timer.period.count() > 0)
                        {
                            timer.next = timer.next + timer.period;
                            m_queue.insert(timer);
                        }
                        else
                        {
                            m_active.erase(timer.id);
                        }
                    }
                }
                else
                {
                    // Wait until the timer is ready or a timer creation notifies
                    m_wakeUp.wait_until(lock, timer.next);
                }
            }
        }
    } // threadStart()

} // namespace QAppNG

/*
// MAIN TEST
#include "timer.h"
int main()
{
    //PeriodicTimer t;
    // PeriodicTimer fires once, one second from now
    QAppNG::PeriodicTimer::Instance().create(1000, 0,
             [](UInt64) {
                 std::cout << "Non-periodic timer fired" << std::endl;
             });
    // PeriodicTimer fires every second, starting five seconds from now
    QAppNG::PeriodicTimer::Instance().create(5000, 1000,
             [](UInt64) {
                 std::cout << "PeriodicTimer fired 0" << std::endl;
             });
    // PeriodicTimer fires every second, starting now
    QAppNG::PeriodicTimer::Instance().create(0, 1000,
             [](UInt64) {
                 std::cout << "PeriodicTimer fired 1" << std::endl;
             });
    // PeriodicTimer fires every 100ms, starting now
    QAppNG::PeriodicTimer::Instance().create(0, 100,
             [](UInt64) {
                 std::cout << "PeriodicTimer fired 2" << std::endl;
             });
}
*/


/*
class Foo
{
public:
    void bar(QAppNG::PeriodicTimer::timer_id) { std::cout << "Foo::bar called" << std::endl; }
};

int something()
{
    Foo example;

    auto tid = QAppNG::PeriodicTimer::Instance().create(0, 100, std::bind(&Foo::bar, &example, std::placeholders::_1));
    // ... do stuff ...
*/
