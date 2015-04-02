/** =================================================================================================================
* @file    TrivialThreadPool.h
*
* @brief   TrivialThreadPool
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
#ifndef INCLUDE_TRIVIALTHREADPOOL_NG
#define INCLUDE_TRIVIALTHREADPOOL_NG

// Include STL & BOOST
#include <vector>
#include <boost/array.hpp>

// Other Includes
#include "core.h"
#include "TrivialCircularLockFreeQueue.h"

#include <QAppNG/ThreadCounter.h>

namespace QAppNG
{
    // --------------------------------------------------------------------------------------------------------

    // THREADS WRITE Back OPERATIVE DATA
    struct ThreadOperativeDataWriteBackClass
    {
        std::shared_ptr< std::vector<UInt64> > per_thread_assigned;
        std::shared_ptr< std::vector<UInt64> > per_thread_consumed;
        std::shared_ptr< std::vector<UInt64> > per_thread_number_of_calls;
        std::shared_ptr< std::vector<UInt64> > per_thread_last_sleep_msec;
        std::shared_ptr< std::vector<UInt64> > consumer_TIDs;
    };

    // --------------------------------------------------------------------------------------------------------
    //                                            *** ThreadDataClass ***
    // --------------------------------------------------------------------------------------------------------

    /**
      *  @author Alessandro Della Villa <alessandro.dellavilla@CommProve.com>
      *
      *  @brief simple class to store thread parameters and thread operative data
      *         the operative data is private and can be written only by TrivialThreadPool class
      *
      */
    class ThreadDataClass
    {
        // Declare TrivialThreadPool as FRIEND CLASS
        template<class CONSUMABLE_CLASS, class WORKER_CLASS> friend class TrivialThreadPool;

    public:
        ThreadDataClass()
            : work_name("")
            , max_queue_size(100000)
            , max_consumables_per_loop(10000)
            , adaptive_load_balance(true)
            , adaptive_min_sleep_msec(10)
            , adaptive_max_sleep_msec(400)
            , fixed_sleep_msec(300)
            , thread_key(0)
            , thread_id()
            , is_running(false)
            , exit_loop(false)
            , thread_num_consumed(0)
            , thread_num_assigned(0)
            , thread_num_of_calls(0)
            , thread_last_sleep(0)
        {};

        static const UInt64 INVALID_THREAD_ID = 0xFFFFFFFFFFFFFFFF;

        // work_name is copied here from work_data fot two reasons:
        // 1- it can be got with BaseWorker::getWorkName()
        // 2- TrivialThreadPool does not depend from WorkManager
        std::string work_name;

        // Init data passed to each thread
        std::shared_ptr< void > thread_init_data;

        // Setup Parameters
        UInt32 max_queue_size;
        UInt32 max_consumables_per_loop;
        bool   adaptive_load_balance;
        UInt32 adaptive_min_sleep_msec;
        UInt32 adaptive_max_sleep_msec;
        UInt32 fixed_sleep_msec;

        // pass private member values
        std::thread::id      getTID()      { return thread_id; };
        UInt64                 getKey()      { return thread_key; };
        std::string            getWorkName() { return work_name; };

    private:
        // ID Parameters
        // thread key is to identify the thread in the POOL
        // thread id is the unique TID
        UInt64 thread_key;
        std::thread::id thread_id;
        volatile bool is_running;
        volatile bool exit_loop;

        // Status Parameters
        UInt64 thread_num_consumed;
        UInt64 thread_num_assigned;
        UInt64 thread_num_of_calls;
        UInt32 thread_last_sleep;
    };

    // --------------------------------------------------------------------------------------------------------
    //                                            *** BaseWorker ***
    // --------------------------------------------------------------------------------------------------------

    class DummyInitData {};

    /**
      *  @author Alessandro Della Villa <alessandro.dellavilla@CommProve.com>
      *
      *  @brief Base Class for Workers
      *
      *         When you define your WORKER_CLASS you must derive
      *         from this class.
      */
    template<class THREAD_INIT_DATA_CLASS = DummyInitData>
    class BaseWorker
    {
        // Declare TrivialThreadPool as FRIEND CLASS
        template<class CONSUMABLE_CLASS, class WORKER_CLASS>
        friend class TrivialThreadPool;

    public:
        typedef THREAD_INIT_DATA_CLASS ThreadInitClass;

        // get Work info, NB TID identify thread in the operative system, "Key" is a unique number associated to thread inside a work!
        std::thread::id getTID()      { if (m_thread_data) return m_thread_data->getTID(); else return std::thread::id(); };
        UInt64            getKey()      { if (m_thread_data) return m_thread_data->getKey(); else return 0; };
        std::string       getWorkName() { if (m_thread_data) return m_thread_data->getWorkName(); else return std::string(""); };

        // set the thread_init_data (used by CAL_ThreadPool and No_Multithread in WorkManager)
        void setInitData( std::shared_ptr< THREAD_INIT_DATA_CLASS > _thread_init_data )
        {
            if ( !m_thread_data )
            {
                m_thread_data.reset( new ThreadDataClass );
            }

            m_thread_data->thread_init_data = _thread_init_data;
        };

        // get the work_init_data
        // NB: it is NOT POSSIBLE to call getInitData() in worker CTOR (copy is done in TrivialThreadPool loop)!!!
        inline std::shared_ptr< THREAD_INIT_DATA_CLASS > getInitData()
        {
            std::shared_ptr<THREAD_INIT_DATA_CLASS> output;

            ( m_thread_data )
                ? output = std::static_pointer_cast< THREAD_INIT_DATA_CLASS >( m_thread_data->thread_init_data )
                : output = std::shared_ptr< THREAD_INIT_DATA_CLASS >();

            return output;
        };

        // dtor
        virtual ~BaseWorker() {};

    private:
        // thread data, it is dynamic and it is different for each thread
        std::shared_ptr<ThreadDataClass> m_thread_data;
    };

    // --------------------------------------------------------------------------------------------------------
    //                                           *** TrivialThreadPool ***
    // --------------------------------------------------------------------------------------------------------

    /**
    *  @author Alessandro Della Villa <alessandro.dellavilla@CommProve.com>
    *
    *  @brief simple alternative to CAL Threadpool
    *
    *         TrivialThreadPool runs the doWork( std::shared_ptr<CONSUMABLE_CLASS> consumable, UInt64 thread_id )
    *         of WORKER_CLASS that is the Worker.
    */
    template<class CONSUMABLE_CLASS, class WORKER_CLASS>
    class TrivialThreadPool
    {
    public:
        TrivialThreadPool( const std::string& work_name
                         , int num_workers
                         , ThreadDataClass* thread_data=NULL
                         , std::shared_ptr<ThreadOperativeDataWriteBackClass> _write_back_data = std::shared_ptr<ThreadOperativeDataWriteBackClass>() )
            : pool_total_assigned(0)
            , pool_total_consumed(0)
            , pool_threads(ThreadCounter::MAX_NUMBER_OF_THREADS)
            , allStarted(false)
            , allStopped(false)
        {
            // If no ThreadDataClass is given create a new one with default values
            if (thread_data==NULL)
                thread_data = new ThreadDataClass;

            // If a write_back_data is given, set it
            if (_write_back_data)
                write_back_data = _write_back_data;

            // Set the member number_of_threads
            number_of_threads = num_workers;

            // Set length for queues vector
            threads_queues.reserve(num_workers);
            thread_datas.reserve(num_workers);
            workers.reserve(num_workers);

            // Set max total places in all queues of all threads
            pool_max_place_in_queues = number_of_threads * thread_data->max_queue_size;

            for (int worker = 0; worker < num_workers; worker++)
            {
                // Store a copy of thread_data in the thread_datas vector
                thread_datas.push_back( std::shared_ptr<ThreadDataClass> (new ThreadDataClass) );
                *thread_datas[worker] = *thread_data;
                thread_datas[worker]->work_name = work_name;

                // Set thread_key (it is used to cycle workers)
                thread_datas[worker]->thread_key = worker;

                // Create Queue
                threads_queues.push_back( new TrivialCircularLockFreeQueue<CONSUMABLE_CLASS>(thread_datas[worker]->max_queue_size) );

                // Create new WORKER_CLASS and store pointer (NUOVA parte aggiunta per Mike)
                workers.push_back( new WORKER_CLASS() );

                workers.back()->m_thread_data = thread_datas[worker];

                // Start Pool Threads
                pool_threads[worker].reset( new std::thread( [this, worker] { this->ThreadMainLoop( this->thread_datas[worker]); } ) );

                // Wait for Complete Thread Start-Up
                while ( !thread_datas[worker]->is_running )
                {
                    std::this_thread::sleep_for( std::chrono::milliseconds(10) );
                }
            }

            // Set all-thread-started to true
            allStarted = true;
        };

        //______________________________________________________
        bool addConsumable( CONSUMABLE_CLASS& consumable )
        {
            //  AUTOMATIC CONSUMABLE ROUTING -> calculate automatic_thread_key (cycling workers)
            size_t automatic_thread_key( size_t(pool_total_assigned % number_of_threads) );

            // using automatic_thread_key the queues are cycled so consumables are equally divided between all workers of the pool
            TrivialCircularLockFreeQueue<CONSUMABLE_CLASS>& thread_queue = *threads_queues[ automatic_thread_key ];

            //increment per thread assigned
            ++thread_datas[ automatic_thread_key ]->thread_num_assigned;

            // push consumable in the queue
            thread_queue.push( consumable );

            // increment pool total assigned consumables counter
            pool_total_assigned++;

            return true;
        };

        //______________________________________________________
        bool addConsumable( CONSUMABLE_CLASS& consumable, UInt64 thread_key )
        {
            // USER DEFINED CONSUMABLE ROUTING -> the consumable is given to a specific worker
            TrivialCircularLockFreeQueue<CONSUMABLE_CLASS>& thread_queue = *threads_queues[ size_t(thread_key) ];

            //increment per thread assigned
            ++thread_datas[ size_t(thread_key) ]->thread_num_assigned;

            // push consumable in the queue
            thread_queue.push( consumable );

            // increment pool total assigned consumables counter
            pool_total_assigned++;

            return true;
        };

        //______________________________________________________
        void stopThreadPool()
        {
            // #9263: stop threads in reverse order of creation
            for (Int8 tid = workers.size() - 1 ; tid >= 0; --tid)
            {
                workers[tid]->m_thread_data->exit_loop = true;

                pool_threads[tid]->join();
            }

            allStopped = true;
        };

        //______________________________________________________
        bool hasPlaceInQueue()
        {
            return ( pool_total_assigned - pool_total_consumed < pool_max_place_in_queues );
        };

        //______________________________________________________
        bool hasPlaceInQueue( UInt64 thread_key )
        {
            return !threads_queues[ size_t(thread_key) ]->full();
        };

        // ____________________________________________________________________________________________________________
        //                                            GET INFO ABOUT WORK

        UInt64 getPoolTotalAssigned() { return pool_total_assigned; };
        UInt64 getPoolTotalConsumed() { return pool_total_consumed; };

        std::vector<UInt64> getPoolPerThreadConsumed()
        { std::vector<UInt64> output; for (int i=0; i<number_of_threads; i++) output.push_back( thread_datas[i]->thread_num_consumed ); return output; };

        std::vector<UInt64> getPoolNumberOfThreadCalls()
        { std::vector<UInt64> output; for (int i=0; i<number_of_threads; i++) output.push_back( thread_datas[i]->thread_num_of_calls ); return output; };

        // ____________________________________________________________________________________________________________

        //______________________________________________________
        ~TrivialThreadPool()
        {
            for (size_t tid = 0; tid < number_of_threads; tid++)
            {
                thread_datas[tid]->thread_init_data.reset();

                // DELETE worker data
                delete workers[tid];
            }
        };

    private:
        UInt64                                                           number_of_threads;
        UInt64                                                           pool_total_assigned;
        UInt64                                                           pool_total_consumed;
        UInt64                                                           pool_max_place_in_queues;
        std::vector< std::unique_ptr< std::thread > >                    pool_threads;
        std::vector< TrivialCircularLockFreeQueue<CONSUMABLE_CLASS>* >   threads_queues;
        std::vector< std::shared_ptr<ThreadDataClass> >                thread_datas;
        std::vector< WORKER_CLASS* >                                     workers;

        // i dati vengono scritti sulla struttura puntata da questo shared pointer
        // che viene passato dall'applicazione client in modo che la stessa possa leggere
        // le statistiche dei threads. I dati scritti qui non hanno nessun altro scopo
        std::shared_ptr<ThreadOperativeDataWriteBackClass> write_back_data;

        // bool to say when startup or termination is complete;
        bool allStarted;
        bool allStopped;

        //______________________________________________________
        bool ProcessConsumable( CONSUMABLE_CLASS& consumable, std::shared_ptr<ThreadDataClass>& thread_data )
        {
            // do the work, the following methods comes from the class WORKER_CLASS
            bool work_done = workers[ size_t(thread_data->thread_key) ]->doWork( consumable, thread_data->thread_key );

            // Increment thread consumed consumables counter.
            thread_data->thread_num_consumed++;

            return work_done;
        };

        //______________________________________________________
        void updatePoolStatistics()
        {
            // In each pool the thread_key = 0 THREAD has to collect
            // the total consumed consumables from all threads

            // Update pool_total_consumed (it is important for hasPlaceInQueue() method)
            UInt64 total = 0;
            for (size_t i = 0; i < number_of_threads; ++i)
                total += thread_datas[i]->thread_num_consumed;

            pool_total_consumed = total;

            // ATTENZIONE
            // dato che è compito del thread 0 aggiornare il numero di consumabili
            // consumati è possibile che chiamando la funzione hasPlaceInQueue
            // ci venga restituito false quando in realtà qualche posto in coda c'è.
            // Questo non è un problema, l'importante è che non avvenga il contrario!

            if ( write_back_data )
            {
                for (size_t i = 0; i < number_of_threads; ++i)
                {
                    write_back_data->per_thread_assigned->operator[](i)        = thread_datas[i]->thread_num_assigned;
                    write_back_data->per_thread_consumed->operator[](i)        = thread_datas[i]->thread_num_consumed;
                    write_back_data->per_thread_last_sleep_msec->operator[](i) = thread_datas[i]->thread_last_sleep;
                    write_back_data->per_thread_number_of_calls->operator[](i) = thread_datas[i]->thread_num_of_calls;
                }
            }
        };

        // ____________________________________________________________________
        // Thread Main Loop
        // ____________________________________________________________________
        void ThreadMainLoop( std::shared_ptr<ThreadDataClass> thread_data )
        {
            // get thread key:
            size_t thread_key = size_t(thread_data->thread_key);

            // OPERAZIONI SPECIALI nel caso in cui ereditiamo dal BaseWorker
            // (potrei fare come sopra e prendere una volta per tutte un pointer al worker da passare a ProcessConsumable)
            workers[thread_key]->m_thread_data = thread_data;

            thread_data->thread_id = std::this_thread::get_id();

            // Convert boost id to numeric value, just to store it in consumer_TIDs. (Is it useful?)
            //std::stringstream convert_tid(""); convert_tid.clear();
            //std::thread::id id = std::this_thread::get_id();
            //convert_tid << id;
            //std::string stringa = convert_tid.str();
            //UInt32 TID;
            //sscanf(stringa.c_str(),"%i", &TID);

            UInt64 TID( QAppNG::ThreadCounter::Instance().getThreadId() );

            // store TID (the Thread ID) in write_back_data (if it was provided)
            if (write_back_data)
                write_back_data->consumer_TIDs->operator[]( thread_key ) = TID;

            // get handler to the thread queue
            TrivialCircularLockFreeQueue<CONSUMABLE_CLASS>& queue = *threads_queues[thread_key];

            // set counters to limit consumables per loop
            UInt64 consumed = 0;
            UInt32 max_to_consume = thread_data->max_consumables_per_loop;

            // set adaptive parameter
            UInt64 used_queue_t0         = 0;
            UInt64 used_queue_t1         = 0;
            int    queue_tendency        = 0;
            UInt64 queue_limit           = thread_data->max_queue_size / 2;
            UInt64 sleep_constant_factor = static_cast<UInt64>( 0.02f * thread_data->max_queue_size );
            UInt32 toSleep               = thread_data->adaptive_min_sleep_msec;
            int    sleep_step            = int( (thread_data->adaptive_max_sleep_msec - thread_data->adaptive_min_sleep_msec) / 100 );

            if (!sleep_step) sleep_step++;

            // store TID (the Thread ID) in the list
            // when it is stored in thread_data->thread_id the CTOR understand
            // that startup is complete
            thread_data->is_running = true;

            // BEGIN THREAD LOOP
            while ( !thread_data->exit_loop )
            {
                if ( !queue.empty() && consumed < max_to_consume )
                {
                    // copy front element from thread queue
                    CONSUMABLE_CLASS consumable = queue.front();

                    // pop front element from thread queue
                    queue.pop();

                    // call processing
                    ProcessConsumable( consumable, thread_data );

                    // SPECIAL CODE FOR THREAD 0: Update Statistics
                    if (thread_key == 0 && allStarted)
                    {
                        updatePoolStatistics();
                    }

                    consumed++;
                }
                else
                {
                    if (consumed>0)
                    {
                        // Increment number of calls counter
                        thread_data->thread_num_of_calls++;
                        consumed = 0;
                    }

                    // do SLEEPING
                    if (thread_data->adaptive_load_balance)
                    {
                        // sample Queue Usage before sleeping
                        used_queue_t0 = queue.getUsedQueue();

                        // if the queue is full above the limit (50%) reduce the sleeping time to the minimum
                        if ( used_queue_t0 > queue_limit )
                            toSleep = thread_data->adaptive_min_sleep_msec;

                        // controllo aggiunto onde evitare di dormire troppo in casi particolari
                        if (toSleep > thread_data->adaptive_max_sleep_msec)
                            toSleep = thread_data->adaptive_max_sleep_msec;

                        // DO SLEEPING
                        std::this_thread::sleep_for( std::chrono::milliseconds(toSleep) );

                        // sample Queue Usage after sleeping
                        used_queue_t1 = queue.getUsedQueue();

                        // store the last sleep duration for the thread
                        thread_data->thread_last_sleep = toSleep;

                        // evaluate filling tendency (to increase or decrease sleeping time)
                        queue_tendency = int( used_queue_t1 - used_queue_t0 ) - sleep_constant_factor;

                        // do adaptive correction to toSleep
                        if ( (queue_tendency < 0) && (toSleep < thread_data->adaptive_max_sleep_msec) )
                            toSleep += sleep_step;

                        if ( (queue_tendency > 0) && (toSleep > thread_data->adaptive_min_sleep_msec) )
                            toSleep -= sleep_step;
                    }
                    else
                    {
                        std::this_thread::sleep_for( std::chrono::milliseconds( thread_data->fixed_sleep_msec ) );
                    }
                    // end DO SLEEPING
                }
            };
            // END THREAD LOOP

            // Let's flush the  queues
            while ( !queue.empty())
            {
                // copy front element from thread queue
                CONSUMABLE_CLASS consumable = queue.front();

                // pop front element from thread queue
                queue.pop();

                // call processing
                ProcessConsumable( consumable, thread_data );

                // SPECIAL CODE FOR THREAD 0: Update Statistics
                if (thread_key == 0 && allStarted)
                {
                    updatePoolStatistics();
                }
            }

            // DELETE queue
            delete threads_queues[thread_key];
            threads_queues[thread_key] = NULL;

            // Release TID (Thread ID), useful to understand that thread termination is complete
            thread_data->is_running = false;
        };
    };

    // --------------------------------------------------------------------------------------------------------
}
#endif // INCLUDE_TRIVIALTHREADPOOL_NG



