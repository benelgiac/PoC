/** ===================================================================================================================
* @file    WorkManager
*
* @brief   WorkManager 2.0 .cpp Implementation 
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

#include "WorkManager.h"
#include <pugixml/pugixml.hpp>

// --------------------------------------------------------------------------------------------------------

namespace QAppNG 
{
    // --------------------------------------------------------------------------------------------------------

    WorkManager::~WorkManager()
    {
        // USE LOCK
        boost::unique_lock<boost::mutex> lock(m_mutex);

        // disable getStatus method
        disable_get_status = true;

        // vaporize thread pool
        works_vector.clear();
        works_map.clear();

        // enable getStatus method
        disable_get_status = false;
    }

    // --------------------------------------------------------------------------------------------------------

    WorkDataClass::work_type_enum WorkManager::getWorkType( const std::string& work_name )
    {
        assert(works_map.count(work_name));
        return works_map[work_name]->work_type;
    };

    // --------------------------------------------------------------------------------------------------------

    size_t WorkManager::getWorkUniqueId( const std::string& work_name )
    {
        assert(works_map.count(work_name)); 
        return works_map[work_name]->work_unique_id; 
    };

    // --------------------------------------------------------------------------------------------------------

    UInt32 WorkManager::getNumberOfWorkers( const std::string& work_name )
    {
        assert(works_map.count(work_name));
        return works_map[work_name]->number_of_workers;
    };

    // --------------------------------------------------------------------------------------------------------

    UInt32 WorkManager::getQueueSize( const std::string& work_name )
    {
        assert(works_map.count(work_name));
        return works_map[work_name]->thread_data_setup.max_queue_size;
    };

    // --------------------------------------------------------------------------------------------------------

    UInt64 WorkManager::getNumberOfDropped( const std::string& work_name )
    {
        assert(works_map.count(work_name)); return works_map[work_name]->dropped;
    };

    // --------------------------------------------------------------------------------------------------------

    UInt64 WorkManager::getNumberOfConsumed( const std::string& work_name )
    {
        assert(works_map.count(work_name));

        // we must sum consumed for each worker involved in the WORK
        UInt64 consumed(0);
        for ( size_t i = 0; i < works_map[work_name]->number_of_workers; i++ )
            consumed += works_map[work_name]->per_thread_consumed->operator[](i);

        return consumed;
    };

    UInt64 WorkManager::getNumberOfProduced( const std::string& work_name )
    {
        assert(works_map.count(work_name));

        return works_map[work_name]->produced;
    };	
    // --------------------------------------------------------------------------------------------------------

    std::string WorkManager::getStatus()
    {
        std::stringstream output; output.clear(); output.str("");

        // DO NOT USE LOCK for status (other ways we get dead lock...)
        if (disable_get_status) 
        {
            output << "getStatus Not Available (stopping a Work?)...";
            return output.str();
        }

        // Define Iterator
        std::unordered_map<std::string, std::shared_ptr<WorkDataClass> >::iterator it;

        for ( it = works_map.begin(); it != works_map.end(); ++it )
        {
            std::shared_ptr<WorkDataClass> work_data = it->second;

            // get work_type
            std::string type("");
            if      (work_data->work_type == WorkDataClass::No_MultiThread)    type = "No_MultiThread";
            else if (work_data->work_type == WorkDataClass::TrivialThreadPool) type = "TrivialThreadPool";
            else if (work_data->work_type == WorkDataClass::Disabled)          type = "Disabled";

            // get work_status
            std::string work_status("");
            if      (work_data->current_work_state == WorkDataClass::eWorkRunning) work_status = "Running";
            else if (work_data->current_work_state == WorkDataClass::eWorkStopped) work_status = "Stopped";

            std::stringstream consumer_TIDs_stream; consumer_TIDs_stream.str("");
            std::stringstream last_sleeps_stream; last_sleeps_stream.str("");
            std::stringstream threads_percentual_load; threads_percentual_load.str("");
            for ( size_t i = 0; i < work_data->consumer_TIDs->size(); i++ )
            {
                if (work_data->consumer_TIDs->operator[](i) > 0)
                {
                    consumer_TIDs_stream << work_data->consumer_TIDs->operator[](i) << ", ";
                }

                if (work_data->per_thread_last_sleep_msec->operator[](i) > 0)
                {
                    last_sleeps_stream << work_data->per_thread_last_sleep_msec->operator[](i) << ", ";
                }

                if (work_data->per_thread_assigned->operator[](i) > 0)
                {
                    threads_percentual_load << std::fixed << std::setprecision(0) << (100.00 * work_data->per_thread_assigned->operator[](i) / work_data->produced )    << "%, ";
                }
            }

            if (consumer_TIDs_stream.str() == "") consumer_TIDs_stream.str("-");
            if (last_sleeps_stream.str() == "") last_sleeps_stream.str("-");
            if (threads_percentual_load.str() == "") threads_percentual_load.str("-");

            UInt64 number_of_calls = 0;
            UInt64 consumed = 0;
            std::ostringstream per_thread_used_queue_output;
            for ( size_t i = 0; i < work_data->number_of_workers; i++ )
            {
                number_of_calls += work_data->per_thread_number_of_calls->operator[](i);
                consumed += work_data->per_thread_consumed->operator[](i);

                UInt64 per_thread_used_queue = work_data->per_thread_assigned->operator[](i) - work_data->per_thread_consumed->operator[](i);
                float per_thread_used_queu_percentage =  100.00 * per_thread_used_queue / work_data->thread_data_setup.max_queue_size;
                per_thread_used_queue_output << std::resetiosflags( std::ios::floatfield ) << per_thread_used_queue << " (" << std::fixed << std::setprecision(2) << per_thread_used_queu_percentage << "%), ";
            }

            float consumables_per_call;
            ( number_of_calls > 0 ) 
                ? consumables_per_call = float( float(consumed) / float(number_of_calls) )
                : consumables_per_call = 0.0;

            UInt64 input_average_used_queue = work_data->produced - consumed;
            float input_average_used_queue_percentage = 100.00 * input_average_used_queue / work_data->number_of_workers / work_data->thread_data_setup.max_queue_size;
            std::ostringstream input_average_used_queue_output;
            input_average_used_queue_output << std::resetiosflags( std::ios::floatfield ) << input_average_used_queue << " (" << std::fixed << std::setprecision(2) << input_average_used_queue_percentage << "%)";

            output << std::endl;
            output << "WORK Name: "                 << work_data->work_name                        << std::endl;
            output << "|- Work Status           = " << work_status                                 << std::endl;
            output << "|- Multithread           = " << type                                        << std::endl;
            output << "|- Pool Producers IDs    = ";
            if ( work_data->producer_TID )
            {
                output << *(work_data->producer_TID)                                               << std::endl;
            }
            else
            {
                output << "-"                                                                      << std::endl;
            }
            output << "|- Pool Workers IDs      = " << consumer_TIDs_stream.str()                  << std::endl;
            output << "|- Number of Workers     = " << work_data->number_of_workers                << std::endl;
            output << "|- Queue Size            = " << work_data->thread_data_setup.max_queue_size << std::endl;
            output << "|- Routing Paths         = " << work_data->routing_map->size()              << std::endl;
            output << "|- Consumable produced   = " << work_data->produced                         << std::endl;
            output << "|- Consumable consumed   = " << consumed                                    << std::endl;
            output << "|- Consumable dropped    = " << work_data->dropped                          << std::endl;
            output << "|- Avg Consumables/Call  = " << consumables_per_call                        << std::endl;
            output << "|- Avg Queue Usage       = " << input_average_used_queue_output.str()       << std::endl;
            output << "|- Thread Queue Usage    = " << per_thread_used_queue_output.str()          << std::endl;
            
            if(work_data->thread_data_setup.adaptive_load_balance == true)
                output << "|- Thread Sleep Times    = " << last_sleeps_stream.str() << std::endl;
            else
                output << "|- Thread Sleep Times    = " << work_data->thread_data_setup.fixed_sleep_msec << std::endl;
                	
            output << "|- Thread Load           = " << threads_percentual_load.str()               << std::endl;
        }

        return output.str();
    };

    // --------------------------------------------------------------------------------------------------------

    bool WorkManager::loadWorkSetup( const std::string& xml_config_filename, const std::string& work_name, std::shared_ptr<WorkDataClass> work_setup )
    {
        // TODO!!! XML!!!
        pugi::xml_document xml_doc;
        std::string root("WorkManager");
        pugi::xml_parse_result result = xml_doc.load_file( xml_config_filename.c_str() );
        if (!result)
        {
            return false;
        }

        // mi pare parecchio pi� semplice delle CAL cazzo!
        pugi::xml_node works = xml_doc.child("WorkManager");

        pugi::xml_node my_work = works.find_child_by_attribute("Work", "name", work_name.c_str());

        // #9077: if work name is not found in config file return false
        if ( !my_work )
        {
            return false;
        }

        // SET WORK TYPE
        if ( my_work.attribute("type") )
        {
            std::string type = my_work.attribute("type").value();

            if      (type == "No_MultiThread")    work_setup->work_type = WorkDataClass::No_MultiThread;
            else if (type == "TrivialThreadPool") work_setup->work_type = WorkDataClass::TrivialThreadPool;
            else if (type == "Disabled")          work_setup->work_type = WorkDataClass::Disabled;
            else
            {
                std::ostringstream errorStr;
                errorStr<<"Unknown work type:"<<type<<" in "<<xml_config_filename<<":"<< work_name <<". Valid settings:'No_MultiThread', 'TrivialThreadPool', 'Disabled'";
                throw std::runtime_error(errorStr.str());
            }
        }
        else
            work_setup->work_type = WorkDataClass::No_MultiThread;

        // SET OVERLOAD STRATEGY
        if ( my_work.attribute("overload_strategy") )
        {
            std::string overload_strategy = my_work.attribute("overload_strategy").value();

            if      (overload_strategy == "Drop") work_setup->overload_strategy = WorkDataClass::Drop;
            else if (overload_strategy == "Wait") work_setup->overload_strategy = WorkDataClass::Wait;
            else
            {
                std::ostringstream errorStr;
                errorStr<<"Unknown overload strategy:"<<overload_strategy<<" in "<<xml_config_filename<<":"<<work_name<<". Valid settings:'Drop','Wait'";
                throw std::runtime_error(errorStr.str());
            }
        }
        else
        {
            work_setup->overload_strategy = WorkDataClass::Drop;
        }

        // SET NUMBER OF WORKERS (correction based on work type are applied in WorkManager::__startWork)
        work_setup->number_of_workers = my_work.attribute("number_of_workers").as_int(1);

        // SET MAX QUEUE SIZE
        work_setup->thread_data_setup.max_queue_size = my_work.attribute("max_queue_size").as_int(100000);

        // SET MAX CONSUMABLE PER LOOP
        work_setup->thread_data_setup.max_consumables_per_loop = my_work.attribute("max_consumables_per_loop").as_int(1000);

        if ( my_work.attribute("adaptive_load_balance") )
        {
            std::string adaptive_load_balance("True");
            adaptive_load_balance = my_work.attribute("adaptive_load_balance").value();

            if      (adaptive_load_balance == "True")  work_setup->thread_data_setup.adaptive_load_balance = true;
            else if (adaptive_load_balance == "False") work_setup->thread_data_setup.adaptive_load_balance = false;
            else
            {
                std::ostringstream errorStr;
                errorStr<<"Unknown adaptive load balance:"<<adaptive_load_balance<<" in "<<xml_config_filename<<":"<<work_name<<". Valid settings:'True','False'";
                throw std::runtime_error(errorStr.str());
            }
        }
        else
            work_setup->thread_data_setup.adaptive_load_balance = true;

        work_setup->thread_data_setup.adaptive_min_sleep_msec = my_work.attribute("adaptive_min_sleep_msec").as_int(10);

        work_setup->thread_data_setup.adaptive_max_sleep_msec = my_work.attribute("adaptive_max_sleep_msec").as_int(300);

        work_setup->thread_data_setup.fixed_sleep_msec = my_work.attribute("fixed_sleep_msec").as_int(300);

        return true;
    };

    // --------------------------------------------------------------------------------------------------------

    bool WorkManager::stopWork( const std::string& work_name )
    {
        if (!works_map.count(work_name)) return false;

        // set Work to STOPPED state (so adding consumable is disabled)
        works_map[work_name]->current_work_state = WorkDataClass::eWorkStopped;

        // gracefully stop threadpools
        if ( works_map[work_name]->thread_pool_destroyer != NULL ) works_map[work_name]->thread_pool_destroyer();

        return true;
    };

    // --------------------------------------------------------------------------------------------------------

    void WorkManager::shutdown()
    {
        // stop all works
        std::unordered_map<std::string, std::shared_ptr<WorkDataClass> >::iterator it;
        
        while (works_map.size())
        {
            it = works_map.begin();
            std::string work_name = it->first;

            //Careful: this call will make it invalid so you cannot do this inside a for loop
            // as it was done before
            stopWork( work_name );
        }

        // not needed:
        //works_vector.clear();
        //works_map.clear();
    };
}
// --------------------------------------------------------------------------------------------------------

