/** =================================================================================================================
* @file    Singleton.h
*
* @brief   Singleton
*
* @copyright
*
* @history
* REF#        Who                                                              When          What
* #????       A. Della Villa                                                   31/10/2008    Original development
*
* @endhistory
* ===================================================================================================================
*/
#ifndef INCLUDE_SINGLETON_NG
#define INCLUDE_SINGLETON_NG

#include <boost/thread/recursive_mutex.hpp>
#include <memory>

// --------------------------------------------------------------------------------------------------------

namespace QAppNG
{
    template <typename T>
    class Singleton
    {
    public:
        static T& instance()
        {
            // to improve performance we use mutex only if needed
            if ( !m_data_ptr.get() )
            {
                // mutex and double pointer check make Singleton::instance() thread safe
                boost::unique_lock<boost::recursive_mutex> creation_lock(m_creation_mutex);
                if ( !m_data_ptr.get() )
                {
                    //m_data_ptr.reset( new T() );
                    m_data_ptr.reset( new T() );
                    m_data_ptr->init();
                }
            }

            return *( m_data_ptr.get() );
        }

        // Added for COMPATIBILITY with old class. TODO: remove after renaming
        static T& Instance()
        {
            return instance();
        }

        static T* getPtr() { return m_data_ptr.get(); }

        // Added for COMPATIBILITY with old class. TODO: remove after renaming
        static T* GetPtr()
        {
            return getPtr();
        }

        // DTOR
        virtual ~Singleton() {}

    protected:
        // CTOR has to be protected
        Singleton() {}

    private:
        // COPY and ASSIGMEMENT have to be deleted
        Singleton( Singleton const & ) {}
        Singleton& operator=( Singleton const & one ) { return one; }

        // store Singleton data
        static std::unique_ptr<T> m_data_ptr;

        // mutex
        static boost::recursive_mutex m_creation_mutex;

        // init method
        virtual void init() {}
    };

    // --------------------------------------------------------------------------------------------------------
    // static members definition:

    template<typename T>
    std::unique_ptr<T> Singleton<T>::m_data_ptr;

    template<typename T>
    boost::recursive_mutex Singleton<T>::m_creation_mutex;

    // --------------------------------------------------------------------------------------------------------
}
#endif // INCLUDE_SINGLETON_NG


