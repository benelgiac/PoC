/** ===================================================================================================================
* @file    ThreadCouter HEADER FILE
*
* @brief   ThreadCouter Singleton Class
*
* @copyright
*
* @history
* REF#        Who                                                              When          What
* #9263       A. Della Villa, D. Verna                                         May-2014      Original Development
*
* @endhistory
* ===================================================================================================================
*/
#ifndef QAPPNG_THREAD_COUNTER_H
#define QAPPNG_THREAD_COUNTER_H

#include <QAppNG/Singleton.h>
#include <array>
#include <string>
#include <mutex>

// --------------------------------------------------------------------------------------------------------------------

namespace QAppNG
{
    class ThreadCounter : public QAppNG::Singleton< ThreadCounter >
    {
    public:
        static const size_t UNDEFINED_THREAD_ID = 0xFFFFFFFF;
        static const size_t MAX_NUMBER_OF_THREADS = 128;

        // get/check thread id
        bool hasThreadId();
        size_t getThreadId();

        // set/unset thread name
        bool hasThreadName();
        bool setThreadName( const std::string & thread_name );
        void unsetThreadName();

        // get thread name
        const std::string & getThreadName();
        const std::string & getThreadName( size_t thread_id );

        // return number of threads that have been counted
        size_t getNumberOfAssignedThreadId();

    private:
        friend class QAppNG::Singleton< ThreadCounter >;

        // private CTOR
        ThreadCounter();

        // delete CTOR and COPY CTOR (#FIXME: to be done in Singleton base class)
        ThreadCounter(const ThreadCounter&) = delete;
        ThreadCounter(const ThreadCounter&&) = delete;
        ThreadCounter& operator=(const ThreadCounter&) = delete;
        ThreadCounter& operator=(const ThreadCounter&&) = delete;

#ifdef WIN32
        static __declspec(thread)   bool m_current_thread_has_id;
        static __declspec(thread)   size_t m_current_thread_id;
#else
        static __thread             bool m_current_thread_has_id;
        static __thread             size_t m_current_thread_id;
#endif

        // thread id handling
        static std::recursive_mutex                           m_thread_counter_mutex;
        static size_t                                         m_thread_counter;

        // thread name handling
        static std::array<std::string, MAX_NUMBER_OF_THREADS> m_thread_id_to_thread_name;
        static std::array<bool, MAX_NUMBER_OF_THREADS>        m_thread_name_is_set;
    };
}

// --------------------------------------------------------------------------------------------------------------------
#endif
