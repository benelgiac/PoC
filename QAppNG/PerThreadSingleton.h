/** =================================================================================================================
* @file    PerThreadSingleton.h
*
* @brief   PerThreadSingleton
*
* @copyright
*
* @history
* REF#        Who                                                              When          What
* #8060       A. Della Villa                                                   Mar 2013      Original development
*
* @endhistory
* ===================================================================================================================
*/
#ifndef INCLUDE_PER_THREAD_SINGLETON_NG
#define INCLUDE_PER_THREAD_SINGLETON_NG

#include <boost/thread/recursive_mutex.hpp>

// ************************************************************************************************************
// TODO: as soon as we have a gcc 4.8+ compiler USE c++11 thread_local instead platform specific thread storage
// ************************************************************************************************************ 

// --------------------------------------------------------------------------------------------------------

namespace QAppNG
{
    template <typename T>
    class PerThreadSingleton
    {
    public:
        static T& instance()
        {
            // to improve performance we use mutex only if needed
            if ( m_data_ptr == NULL )
            {
                // mutex and double pointer check make PerThreadSingleton::instance() thread safe
                boost::unique_lock<boost::recursive_mutex> creation_lock(m_creation_mutex);
                if ( m_data_ptr == NULL )
                {
                    m_data_ptr = new T();
                    m_data_ptr->init();

                    // store all per thread pointers in a vector and delete all in DTOR
                    m_all_threads_data_ptr_vector.push_back( m_data_ptr  );
                }
            }

            return *( m_data_ptr );
        }

        // Added for COMPATIBILITY with old class. TODO: remove after renaming
        static T& Instance()
        {
            return instance();
        }

        // return raw pointer to object T
        static T* getPtr() { return m_data_ptr; }

        // DTOR
        virtual ~PerThreadSingleton() 
        {
            // destroy memory allocated by all threads
            for ( size_t i = 0; i < m_all_threads_data_ptr_vector.size(); i++ )
            {
                delete m_all_threads_data_ptr_vector[i];
            }

            m_all_threads_data_ptr_vector.clear();
        }

    protected:
        // CTOR has to be protected
        PerThreadSingleton() {}

        // init method
        virtual void init() {}
    private:
        // COPY and ASSIGMEMENT have to be deleted
        PerThreadSingleton( PerThreadSingleton const & ) {}
        PerThreadSingleton& operator=( PerThreadSingleton const & one ) { return one; }

        // store PerThreadSingleton data
#ifdef WIN32
        static __declspec(thread) T* m_data_ptr;
#else
        static __thread T* m_data_ptr;
#endif
        
        // store all pointer used by all threads
        static std::vector<T*> m_all_threads_data_ptr_vector;

        // mutex
        static boost::recursive_mutex m_creation_mutex;
    };

    // --------------------------------------------------------------------------------------------------------
    // static members definition:

#ifdef WIN32
    template<typename T>
    __declspec(thread) T* PerThreadSingleton<T>::m_data_ptr;
#else
    template<typename T>
    __thread T* PerThreadSingleton<T>::m_data_ptr(NULL);
#endif

    template<typename T>
    std::vector<T*> PerThreadSingleton<T>::m_all_threads_data_ptr_vector;
    
    template<typename T>
    boost::recursive_mutex PerThreadSingleton<T>::m_creation_mutex;

    // --------------------------------------------------------------------------------------------------------
}

// --------------------------------------------------------------------------------------------------------
#endif // INCLUDE_PER_THREAD_SINGLETON_NG
