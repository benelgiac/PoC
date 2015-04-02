/** ===================================================================================================================
* @file    ThreadCouter Cpp FILE
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

#include "ThreadCounter.h"
#include <exception>

// --------------------------------------------------------------------------------------------------------------------

namespace QAppNG
{
    // static members DEFINITION
#ifdef WIN32
    __declspec(thread)   bool   ThreadCounter::m_current_thread_has_id(false);
    __declspec(thread)   size_t ThreadCounter::m_current_thread_id(UNDEFINED_THREAD_ID);
#else
    __thread             bool   ThreadCounter::m_current_thread_has_id(false);
    __thread             size_t ThreadCounter::m_current_thread_id(UNDEFINED_THREAD_ID);
#endif

    std::recursive_mutex ThreadCounter::m_thread_counter_mutex;
    size_t               ThreadCounter::m_thread_counter(0);

    std::array<std::string, ThreadCounter::MAX_NUMBER_OF_THREADS> ThreadCounter::m_thread_id_to_thread_name;
    std::array<bool, ThreadCounter::MAX_NUMBER_OF_THREADS>        ThreadCounter::m_thread_name_is_set;

    // --------------------------------------------------------------------------------------------------------------------

    ThreadCounter::ThreadCounter()
    {
        m_thread_id_to_thread_name.fill("NO_NAME");
        m_thread_name_is_set.fill(false);
    }

    // --------------------------------------------------------------------------------------------------------------------

    bool ThreadCounter::hasThreadId()
    {
        return m_current_thread_has_id;
    }

    // --------------------------------------------------------------------------------------------------------------------

    size_t ThreadCounter::getThreadId()
    {
        if ( !hasThreadId() )
        {
            std::unique_lock< std::recursive_mutex > lock( m_thread_counter_mutex );

            if ( m_thread_counter >= MAX_NUMBER_OF_THREADS )
            {
                throw std::runtime_error( "Max number of thread reached!!!" );
            }

            m_current_thread_has_id = true;
            m_current_thread_id = m_thread_counter++;
        }

        return m_current_thread_id;
    }

    // --------------------------------------------------------------------------------------------------------------------

    bool ThreadCounter::hasThreadName()
    {
        return m_thread_name_is_set[ getThreadId() ];
    }

    // --------------------------------------------------------------------------------------------------------------------

    bool ThreadCounter::setThreadName( const std::string & thread_name )
    {
        size_t tid = getThreadId();

        if ( !m_thread_name_is_set[tid] )
        {
            m_thread_name_is_set[tid] = true;
            m_thread_id_to_thread_name[tid] = thread_name;
            return true;
        }
        else
        {
            return false;
        }
    }

    // --------------------------------------------------------------------------------------------------------------------

    void ThreadCounter::unsetThreadName()
    {
        size_t tid = getThreadId();

        m_thread_name_is_set[tid] = false;
        m_thread_id_to_thread_name[tid] = "NO_NAME";
    }

    // --------------------------------------------------------------------------------------------------------------------

    const std::string & ThreadCounter::getThreadName()
    {
        return m_thread_id_to_thread_name[ getThreadId() ];
    }

    // --------------------------------------------------------------------------------------------------------------------

    const std::string & ThreadCounter::getThreadName( size_t thread_id )
    {
        //if ( thread_id < MAX_NUMBER_OF_THREADS )
        //{
            //return m_thread_id_to_thread_name[ thread_id ];
        //}

        // it throws an exception if thread_id is out of bounds
        return m_thread_id_to_thread_name.at( thread_id );
    }

    // --------------------------------------------------------------------------------------------------------------------

    size_t ThreadCounter::getNumberOfAssignedThreadId()
    {
        return m_thread_counter;
    }
}

// --------------------------------------------------------------------------------------------------------------------
