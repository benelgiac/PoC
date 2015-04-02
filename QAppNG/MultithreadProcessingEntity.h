#ifndef INCLUDE_MultithreadProcessingEntity_H_NG
#define INCLUDE_MultithreadProcessingEntity_H_NG
/** ===================================================================================================================
  * @file    QAppNG::MultithreadProcessingEntity HEADER FILE
  *
  * @brief   ReEvolution for Iub ReArch project
  *
  * @copyright
  *
  * @history
  * REF#        Who                                                              When          What
  * #4732       Alessandro Della Villa                                           Mar-2010      Original development
  * #5774       Stanislav Timinsky                                               Jan-2011      Added shutdown method
  * #6439       Alessandro Della Villa                                           Oct-2011      Added getPointerToMaster
  *
  * @endhistory
  * ===================================================================================================================
  */

// Include STL & BOOST
#include <memory>

// Other Includes
#include <QAppNG/core.h>
#include <QAppNG/QObservable.h>
#include <QAppNG/WorkManager.h>
#include <QAppNG/QConfigManager.h>

// --------------------------------------------------------------------------------------------------------------------

namespace QAppNG
{
    /**
    *   QAppNG::MultithreadProcessingEntity Class
    *
    *   Let's call MASTER the instance of QAppNG::MultithreadProcessingEntity that runs the start() method
    *   and SLAVE the other instances allocated by WorkManager that run the doWork() method.
    *   Each SLAVE (that is also a Worker) has a pointer to MASTER as init data so it is possible
    *   to copy post_process_function from MASTER.
    *   m_post_process_function = getInitData()->m_post_process_function;
    */
    template<typename PROCESSED_ENTITY>
    class MultithreadProcessingEntity
        : public std::enable_shared_from_this< QAppNG::MultithreadProcessingEntity<PROCESSED_ENTITY> >
        , public BaseWorker< QAppNG::MultithreadProcessingEntity<PROCESSED_ENTITY> >
    {
        friend class WorkManager;

    public:
        // Default Threading params
        static const UInt32 DEFAULT_MAX_QUEUE_SIZE = 100000;
        static const UInt32 DEFAULT_MAX_CONSUMABLES_PER_LOOP = 10000;
        static const bool   DEFAULT_ADAPTIVE_LOAD_BALANCE = true;
        static const UInt32 DEFAULT_ADAPTIVE_MIN_SLEEP_MSEC = 10;
        static const UInt32 DEFAULT_ADAPTIVE_MAX_SLEEP_MSEC = 400;
        static const UInt32 DEFAULT_FIXED_SLEEP_MSEC = 300;

        // CTOR
        MultithreadProcessingEntity()
            : m_work_started(false)
            , m_process_function(DoNothing)
            , m_process_function_with_thread_routing(DoNothing)
            , m_processed_entities(0)
            , m_init_done(false)
        {
            QAppNG::QConfigManager::instance().getValue<std::string>("main", "WorkManagerConfigFile", m_workmanager_config_file);
        }

        // virtual DTOR
        virtual ~MultithreadProcessingEntity() {}

        template <typename PROCESSING_ENTITY_DERIVED_CLASS>
        void start( std::string work_name, std::shared_ptr<WorkDataClass> work_setup = std::shared_ptr<WorkDataClass>( new WorkDataClass ) )
        {
            /** Set the PROCESS FUNCTION
            *
            *   m_process_function will be called in the SAME INSTANCE that now is running this start method, let's say the
            *   MASTER instance.
            */
            fastdelegate::FastDelegate2<size_t, std::shared_ptr<PROCESSED_ENTITY>&, bool> _process_function
                ( WorkManager::Instance().getPtr(), &WorkManager::addConsumable<PROCESSED_ENTITY, PROCESSING_ENTITY_DERIVED_CLASS> );
            m_process_function = _process_function;

            /** Set the PROCESS FUNCTION WITH THREAD ROUTING
            *
            *   the same of process_function but point to the WorkManager method that uses custom thread routing
            */
            fastdelegate::FastDelegate4<size_t, std::shared_ptr<PROCESSED_ENTITY>&, UInt64, bool, bool> _process_function_with_thread_routing
                ( WorkManager::Instance().getPtr(), &WorkManager::addConsumable<PROCESSED_ENTITY, PROCESSING_ENTITY_DERIVED_CLASS> );
            m_process_function_with_thread_routing = _process_function_with_thread_routing;

            /** Start the WORK passing a shared pointer to the current instance as INIT parameter 
            *
            *   (all threads run doWork() method of other instances)
            *   Work manager first try to load configuration from WorkManagerConfigxml. If it fails the uses work_setup class
            */
            if ( !WorkManager::instance().startWork<PROCESSED_ENTITY, PROCESSING_ENTITY_DERIVED_CLASS>( work_name, m_workmanager_config_file, this->shared_from_this() ) )
            {
                WorkManager::instance().startWork<PROCESSED_ENTITY, PROCESSING_ENTITY_DERIVED_CLASS>
                    ( work_name, work_setup, this->shared_from_this() );
            }

            //Store Work Name
            m_work_name = work_name;

            // STORE work_id
            m_work_id = WorkManager::instance().getWorkUniqueId(work_name);

            // STORE number of workers
            m_number_of_workers = WorkManager::instance().getNumberOfWorkers(work_name);

            // DO MASTER INSTANCE INITIALIZATION (it is done once for each pool)
            // NB: Master init is called after running startWork(...) method. Because of override we do not know the exact number of worker 
            //     (we should read WorkmanagerConfig.xml) until we start work. To avoid that slave init is run BEFORE master init, m_work_started is used.
            doMasterInstanceInitialization();

            // mark work as started
            m_work_started = true;
        }

        /**
        *  This method will stop all SLAVEs
        */
        virtual bool stop()
        {
            if ( (m_number_of_workers > 0) && (m_work_started) )
            {
                WorkManager::instance().stopWork( m_work_name );
                return true;
            }
            else
            {
                return false;
            }
        }

        /**
        *  run the right addConsumable<SPECIALIZED_STACK_DECODER>(...)
        */
        inline void process( std::shared_ptr<PROCESSED_ENTITY>& processed_entity )
        {
            // do nothing if the work is not started yet
            if ( !m_work_started ) return;

            // get routing key (we are running in the MASTER instance)
            UInt64 routing_key( getRoutingKey(processed_entity) );

            if (m_number_of_workers > 0 && routing_key != QAppNG::AUTOMATIC_ROUTING_KEY_VALUE && routing_key != QAppNG::BROADCAST_ROUTING_KEY_VALUE)
            {
                threadSend( routing_key, processed_entity );
            }
            else if (m_number_of_workers > 0 && routing_key == QAppNG::BROADCAST_ROUTING_KEY_VALUE)
            {
                threadBroadcast( processed_entity );
            }
            else
            {
                threadSend( processed_entity );
            }
        }

        //FIX UGLY WORKAROUND
        inline void process_ent(std::shared_ptr<PROCESSED_ENTITY>& processed_entity)
        {
            process(processed_entity);
        }
        
        template< typename CLASS_DERIVED_FROM_PROCESSED_ENTITY >
        inline  void process( std::shared_ptr< CLASS_DERIVED_FROM_PROCESSED_ENTITY >& derived_class_from_processed_entity )
        {
            std::shared_ptr< PROCESSED_ENTITY > processed_element
                = std::static_pointer_cast< PROCESSED_ENTITY >( derived_class_from_processed_entity );

            process( processed_element );
        }

    public:
        // --------------------------------------------------------------------------------------------------------------------
        // getWorkId()    --> very important, is used for *WorkManager fast lookup*. NOT PRESENT IN BaseWorker CLASS!!!
        // getThreadId()  --> identify thread in the operative system
        // getThreadKey() --> it is unique only in the same work: (0, 1, ..., n-1) where n=number_of_workers
        std::string         getWorkName()                  { return BaseWorker< QAppNG::MultithreadProcessingEntity<PROCESSED_ENTITY> >::getWorkName(); }
        UInt64              getWorkId()                    { return m_work_id; }
        std::thread::id   getThreadId()                  { return BaseWorker< QAppNG::MultithreadProcessingEntity<PROCESSED_ENTITY> >::getTID(); }
        std::string         getThreadIdString()            { std::stringstream output; output.clear(); output << getThreadId(); return output.str(); }
        UInt64              getThreadKey()                 { return BaseWorker< QAppNG::MultithreadProcessingEntity<PROCESSED_ENTITY> >::getKey(); }
        UInt32              getNumberOfWorkers()           { return m_number_of_workers; }
        UInt64              getNumberOfProcessedEntities() { return m_processed_entities; }

    protected:
        /**
        *   Following method is intended to get a pointer to MASTER instance from SLAVEs and copy init values.
        *   Typically it should be called from SLAVE INIT. The actual class of MASTER/SLAVE has to be provided as template argument.
        */
        template<typename PROCESSING_ENTITY_CLASS>
        std::shared_ptr<PROCESSING_ENTITY_CLASS> getPointerToMasterInstance()
        {
            // if following check fails we are running this method from master instance and this has no sense!
            if ( !this->getInitData() ) return std::shared_ptr<PROCESSING_ENTITY_CLASS>();

            return std::static_pointer_cast<PROCESSING_ENTITY_CLASS>( this->getInitData() );
        }

        // --------------------------------------------------------------------------------------------------------------------

        // THREAD BROADCAST
        inline void threadBroadcast(std::shared_ptr<PROCESSED_ENTITY>& processed_entity)
        {
            for ( UInt64 tid=0; tid < m_number_of_workers; tid++)
            {
                m_process_function_with_thread_routing( m_work_id, processed_entity, tid, true );
            }
        }

        // --------------------------------------------------------------------------------------------------------------------

        // THREAD SEND WITH CUSTOM ROUTING
        inline void threadSend(UInt64 routing_key,  std::shared_ptr<PROCESSED_ENTITY>& processed_entity)
        {
            m_process_function_with_thread_routing( m_work_id, processed_entity, routing_key, false );
        }

        // --------------------------------------------------------------------------------------------------------------------

        // THREAD SEND WITH AUTOMATIC ROUTING
        inline void threadSend(std::shared_ptr<PROCESSED_ENTITY>& processed_entity)
        {
            m_process_function( m_work_id, processed_entity );
        }

        // --------------------------------------------------------------------------------------------------------------------

        // states if work has been started or not
        volatile bool m_work_started;

        /** 
        *   Method called by WorkManager in multithread context - DO NOT USE or MODIFY it
        */
    public: // TODO: this method should be PRIVATE
        bool doWork( std::shared_ptr<PROCESSED_ENTITY>& processed_entity, UInt64 thread_key )
        {
            UNUSED( thread_key );

            if ( !m_init_done )
            {
                // copy m_work_started value from MASTER
                m_work_started = getPointerToMasterInstance< MultithreadProcessingEntity<PROCESSED_ENTITY> >()->m_work_started;

                // do nothing if the work is not started yet
                if (!m_work_started) return false;

                // copy data from master instance
                m_work_name = getWorkName();
                m_work_id = WorkManager::instance().getWorkUniqueId( getWorkName() );
                m_number_of_workers = WorkManager::instance().getNumberOfWorkers( getWorkName() );

                // run worker init method if it was implemented in the derived class
                doSlaveInstancesInitialization();

                // init must be done ONCE
                m_init_done = true;
            }

            // EXECUTE PROCESS 
            doProcessing( processed_entity );

            // increment processed entities counter
            ++m_processed_entities;

            return true;
        }

        virtual void   doSlaveInstancesTermination() {}

    private:
        // *************************************************************************************
        //                      METHODs to BE IMPLEMENTED in DERIVED CLASS
        // *************************************************************************************
        virtual void   doMasterInstanceInitialization() {}
        virtual void   doSlaveInstancesInitialization() {}
        virtual void   doProcessing( std::shared_ptr<PROCESSED_ENTITY>& processed_entity ) = 0;
        virtual UInt64 getRoutingKey( std::shared_ptr<PROCESSED_ENTITY>& processed_entity )
        {
            UNUSED( processed_entity );
            return QAppNG::AUTOMATIC_ROUTING_KEY_VALUE;
        }

        // Dummy Functions to init the processing delegates
        static bool DoNothing(size_t, std::shared_ptr<PROCESSED_ENTITY>&) { return true; }
        static bool DoNothing(size_t, std::shared_ptr<PROCESSED_ENTITY>&, UInt64, bool) { return true; }

        // pointer to PROCESS method
        fastdelegate::FastDelegate2<size_t, std::shared_ptr<PROCESSED_ENTITY>&, bool> m_process_function;

        // pointer to PROCESS method that uses thread routing
        fastdelegate::FastDelegate4<size_t, std::shared_ptr<PROCESSED_ENTITY>&, UInt64, bool, bool> m_process_function_with_thread_routing;

        // pointer to POST PROCESS method
        fastdelegate::FastDelegate2<std::shared_ptr<PROCESSED_ENTITY>&, UInt64, void> m_post_process_function;

        // number of workers (slave instances)
        UInt32 m_number_of_workers;

        // Work Name
        std::string m_work_name;

        // counter
        UInt64 m_processed_entities;

        // work parameters (not present in BaseWorker class)
        std::string m_workmanager_config_file;
        size_t      m_work_id;
        bool        m_init_done;
    };

}
#endif // INCLUDE_MultithreadProcessingEntity_H_NG
// --------------------------------------------------------------------------------------------------------------------
// End of file




