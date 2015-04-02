/** ===================================================================================================================
* @file    WorkManager
*
* @brief   WorkManager 2.0 Headers
*
* @copyright
*
* @history
* REF#        Who                                                              When          What
* #????       A. Della Villa                                                   Oct-2008      Original development
* #4331       A. Della Villa                                                   Nov-2009      WorkManager version 2.0
* #5536       A. Della Villa                                                   Dec-2010      New user-defined routing policy
*
* @endhistory
* ===================================================================================================================
*/
#ifndef INCLUDE_WORKMANAGER_NG
#define INCLUDE_WORKMANAGER_NG

// Include STL & BOOST
#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <exception>
#include <boost/array.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>
#include <boost/optional.hpp>

// other Includes
#include "core.h"
#include "Singleton.h"
#include "TrivialThreadPool.h"
#include "WorkManagerStatus.h"

namespace QAppNG
{
    // --------------------------------------------------------------------------------------------------------

    // forward declaration of FicticiousWorker used for NO_Multithread case
    template<class WORK_CLASS> class FicticiousWorker;

    // --------------------------------------------------------------------------------------------------------
    //                                             *** WorkDataClass ***
    // --------------------------------------------------------------------------------------------------------

    /**
    *  @author Alessandro Della Villa <alessandro.dellavilla@CommProve.com>
    *
    *  @brief simple class to store WORK parameters and WORKERs operative data
    *         it defines also the default values for a WORK
    *
    */
    class WorkDataClass
    {
        // Declare WorkManager as FRIEND CLASS
        friend class WorkManager;

    public:
        WorkDataClass()
            : number_of_workers(1)
            , work_type(TrivialThreadPool)
            , overload_strategy(Drop)
            , current_work_state(eWorkRunning)
            , work_name("no_work_name_defined")
            , thread_pool_destroyer(NULL)
            , produced(0)
            , dropped(0)
        {
            routing_map.reset( new std::unordered_map<UInt64, UInt64>() );
        };

        // Work Setup
        UInt32 number_of_workers;
        enum work_type_enum { No_MultiThread, TrivialThreadPool, Disabled } work_type;
        enum overload_strategy_enum { Drop, Wait } overload_strategy;

        // Thread Setup
        ThreadDataClass thread_data_setup;

    private:
        // Work current state: each work starts with state Running and when is stopped and the
        // stop is complete the state is changed to Stopped. WorkManager has its own state
        // that is used to avoid to be "disturbed" meanwhile it is stopping a Work.
        enum work_state_enum { eWorkRunning, eWorkStopped } current_work_state;

        // Work UNIQUE ID is used to fast access to the work (using vector)
        size_t work_unique_id;

        // Work name, is set using addWork method
        std::string work_name;

        // POOL
        std::shared_ptr<void> thread_pool;

        // POOL DESTROYER METHOD
        fastdelegate::FastDelegate0<void> thread_pool_destroyer;

        // Work operative data
        UInt64 produced;
        UInt64 dropped;

        // Thread IDs
        boost::optional< UInt64 > producer_TID;

        // these fields have to be updated from the pool
        std::shared_ptr< std::vector<UInt64> > per_thread_assigned;
        std::shared_ptr< std::vector<UInt64> > per_thread_consumed;
        std::shared_ptr< std::vector<UInt64> > per_thread_number_of_calls;
        std::shared_ptr< std::vector<UInt64> > per_thread_last_sleep_msec;
        std::shared_ptr< std::vector<UInt64> > consumer_TIDs;

        // user defined THREADs ROUTING MAP
        // when user specify a routing_key this map is used to lear the routes
        std::shared_ptr< std::unordered_map<UInt64, UInt64> > routing_map;
    };

    // --------------------------------------------------------------------------------------------------------
    //                                              *** WorkManager ***
    // --------------------------------------------------------------------------------------------------------

    /**
    *  @author Alessandro Della Villa <alessandro.dellavilla@CommProve.com>
    *
    *  @brief Works Manager. Main class to create works, run them multithread and dispatch consumables.
    */
    class WorkManager: public Singleton<WorkManager>
    {
        friend class Singleton<WorkManager>;

        static const size_t MAX_NUMBER_OF_WORK = 256;

    public:
        // DTOR
        ~WorkManager();

        //____________________________________________________________________________________________________________
        // START WORK METHOD 1: uses configuration LOADED from xml file
        template < class WORK_CONSUMABLE_CLASS, class WORK_CLASS >
        bool startWork( const std::string& work_name
            , const std::string& xml_config_filename
            , std::shared_ptr<typename WORK_CLASS::ThreadInitClass> work_init_data = std::shared_ptr<typename WORK_CLASS::ThreadInitClass>() )
        {
            std::shared_ptr<WorkDataClass> work_setup( new WorkDataClass );
            if ( loadWorkSetup( xml_config_filename, work_name, work_setup ) )
            {
                return __startWork<WORK_CONSUMABLE_CLASS, WORK_CLASS>( work_name, work_setup, work_init_data );
            }
            else
            {
                return false;
            }
        };

        //____________________________________________________________________________________________________________
        // START WORK METHOD 2: complete configuration structure is given (work_setup) and INIT-DATA is optional
        template < class WORK_CONSUMABLE_CLASS, class WORK_CLASS >
        bool startWork( const std::string& work_name
            , std::shared_ptr<WorkDataClass> work_setup
            , std::shared_ptr<typename WORK_CLASS::ThreadInitClass> work_init_data = std::shared_ptr<typename WORK_CLASS::ThreadInitClass>() )
        {
            return __startWork<WORK_CONSUMABLE_CLASS, WORK_CLASS>( work_name, work_setup, work_init_data );
        };

        //____________________________________________________________________________________________________________
        // START WORK METHOD 3: parameters are given and NO-INIT-DATA is passed to WORKERs
        template < class WORK_CONSUMABLE_CLASS, class WORK_CLASS >
        bool startWork( const std::string& work_name
            , UInt32 number_of_workers = 1
            , UInt32 worker_max_queue_size = 100000
            , WorkDataClass::work_type_enum work_type = WorkDataClass::TrivialThreadPool
            , UInt32 max_consumable_per_worker_loop = 10000
            , WorkDataClass::overload_strategy_enum overload_strategy = WorkDataClass::Drop)
        {
            std::shared_ptr<WorkDataClass> work_setup( new WorkDataClass );
            work_setup->work_type                                  = work_type;
            work_setup->overload_strategy                          = overload_strategy;
            work_setup->number_of_workers                          = number_of_workers;
            work_setup->thread_data_setup.max_queue_size           = worker_max_queue_size;
            work_setup->thread_data_setup.max_consumables_per_loop = max_consumable_per_worker_loop;
            work_setup->thread_data_setup.adaptive_load_balance    = true;
            work_setup->thread_data_setup.adaptive_min_sleep_msec  = 10;
            work_setup->thread_data_setup.adaptive_max_sleep_msec  = 400;
            work_setup->thread_data_setup.fixed_sleep_msec         = 300;

            return __startWork<WORK_CONSUMABLE_CLASS, WORK_CLASS>( work_name, work_setup, std::shared_ptr<typename WORK_CLASS::ThreadInitClass>() );
        };

        //____________________________________________________________________________________________________________
        // START WORK METHOD 4: parameters are given and INIT-DATA is passed to WORKERs
        template < class WORK_CONSUMABLE_CLASS, class WORK_CLASS >
        bool startWork( const std::string& work_name
            , std::shared_ptr<typename WORK_CLASS::ThreadInitClass> work_init_data
            , UInt32 number_of_workers
            , UInt32 worker_max_queue_size = 100000
            , WorkDataClass::work_type_enum work_type = WorkDataClass::TrivialThreadPool
            , UInt32 max_consumable_per_worker_loop = 10000
            , WorkDataClass::overload_strategy_enum overload_strategy = WorkDataClass::Drop)
        {
            std::shared_ptr<WorkDataClass> work_setup( new WorkDataClass );
            work_setup->work_type                                  = work_type;
            work_setup->overload_strategy                          = overload_strategy;
            work_setup->number_of_workers                          = number_of_workers;
            work_setup->thread_data_setup.max_queue_size           = worker_max_queue_size;
            work_setup->thread_data_setup.max_consumables_per_loop = max_consumable_per_worker_loop;
            work_setup->thread_data_setup.adaptive_load_balance    = true;
            work_setup->thread_data_setup.adaptive_min_sleep_msec  = 10;
            work_setup->thread_data_setup.adaptive_max_sleep_msec  = 400;
            work_setup->thread_data_setup.fixed_sleep_msec         = 300;

            return __startWork<WORK_CONSUMABLE_CLASS, WORK_CLASS>( work_name, work_setup, work_init_data );
        };


        //____________________________________________________________________________________________________________
        // ADD CONSUMABLE METHOD 1: lookup by work_name (SLOW LOOKUP), AUTOMATIC-CONSUMABLE-ROUTING between threads
        template <class WORK_CONSUMABLE_CLASS, class WORK_CLASS>
        inline bool addConsumable( const std::string& work_name
            , std::shared_ptr<WORK_CONSUMABLE_CLASS>& work_consumable )
        {
            std::shared_ptr<WorkDataClass> work_data
                = works_map[work_name];

            std::shared_ptr< TrivialThreadPool< std::shared_ptr < WORK_CONSUMABLE_CLASS >, WORK_CLASS > > thread_pool
                = std::static_pointer_cast<TrivialThreadPool< std::shared_ptr < WORK_CONSUMABLE_CLASS >, WORK_CLASS >>(work_data->thread_pool);

            UInt64 automatic_routing_thread_key(thread_pool->getPoolTotalAssigned() % work_data->number_of_workers);

            // Call method: __addConsumable specifying Automatic Consumable Routing
            return __addConsumable<WORK_CONSUMABLE_CLASS, WORK_CLASS>( work_data, work_consumable, eAutomaticRouting, automatic_routing_thread_key );
        };

        //____________________________________________________________________________________________________________
        // ADD CONSUMABLE METHOD 2: lookup by work_unique_id (FAST LOOKUP), AUTOMATIC-CONSUMABLE-ROUTING between threads
        template <class WORK_CONSUMABLE_CLASS, class WORK_CLASS>
        inline bool addConsumable( size_t work_unique_id
            , std::shared_ptr<WORK_CONSUMABLE_CLASS>& work_consumable )
        {
            std::shared_ptr<WorkDataClass> work_data
                = works_vector[work_unique_id];

            std::shared_ptr< TrivialThreadPool< std::shared_ptr < WORK_CONSUMABLE_CLASS >, WORK_CLASS > > thread_pool
                = std::static_pointer_cast<TrivialThreadPool< std::shared_ptr < WORK_CONSUMABLE_CLASS >, WORK_CLASS > >(work_data->thread_pool);

            UInt64 automatic_routing_thread_key( thread_pool->getPoolTotalAssigned() % work_data->number_of_workers );


            // Call method: __addConsumable specifying Automatic Consumable Routing
            return __addConsumable<WORK_CONSUMABLE_CLASS, WORK_CLASS>( work_data, work_consumable, eAutomaticRouting, automatic_routing_thread_key);
        };

        //____________________________________________________________________________________________________________
        // ADD CONSUMABLE METHOD 3: lookup by work_name (SLOW LOOKUP), USER-DEFINED-CONSUMABLE-ROUTING between threads
        template <class WORK_CONSUMABLE_CLASS, class WORK_CLASS>
        inline bool addConsumable( const std::string& work_name
            , std::shared_ptr<WORK_CONSUMABLE_CLASS>& work_consumable
            , UInt64 user_defined_routing_key, bool broadcast)
        {
            std::shared_ptr<WorkDataClass> work_data( works_map[work_name] );
            UInt64 thread_key;

            // the following assert is not needed if no routing key is used
            assert( work_data->number_of_workers );

            // OLD Map-Based routing policy

            if (!broadcast)
            {
                if (!work_data->routing_map->count(user_defined_routing_key))
                {
                    // #10650: We reached Maximum number of routing paths.
                    // An exception is thrown to notify we are having too many routing keys
                    // for any WORK_CONSUMABLE_CLASS
                    if (work_data->routing_map->size() > MAX_NUMBER_OF_ROUTING_PATHS)
                    {
                        std::ostringstream error_message;
                        error_message << "WorkManager - Maximum Number Of Routing Path Exceeded - "
                            << "Work Name: " << work_data->work_name;

                        throw std::runtime_error(error_message.str().c_str());
                    }

                    work_data->routing_map->insert(std::unordered_map< UInt64, UInt64 >::value_type
                        (user_defined_routing_key, work_data->routing_map->size() % work_data->number_of_workers));
                }

                thread_key = work_data->routing_map->operator[]( user_defined_routing_key );
            }
            else
            {
                // A broadcast event is routed by threadId
                thread_key = user_defined_routing_key;
            }

            // NEW routing policy
            //UInt64 thread_key = user_defined_routing_key % work_data->number_of_workers;

            // Call method: __addConsumable using thred_key calculated from user_defined_routing_key
            return __addConsumable<WORK_CONSUMABLE_CLASS, WORK_CLASS>( works_map[work_name], work_consumable, eUserDefinedRouting, thread_key );
        };

        //____________________________________________________________________________________________________________
        // ADD CONSUMABLE METHOD 4: lookup by work_unique_id (FAST LOOKUP), USER-DEFINED-CONSUMABLE-ROUTING between threads
        template <class WORK_CONSUMABLE_CLASS, class WORK_CLASS>
        inline bool addConsumable( size_t work_unique_id
            , std::shared_ptr<WORK_CONSUMABLE_CLASS>& work_consumable
            , UInt64 user_defined_routing_key, bool broadcast )
        {
            std::shared_ptr<WorkDataClass> work_data( works_vector[work_unique_id] );
            UInt64 thread_key;

            // the following assert is not needed if no routing key is used
            assert( work_data->number_of_workers );

            // OLD Map-Based routing policy

            if (!broadcast)
            {
                if ( !work_data->routing_map->count(user_defined_routing_key) )
                {
                    // #10650: We reached Maximum number of routing paths.
                    // An exception is thrown to notify we are having too many routing keys
                    // for any WORK_CONSUMABLE_CLASS
                    if( work_data->routing_map->size() > MAX_NUMBER_OF_ROUTING_PATHS )
                    {
                        std::ostringstream error_message;
                        error_message << "WorkManager - Maximum Number Of Routing Path Exceeded - "
                                      << "Work Name: " << work_data->work_name;

                        throw std::runtime_error( error_message.str().c_str() );
                    }

                    work_data->routing_map->insert( std::unordered_map< UInt64, UInt64 >::value_type
                        ( user_defined_routing_key, work_data->routing_map->size() % work_data->number_of_workers ) );
                }

                thread_key = work_data->routing_map->operator[]( user_defined_routing_key );
            }
            else
            {
                 // A broadcast event is routed by threadId
                thread_key = user_defined_routing_key;
            }

            // NEW routing policy
            // This policy doesn't guarantee agood lad balangind between threads
            //UInt64 thread_key = user_defined_routing_key % work_data->number_of_workers;

            // Call method: __addConsumable using thred_key calculated from user_defined_routing_key
            return __addConsumable<WORK_CONSUMABLE_CLASS, WORK_CLASS>( works_vector[work_unique_id], work_consumable, eUserDefinedRouting, thread_key );
        };

        //____________________________________________________________________________________________________________
        // STOP WORK
        bool stopWork( const std::string& work_name );

        //____________________________________________________________________________________________________________
        // get info about WORKs and also get UNIQUE_WORK_ID for a given work
        WorkDataClass::work_type_enum getWorkType( const std::string& work_name );
        size_t getWorkUniqueId( const std::string& work_name );
        UInt32 getNumberOfWorkers( const std::string& work_name );
        UInt32 getQueueSize( const std::string& work_name );
        UInt64 getNumberOfDropped( const std::string& work_name );
        UInt64 getNumberOfConsumed( const std::string& work_name );
        UInt64 getNumberOfProduced( const std::string& work_name );

        //______________________________________________________
        void startStatusReport( const std::string& status_file_name, UInt16 status_rate )
        {
            if ( !work_manager_status.get() )
            {
                fastdelegate::FastDelegate0<std::string> get_status_delegate(this, &WorkManager::getStatus);
                //fastdelegate::FastDelegate<std::string(void)> get_status_delegate(this, &WorkManager::getStatus);

                work_manager_status.reset( new WorkManagerStatus(status_file_name, get_status_delegate) );
                work_manager_status->StartTimer( status_rate );
            }
        };

        //______________________________________________________
        std::string getStatus();

        //______________________________________________________
        void shutdown();

    private:
        //____________________________________________________________________________________________________________
        // LOAD a WORK from xml file and start it (to make WorkManager load work configuration from file, an xml file should be passed to startWork methdod)
        bool loadWorkSetup( const std::string& xml_config_filename, const std::string& work_name, std::shared_ptr<WorkDataClass> work_setup );

        // CTOR
        WorkManager() : Singleton<WorkManager>(), works_vector(MAX_NUMBER_OF_WORK), work_vector_element_counter(0), disable_get_status(false) { };

        // *********************** MAIN WORKs CONTAINERS ************************
        // the shared_ptr to works are stored in two different containers,
        // the access through the vector is faster. It is possible to get the
        // unique ID of a given work using: getWorkUniqueId("work_name")
        std::unordered_map<std::string, std::shared_ptr<WorkDataClass> > works_map;
        std::vector< std::shared_ptr<WorkDataClass> > works_vector;
        size_t work_vector_element_counter;

        // Strategy to route current consumable (used internally by addConsumable & __addConsumable methods
        enum CurrentConsumableRouting { eAutomaticRouting, eUserDefinedRouting };

        // Status handler, it provides status output on file
        std::unique_ptr<WorkManagerStatus> work_manager_status;

        // enable/disable status: it is used to inhibit getStatus during stopWork
        bool disable_get_status;

        // START WORK MAIN METHOD
        template <class WORK_CONSUMABLE_CLASS, class WORK_CLASS>
        bool __startWork( const std::string& work_name
            , std::shared_ptr<WorkDataClass> work_setup
            , std::shared_ptr<typename WORK_CLASS::ThreadInitClass> work_init_data );

        // ADD CONSUMABLE MAIN METHOD
        template <class WORK_CONSUMABLE_CLASS, class WORK_CLASS>
        inline bool __addConsumable( std::shared_ptr<WorkDataClass>& work_data
            , std::shared_ptr<WORK_CONSUMABLE_CLASS>& work_consumable
            , CurrentConsumableRouting routing_type
            , UInt64 thread_key );

        // LOCK: used just for start/stop
        boost::mutex m_mutex;

        static const size_t MAX_NUMBER_OF_ROUTING_PATHS = 256000L;
    };

    // --------------------------------------------------------------------------------------------------------
    //                                           *** FicticiousWorker ***
    // --------------------------------------------------------------------------------------------------------

    /**
    *  @author Alessandro Della Villa <alessandro.dellavilla@CommProve.com>
    *
    *  @brief builds a Ficticious ThreadPool Worker based on the class WORKER_CLASS
    */
    template<class WORK_CLASS>
    class FicticiousWorker : public WORK_CLASS
    {
    public:
        // store the work name
        std::string work_name;

        // re-implement virtual functions
        std::string getWorkName() { return work_name; };

    private:
    };

    // --------------------------------------------------------------------------------------------------------
}

// include .h implementation
#include "detail/WorkManagerImpl.h"

// --------------------------------------------------------------------------------------------------------
#endif // INCLUDE_WORKMANAGER_NG

