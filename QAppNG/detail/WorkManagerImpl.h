/** ===================================================================================================================
* @file    WorkManagerImpl
*
* @brief   WorkManager 2.0 .h Implementation 
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

// --------------------------------------------------------------------------------------------------------

#include <QAppNG/ThreadCounter.h>

namespace QAppNG
{
    // --------------------------------------------------------------------------------------------------------
    //                                              *** WorkManager ***
    // --------------------------------------------------------------------------------------------------------

    template <class WORK_CONSUMABLE_CLASS, class WORK_CLASS >
    bool WorkManager::__startWork( const std::string& work_name
        , std::shared_ptr<WorkDataClass> work_setup
        , std::shared_ptr<typename WORK_CLASS::ThreadInitClass> work_init_data )
    {
        // apply correction to number_of_workers based on work_type
        if (work_setup->work_type == WorkDataClass::No_MultiThread)
        {
            work_setup->number_of_workers = 1;
        }
        if (work_setup->work_type == WorkDataClass::Disabled)
        {
            work_setup->number_of_workers = 0;
        }

        // USE LOCK
        boost::unique_lock<boost::mutex> lock(m_mutex);

        typedef typename WORK_CLASS::ThreadInitClass WORK_INIT_DATA_CLASS;

        // FAIL to Start Work: the Work is already running
        if ( works_map.count(work_name) ) return false;

        // *** CREATE new WORK *** and copy work_setup
        std::shared_ptr<WorkDataClass> work_data( new WorkDataClass() );
        *work_data = *work_setup;

        // set work_name
        work_data->work_name = work_name;

        // STORE a copy of shared pointer to work in works_map (SLOW LOOKUP) and works_vector (FAST LOOKUP)
        works_map.insert( std::make_pair( work_name, work_data ) );
        assert( work_vector_element_counter < MAX_NUMBER_OF_WORK );
        work_data->work_unique_id = work_vector_element_counter++;
        works_vector[work_data->work_unique_id] = work_data;
        //works_vector.push_back( work_data );

        // SET WORK UNIQUE ID (used for fast work lookup)
        //work_data->work_unique_id = works_vector.size() - 1;

        // INIT per_thread_data
        work_data->per_thread_assigned.reset(        new std::vector<UInt64>() );
        work_data->per_thread_consumed.reset(        new std::vector<UInt64>() );
        work_data->per_thread_number_of_calls.reset( new std::vector<UInt64>() );
        work_data->per_thread_last_sleep_msec.reset( new std::vector<UInt64>() );
        work_data->consumer_TIDs.reset(              new std::vector<UInt64>() );
        for ( size_t i = 0; i < work_data->number_of_workers; i++ )
        {
            work_data->per_thread_assigned->push_back(0);
            work_data->per_thread_consumed->push_back(0);
            work_data->per_thread_number_of_calls->push_back(0);
            work_data->per_thread_last_sleep_msec->push_back(0);
            work_data->consumer_TIDs->push_back(0);
        }

        switch ( work_data->work_type )
        {

            case (WorkDataClass::No_MultiThread):
            {
                // Create a FicticiousWorker to use in the No_Multithread case
                std::shared_ptr< FicticiousWorker< WORK_CLASS > > NO_MULTITHREAD_WORKER( new FicticiousWorker< WORK_CLASS >() );

                // Set FicticiosWorker work_name and thread_init_data
                NO_MULTITHREAD_WORKER->work_name = work_name;
                NO_MULTITHREAD_WORKER->setInitData( work_init_data );

                // Store No_MultiThread pointer
                work_data->thread_pool = NO_MULTITHREAD_WORKER;
                break;
            }

            case (WorkDataClass::TrivialThreadPool):
            {
                // Set Work Name
                work_data->thread_data_setup.work_name = work_name;

                // Set thread_init_data
                work_data->thread_data_setup.thread_init_data = std::static_pointer_cast<WORK_INIT_DATA_CLASS>( work_init_data );

                std::shared_ptr<ThreadOperativeDataWriteBackClass> write_back_data( new ThreadOperativeDataWriteBackClass );
                write_back_data->per_thread_assigned        = work_data->per_thread_assigned;
                write_back_data->per_thread_consumed        = work_data->per_thread_consumed;
                write_back_data->per_thread_number_of_calls = work_data->per_thread_number_of_calls;
                write_back_data->per_thread_last_sleep_msec = work_data->per_thread_last_sleep_msec;
                write_back_data->consumer_TIDs              = work_data->consumer_TIDs;

                // Create & Start TrivialThreadPool
                std::shared_ptr< TrivialThreadPool< std::shared_ptr< WORK_CONSUMABLE_CLASS >, WORK_CLASS> > TRIVIAL_THREAD_POOL
                    ( new TrivialThreadPool< std::shared_ptr< WORK_CONSUMABLE_CLASS >, WORK_CLASS>
                    (work_data->work_name, work_data->number_of_workers, &work_data->thread_data_setup, write_back_data) );

                // Store TrivialThreadPool pointer
                work_data->thread_pool = TRIVIAL_THREAD_POOL;

                // Store TrivialThreadPool Destroyer Method pointer
                work_data->thread_pool_destroyer = fastdelegate::MakeDelegate(TRIVIAL_THREAD_POOL.get(), &TrivialThreadPool< std::shared_ptr< WORK_CONSUMABLE_CLASS >, WORK_CLASS>::stopThreadPool);
                break;
            }

            case (WorkDataClass::Disabled):
            {
                // Nothing to do...
                break;
            }

        } // end switch

        return true;
    }; // END of __startWork(...)


    //____________________________________________________________________________________________________________
    // ADD CONSUMABLE IMPLEMENTATION
    template <class WORK_CONSUMABLE_CLASS, class WORK_CLASS>
    bool WorkManager::__addConsumable( std::shared_ptr<WorkDataClass>& work_data
        , std::shared_ptr<WORK_CONSUMABLE_CLASS>& work_consumable
        , CurrentConsumableRouting routing_type
        , UInt64 thread_key )
    {
        // DO NOT USE LOCK!!!
        // if the work is not in the running state we cannot add consumables
        if (work_data->current_work_state != WorkDataClass::eWorkRunning) return false;

        // we get the producer thread id. We do it just one time to waste less resources
        if ( !work_data->producer_TID )
        {
            work_data->producer_TID = QAppNG::ThreadCounter::Instance().getThreadId();
        }

        // increment produced consumables counter
        ++work_data->produced;

        // START SELECTIVE consumable processing
        if ( work_data->work_type == WorkDataClass::No_MultiThread )
        {//
            // add current thread ID to the WORKERS TIDs (do it once)
            if ( work_data->consumer_TIDs->operator[](0) == 0 )
            {
                work_data->consumer_TIDs->operator[](0) = QAppNG::ThreadCounter::Instance().getThreadId();
            }

            // cast work_data->thread_pool pointer to FicticiousWorker< WORK_CLASS >
            std::shared_ptr< FicticiousWorker< WORK_CLASS > > NO_MULTITHREAD_WORKER
                = std::static_pointer_cast< FicticiousWorker< WORK_CLASS > >( work_data->thread_pool );

            // do the work
            bool work_done = NO_MULTITHREAD_WORKER->doWork( work_consumable, 0 );

            // increment consumed counter
            ++work_data->per_thread_consumed->operator[](0);

            return work_done;
        }//

        else if ( work_data->work_type == WorkDataClass::TrivialThreadPool )
        {//
            // cast the pointer to get access to the pool
            std::shared_ptr< TrivialThreadPool< std::shared_ptr < WORK_CONSUMABLE_CLASS >, WORK_CLASS > > thread_pool
                = std::static_pointer_cast< TrivialThreadPool< std::shared_ptr < WORK_CONSUMABLE_CLASS >, WORK_CLASS > >(work_data->thread_pool);

            // if there is place, enqueue the consumable. If there is no place and the overload_strategy
            // is Drop, drop the consumable (it is enough to not enter in the IF statement)
            // THE only difference for OVERLOAD strategy is done here. The queue is designed to BLOCK if FULL !!!
            //if (routing_type == eAutomaticRouting
            //    && (thread_pool->hasPlaceInQueue() || work_data->overload_strategy == WorkDataClass::Wait) )
            //{
            //    return thread_pool->addConsumable( work_consumable );
            //}
            //else if ( routing_type == eUserDefinedRouting
            //    && (thread_pool->hasPlaceInQueue( thread_key ) || work_data->overload_strategy == WorkDataClass::Wait) )
            //{
            //    return thread_pool->addConsumable( work_consumable, thread_key );
            //}

            // ThreadPool Queues are default-BLOCKING, so we eneque a consumable it either Wait Policy is enabled or thread_pool.queue[thread_key] has place.
            if ( work_data->overload_strategy == WorkDataClass::Wait || thread_pool->hasPlaceInQueue( thread_key ) )
            {
                return thread_pool->addConsumable( work_consumable, thread_key );
            }
        }//

        // no worker defined on consumable or DROP Policy enabled and no place in queue, we drop it
        ++work_data->dropped;

        return false;
    }; // END of __addConsumable(...)

    // --------------------------------------------------------------------------------------------------------
}

