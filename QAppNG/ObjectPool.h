#if !defined ( ObjectPool_H_NG )
#define ObjectPool_H_NG
// ===========================================================================
/// @file
/// @brief This file contains a generic object pool
///
/// Useful for handling a shared resource that is expensive to construct,
/// like tshark processes in ddserver.
///
/// @copyright
/// @history
/// REF#        Who              When        What
///        Bennett Schneider     26-May-2010 Original development
/// 9996        Brian Molina     17-Jan-2014 Added ISVALID class template parameter. If provided, it
///                                          uses this method to determine if object is valid before returning
///                                          it. If not valid, then delete the object and return empty
///                                          shared pointer.
/// @endhistory
///
// ===========================================================================

/*  ...Include files
*/
#include <cal/defs.h>
#include <cal/Mutex.h>
#include <cal/AutoLock.h>
#include <cal/CondVar.h>
#include <stack>
#include <cal/SmartPtr.h>
#include <memory>

namespace QAppNG
{
    /// @brief  Enumerated return values from GetObject()
    ///
    enum ObjPool_Status
        {
        ObjPool_Ok = 0,         ///< call returned successfully
        ObjPool_TimeOut,        ///< TimedWait() timedout
        ObjPool_Empty,          ///< non-blocking specified and pool is empty
        ObjPool_Invalid         ///< ISVALID function provided by user returned false
        };
        
    /*  ...Class definitions
    */
    ///@brief A default reset object
    ///
    template<class OBJECT>
    class ResetDoNothing
        {
    public:
        void operator() (OBJECT&) {};
        };

    template<class OBJECT>
    class IsValidAlwaysTrue
        {
    public:
        bool operator() (OBJECT&) {return true;};
        };

    ///@brief ObjectPool
    ///
    ///A pool of objects that can be reused by multiple threads
    ///The RESET template should perform any necessary cleanup to the
    ///objects that will allow for their reuse (clearing buffers, etc).
    ///It is the user's responsibility to allocate and pass ownership on to
    ///the ObjectPool.
    template <class OBJECT, class ISVALID = IsValidAlwaysTrue<OBJECT>, class LOCK = cal::Mutex, class RESET = ResetDoNothing<OBJECT> >
    class ObjectPool : public std::enable_shared_from_this<ObjectPool<OBJECT,ISVALID,LOCK,RESET> >
        {
    public:

        ObjectPool()
            {
            }

        virtual ~ObjectPool()
            {
            RESET reset;
            while (!m_pool.empty())
                {
                reset(*(m_pool.top()));
                delete(m_pool.top());
                m_pool.pop();
                }
            }

        ///@brief Add a user constructed object to the pool
        ///
        /// This allows the user to use whatever constructor they prefer.  The object pool will
        /// invoke delete on any objects added to it in it's destructor.
        void AddObject(OBJECT* newObject)
            {
            cal::AutoLock<LOCK> locker(m_poolLock);
            m_pool.push(newObject);
            m_wait.Signal();
            }

        ///@brief Acquire an object from the pool
        ///
        ///This returns a modified shared pointer.  When the shared pointer is released
        ///rather than being freed, the object is returned to the pool.  It will be freed
        ///when the object pool itself is destructed.
        std::shared_ptr<OBJECT> GetObject(ObjPool_Status * status, bool blocking = true)
            {
            OBJECT * rv = NULL;
            m_poolLock.Lock();
            if (m_pool.empty())
                {
                m_poolLock.Unlock();
                if (blocking)
                    {
                    cal::CondVar::CondStatus waitStatus = m_wait.TimedWait(MakeDelegate(this, &ObjectPool<OBJECT,ISVALID,LOCK,RESET>::Wait), cal::time::Duration<SecondRes> (30) );
                    if ( waitStatus != cal::CondVar::Cond_Ok )
                        {
                        *status = ObjPool_TimeOut;
                        return std::shared_ptr<OBJECT>();
                        }
                    assert(!m_pool.empty() && "If we woke up and the pool is empty, there's a logic error");
                    m_poolLock.Lock();
                    }
                else
                    {
                    *status = ObjPool_Empty;
                    return std::shared_ptr<OBJECT>();
                    }
                }
            rv = m_pool.top();
            m_pool.pop();
            m_poolLock.Unlock();
            
            ISVALID isvalid;
            if ( isvalid(*rv) )
                {
                *status = ObjPool_Ok;
            std::shared_ptr<Reference > deleter( new Reference(rv, this->shared_from_this()));
            return std::member_pointer(deleter, rv);
            }
            else
                {
                delete rv;
                *status = ObjPool_Invalid;
                return std::shared_ptr<OBJECT>();
                }
            }

    private:
        class Reference
            {
        public:
            Reference( OBJECT * a_ptr, std::shared_ptr<ObjectPool<OBJECT,ISVALID,LOCK,RESET> > a_owner)
                :m_object (a_ptr)
                ,m_owner(a_owner)
                {
                }
            ~Reference()
                {
                m_owner->ReturnObject(m_object);
                }
        private:
            OBJECT * m_object;
            std::shared_ptr<ObjectPool<OBJECT,ISVALID,LOCK,RESET> > m_owner;
            };

        friend class Reference;

        void ReturnObject( OBJECT * a_obj)
            {
            RESET reset;
            reset(*a_obj);
            cal::AutoLock<LOCK> locker(m_poolLock);
            m_pool.push(a_obj);
            m_wait.Signal();
            }

        bool Wait()
            {
            cal::AutoLock<LOCK> locker(m_poolLock);
            return !m_pool.empty();
            }

        std::stack<OBJECT *>  m_pool;
        LOCK                  m_poolLock;
        cal::CondVar          m_wait;
        };
    //#include    "ObjectPoolImpl.h"

}
#endif // ObjectPool_H_NG

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  ...End of file */





