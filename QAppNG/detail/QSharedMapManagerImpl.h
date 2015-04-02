#ifndef Q_SHARED_MAP_MGR_IMPL_H_NG
#define Q_SHARED_MAP_MGR_IMPL_H_NG

/** ===================================================================================================================
* @file    SharedMapManager
*
* @brief   SharedMapManager is a class to handle maps shared
*          between threads with minimum use of locks, automatic SYNCH and PURGE
*
* @copyright
*
* @history
* REF#        Who                                                              When          What
* #3062       A. Della Villa, A. Manetti, F. Gragnani, R. Buti, S. Luceri      Feb-2009      Original development
* #4171       A. Della Villa                                                   Oct-2009      SMM version 2.0
* #5734       A. Della Villa                                                   Dec-2010      SMM version 3.0 
*                                                                                            ( new locks, locks policy, timers
*                                                                                             , time storage, new methods etc )
* #5848       R. Buti                                                          Mar 2011      Added masetrSize and slaveSize methods
* #5975       A. Della Villa                                                   Apr-2011      added SET/DEL/UPD delegates
*
* @endhistory
* ===================================================================================================================
*/

#include <QAppNG/ThreadCounter.h>

namespace QAppNG
{

    // --------------------------------------------------------------------------------------------------
    //                                       *** SharedMap ***
    // --------------------------------------------------------------------------------------------------

    template<class KEY_CLASS, class VALUE_CLASS>
    SharedMap<KEY_CLASS, VALUE_CLASS>::SharedMap( std::shared_ptr< MasterMap< KEY_CLASS, VALUE_CLASS > > master_map_back_ptr, fastdelegate::FastDelegate0<UInt64> virtual_timer_function )
        : m_master_map_back_ptr(master_map_back_ptr)
        , m_virtual_timer_function(virtual_timer_function)
    {
    }

    // --------------------------------------------------------------------------------------------------

    template<class KEY_CLASS, class VALUE_CLASS>
    bool SharedMap<KEY_CLASS, VALUE_CLASS>::get( const KEY_CLASS& key, VALUE_CLASS& value )
    {
        // acquire back pointer to master
        std::shared_ptr< MasterMap< KEY_CLASS, VALUE_CLASS > > master_map( m_master_map_back_ptr.lock() );

        // get a shared READ LOCK to SharedMap mutex
        boost::shared_lock<boost::shared_mutex> shared_map_read_lock( m_shared_map_mutex );

        if ( m_data_map.count(key) )
        {
            value = m_data_map[key].second;
            return true;
        }

        shared_map_read_lock.unlock();
    
        // get a shared READ LOCK to MasterMap mutex
        boost::shared_lock<boost::shared_mutex> master_map_read_lock( master_map->m_master_map_mutex );
    
        if ( master_map.get() && master_map->m_data_map.count(key) )
        {
            value = master_map->m_data_map[key].second;
            return true;
        }

        return false;
    }

    // --------------------------------------------------------------------------------------------------

    template<class KEY_CLASS, class VALUE_CLASS>
    bool SharedMap<KEY_CLASS, VALUE_CLASS>::getTime( const KEY_CLASS& key, UInt64& value )
    {
        // acquire back pointer to master
        std::shared_ptr< MasterMap< KEY_CLASS, VALUE_CLASS > > master_map( m_master_map_back_ptr.lock() );

        // get a shared READ LOCK to SharedMap mutex
        boost::shared_lock<boost::shared_mutex> shared_map_read_lock( m_shared_map_mutex );

        if ( m_data_map.count(key) )
        {
            // we return the TIME of the entry
            value = m_data_map[key].first;
            return true;
        }
    
        shared_map_read_lock.unlock();

        // get a shared READ LOCK to MasterMap mutex
        boost::shared_lock<boost::shared_mutex> master_map_read_lock( master_map->m_master_map_mutex );
    
        if ( master_map.get() && master_map->m_data_map.count(key) )
        {
            // we return the TIME of the entry
            value = master_map->m_data_map[key].first;
            return true;
        }

        return false;
    }

    // --------------------------------------------------------------------------------------------------

    template<class KEY_CLASS, class VALUE_CLASS>
    bool SharedMap<KEY_CLASS, VALUE_CLASS>::getAge( const KEY_CLASS& key, UInt64& value )
    {
        // acquire back pointer to master
        std::shared_ptr< MasterMap< KEY_CLASS, VALUE_CLASS > > master_map( m_master_map_back_ptr.lock() );

        // get a shared READ LOCK to SharedMap mutex
        boost::shared_lock<boost::shared_mutex> shared_map_read_lock( m_shared_map_mutex );

        if ( m_data_map.count(key) && m_virtual_timer_function )
        {
            // we calculate the AGE of the entry
            value = m_virtual_timer_function() - m_data_map[key].first;
            return true;
        }

        shared_map_read_lock.unlock();

        // get a shared READ LOCK to MasterMap mutex
        boost::shared_lock<boost::shared_mutex> master_map_read_lock( master_map->m_master_map_mutex );

        if ( master_map.get() && master_map->m_data_map.count(key) && m_virtual_timer_function )
        {
            // we calculate the AGE of the entry
            value = m_virtual_timer_function() - master_map->m_data_map[key].first;
            return true;
        }

        return false;
    }

    // --------------------------------------------------------------------------------------------------

    template<class KEY_CLASS, class VALUE_CLASS>
    bool SharedMap<KEY_CLASS, VALUE_CLASS>::set( const KEY_CLASS& key, const VALUE_CLASS& value )
    {
        // get a unique WRITE LOCK to SharedMap mutex
        boost::unique_lock<boost::shared_mutex> shared_map_write_lock( m_shared_map_mutex );

        // create entry to insert in the map using clock also if the delegate is defined
        std::pair<UInt64, VALUE_CLASS> value_with_time;
        ( m_virtual_timer_function ) ? value_with_time = std::make_pair(m_virtual_timer_function(), value) : value_with_time = std::make_pair(0, value);

        // try to insert as new element
        std::pair<typename MapType::iterator, bool> rv = m_data_map.insert( std::make_pair(key, value_with_time) );
        if (rv.second)
        {
            // since insert was successful it was a new element
        }
        else
        {
            // it is already in the map we update it
            m_data_map[key] = value_with_time;
        }

        // disable delete status if it was set in the delete map
        if ( m_deleted_data_map.count(key) ) m_deleted_data_map[key] = false;

        return rv.second;
    }

    // --------------------------------------------------------------------------------------------------

    template<class KEY_CLASS, class VALUE_CLASS>
    bool SharedMap<KEY_CLASS, VALUE_CLASS>::set( const KEY_CLASS& key, const VALUE_CLASS& value, const UInt64& entry_time )
    {
        // get a unique WRITE LOCK to SharedMap mutex
        boost::unique_lock<boost::shared_mutex> shared_map_write_lock( m_shared_map_mutex );

        // create entry to insert in the map using clock also if the delegate is defined
        std::pair<UInt64, VALUE_CLASS> value_with_time = std::make_pair(entry_time, value);

        // try to insert as new element
        std::pair<typename MapType::iterator, bool> rv = m_data_map.insert( std::make_pair(key, value_with_time) );
        if (rv.second)
        {
            // since insert was successful it was a new element
        }
        else
        {
            // it is already in the map we update it
            m_data_map[key] = value_with_time;
        }

        // disable delete status if it was set in the delete map
        if ( m_deleted_data_map.count(key) ) m_deleted_data_map[key] = false;

        return rv.second;
    }

    // --------------------------------------------------------------------------------------------------

    template<class KEY_CLASS, class VALUE_CLASS>
    bool SharedMap<KEY_CLASS, VALUE_CLASS>::del( const KEY_CLASS& key )
    {
        // get a unique WRITE LOCK to SharedMap mutex
        boost::unique_lock<boost::shared_mutex> shared_map_write_lock( m_shared_map_mutex );

        // logic delete (to be applied to master map)
        if ( m_deleted_data_map.count(key) ) 
        {
            m_deleted_data_map[key] = true;
        }
        else
        {
            m_deleted_data_map.insert( std::make_pair(key, true) );
        }

        // local delete
        if ( m_data_map.count(key) )
        {
            m_data_map.erase(key);
        }

        return true;
    }

    // --------------------------------------------------------------------------------------------------

    template<class KEY_CLASS, class VALUE_CLASS>
    bool SharedMap<KEY_CLASS, VALUE_CLASS>::touch( const KEY_CLASS& key )
    {
        if ( !m_virtual_timer_function ) return false;

        // acquire back pointer to master
        std::shared_ptr< MasterMap< KEY_CLASS, VALUE_CLASS > > master_map( m_master_map_back_ptr.lock() );

        // get a shared READ LOCK to SharedMap mutex
        boost::shared_lock<boost::shared_mutex> shared_map_read_lock( m_shared_map_mutex );

        // search entry in SharedMap
        if ( m_data_map.count(key) )
        {
            // we COUNT on the fact that the following operation is atomic so READ LOCK it is enough
            m_data_map[key]->first = m_virtual_timer_function();
            return true;
        }

        shared_map_read_lock.unlock();

        // get a shared READ LOCK to MasterMap mutex
        boost::shared_lock<boost::shared_mutex> master_map_read_lock( master_map->m_master_map_mutex );

        // search entry in MasterMap
        if ( master_map.get() && master_map->m_data_map.count(key) )
        {
            // we COUNT on the fact that the following operation is atomic so READ LOCK it is enough
            master_map->m_data_map[key]->first = m_virtual_timer_function();
            return true;
        }

        return false;
    }

    // --------------------------------------------------------------------------------------------------

    template<class KEY_CLASS, class VALUE_CLASS>
    size_t SharedMap<KEY_CLASS, VALUE_CLASS>::count( const KEY_CLASS& key )
    {
        // acquire back pointer to master
        std::shared_ptr< MasterMap< KEY_CLASS, VALUE_CLASS > > master_map( m_master_map_back_ptr.lock() );

        // get a shared READ LOCK to SharedMap mutex
        boost::shared_lock<boost::shared_mutex> shared_map_read_lock( m_shared_map_mutex );


        if ( m_deleted_data_map.count(key) )
        {
            // Case A: entry has been logically deleted
            return 0;
        }
    
        if ( m_data_map.count(key) )
        {
            // Case B: we found it in SharedMap
            return m_data_map.count(key);
        }

        shared_map_read_lock.unlock();

        // get a shared READ LOCK to MasterMap mutex
        boost::shared_lock<boost::shared_mutex> master_map_read_lock( master_map->m_master_map_mutex );

        if ( master_map.get() && master_map->m_data_map.count(key) )
        {
            // Case C: we found it in MasterMap
            return master_map->m_data_map.count(key);
        }
    
        // Case D: we didn't find it
        return 0;
    }

    // --------------------------------------------------------------------------------------------------

    template<class KEY_CLASS, class VALUE_CLASS>
    size_t SharedMap<KEY_CLASS, VALUE_CLASS>::slaveSize()
    {
        // get a shared READ LOCK to SharedMap mutex
        boost::shared_lock<boost::shared_mutex> shared_map_read_lock( m_shared_map_mutex );

        return m_data_map.size();
    }

    // --------------------------------------------------------------------------------------------------

    template<class KEY_CLASS, class VALUE_CLASS>
    size_t SharedMap<KEY_CLASS, VALUE_CLASS>::masterSize()
    {
        // acquire back pointer to master
        std::shared_ptr< MasterMap< KEY_CLASS, VALUE_CLASS > > master_map( m_master_map_back_ptr.lock() );

        return master_map->m_data_map.size();

    }
    // --------------------------------------------------------------------------------------------------

    template<class KEY_CLASS, class VALUE_CLASS>
    void SharedMap<KEY_CLASS, VALUE_CLASS>::getKeys( std::vector<KEY_CLASS>& key_vector )
    {


        // acquire back pointer to master
        std::shared_ptr< MasterMap< KEY_CLASS, VALUE_CLASS > > master_map( m_master_map_back_ptr.lock() );

        if (!master_map)
        {
            return;
        }

        // ATTENTION: since the keys are collected only from MASTER MAP before to get them we do SYNCH
        master_map->synchMapset();

        // clear output vector
        key_vector.clear();

        // get a shared READ LOCK to MasterMap mutex
        boost::shared_lock<boost::shared_mutex> master_map_read_lock( master_map->m_master_map_mutex );

        // we try to collect the keys from MasterMap
        for (typename MapType::iterator it = master_map->m_data_map.begin(); it != master_map->m_data_map.end(); ++it)
        {
            key_vector.push_back( it->first );
        }
    }

    // --------------------------------------------------------------------------------------------------

    template<class KEY_CLASS, class VALUE_CLASS>
    void SharedMap<KEY_CLASS, VALUE_CLASS>::iterateAndRunFunctionOnKeys( fastdelegate::FastDelegate1< const KEY_CLASS&, void> function_to_execute_on_element_key )
    {
        // acquire back pointer to master
        std::shared_ptr< MasterMap< KEY_CLASS, VALUE_CLASS > > master_map( m_master_map_back_ptr.lock() );

        // ATTENTION: a SYCH is executed before iteration
        master_map->synchMapset();

        // get a shared READ LOCK to MasterMap mutex
        boost::shared_lock<boost::shared_mutex> master_map_read_lock( master_map->m_master_map_mutex );

        for ( typename MapType::iterator element = master_map->m_data_map.begin(); element != master_map->m_data_map.end(); ++element )
        {
            function_to_execute_on_element_key( element->first );
        }
    }

    // --------------------------------------------------------------------------------------------------

    template<class KEY_CLASS, class VALUE_CLASS>
    void SharedMap<KEY_CLASS, VALUE_CLASS>::iterateAndRunFunctionOnValues( fastdelegate::FastDelegate1< VALUE_CLASS&, void> function_to_execute_on_element_value )
    {
        // acquire back pointer to master
        std::shared_ptr< MasterMap< KEY_CLASS, VALUE_CLASS > > master_map( m_master_map_back_ptr.lock() );

        // ATTENTION: a SYCH is executed before iteration
        master_map->synchMapset();

        // get a shared READ LOCK to MasterMap mutex
        boost::shared_lock<boost::shared_mutex> master_map_read_lock( master_map->m_master_map_mutex );

        for ( typename MapType::iterator element = master_map->m_data_map.begin(); element != master_map->m_data_map.end(); ++element )
        {
            function_to_execute_on_element_value( element->second.second );
        }
    }

    // --------------------------------------------------------------------------------------------------

    template<class KEY_CLASS, class VALUE_CLASS>
    void SharedMap<KEY_CLASS, VALUE_CLASS>::dumpToFile(std::string map_dump_filename)
    {
        // acquire back pointer to master
        std::shared_ptr< MasterMap< KEY_CLASS, VALUE_CLASS > > master_map(m_master_map_back_ptr.lock());

        // ATTENTION: a SYCH is executed before iteration
        master_map->synchMapset();

        // get a shared READ LOCK to MasterMap mutex
        boost::shared_lock<boost::shared_mutex> master_map_read_lock(master_map->m_master_map_mutex);

        //Open file              
        std::ofstream  file_stream;
        file_stream.open(map_dump_filename.c_str(), std::ios_base::binary);

        if (file_stream.is_open())
        {
            //loop over map and dump each element to file
            for (typename MapType::iterator element = master_map->m_data_map.begin(); element != master_map->m_data_map.end(); ++element)
            {
                if (dumpKeyValue<KEY_CLASS, VALUE_CLASS>(element->first, element->second.second, file_stream) == false)
                {
                    break;
                }
            }
        }

        //Close file
        file_stream.close();
    }

    // --------------------------------------------------------------------------------------------------

    template<class KEY_CLASS, class VALUE_CLASS>
    void SharedMap<KEY_CLASS, VALUE_CLASS>::loadFromFile(std::string map_dump_filename)
    {
        //Open file              
        std::ifstream  file_stream;
        file_stream.open(map_dump_filename.c_str(), std::ios_base::binary);

        if (file_stream.is_open())
        {
            // acquire back pointer to master
            std::shared_ptr< MasterMap< KEY_CLASS, VALUE_CLASS > > master_map(m_master_map_back_ptr.lock());

            //loop over map and load each element from file
            while (!file_stream.eof())
            {
                KEY_CLASS key;
                VALUE_CLASS value;
                if (loadKeyValue<KEY_CLASS, VALUE_CLASS>(key, value, file_stream))
                {
                    set(key, value);
                }
            }

            // ATTENTION: a SYCH is executed after iteration
            master_map->synchMapset();
        }

        //Close file
        file_stream.close();
    }

    // --------------------------------------------------------------------------------------------------
    //                                       *** MasterMap ***
    // --------------------------------------------------------------------------------------------------

    template<class KEY_CLASS, class VALUE_CLASS>
    MasterMap<KEY_CLASS, VALUE_CLASS>::~MasterMap()
    {
        // LOCK ALL SHARED MAPS, disconnect them from the master and clear all data
        for ( typename std::unordered_map<UInt64, SharedMapTypePtr>::iterator it = m_per_thread_shared_maps.begin(); it != m_per_thread_shared_maps.end(); ++it )
        {
            // get a unique WRITE LOCK to SharedMap mutex
            boost::unique_lock<boost::shared_mutex> write_lock( it->second->m_shared_map_mutex );

            // release back pointer from shared_map to master_map
            it->second->m_master_map_back_ptr.reset();

            // clear shared_map
            it->second->m_data_map.clear();
            it->second->m_deleted_data_map.clear();

            // decrease counter
            --m_total_shared_maps;
        }

        // clear master data_map
        m_data_map.clear();

        // clear map of shared maps
        m_per_thread_shared_maps.clear();
    }

    // --------------------------------------------------------------------------------------------------

    template<class KEY_CLASS, class VALUE_CLASS>
    typename MasterMap<KEY_CLASS, VALUE_CLASS>::SharedMapTypePtr MasterMap<KEY_CLASS, VALUE_CLASS>::getPerThreadSharedMap( UInt64 thread_id )
    {
        // get a unique WRITE LOCK to MasterMap mutex
        boost::unique_lock<boost::shared_mutex> write_lock( m_master_map_mutex );

        if ( !m_per_thread_shared_maps.count(thread_id) )
        {
            // create new shared map
            SharedMapTypePtr per_thread_shared_map( new SharedMapType( this->shared_from_this(), m_virtual_timer_function ) );

            // store new shared map inside the per thread map of shared maps
            m_per_thread_shared_maps.insert( std::make_pair(thread_id, per_thread_shared_map) );

            // increment counter
            ++m_total_shared_maps;
        }

        return m_per_thread_shared_maps[thread_id];
    }

    // --------------------------------------------------------------------------------------------------

    // called by SharedMapManager to sync master from shared maps
    template<class KEY_CLASS, class VALUE_CLASS>
    void MasterMap<KEY_CLASS, VALUE_CLASS>::synchMapset()
    {
        // get a unique WRITE LOCK to MasterMap mutex
        boost::unique_lock<boost::shared_mutex> write_lock( m_master_map_mutex );

        // *iterate on shared_maps
        for ( typename std::unordered_map<UInt64, SharedMapTypePtr>::iterator it = m_per_thread_shared_maps.begin(); it != m_per_thread_shared_maps.end(); ++it )
        {
            // first we must FREEZE the shared map before copy and delete
            // get a unique WRITE LOCK to SharedMap mutex
            boost::unique_lock<boost::shared_mutex> write_lock( it->second->m_shared_map_mutex );

            // *iterate on elements to APPLY Insert and Update
            for ( typename MapType::iterator element = it->second->m_data_map.begin(); element != it->second->m_data_map.end(); ++element )
            {
                // first try to insert as new element
                std::pair<typename MapType::iterator, bool> rv = m_data_map.insert( *element );
                if (rv.second)
                {
                    // it was a NEW ELEMENT
                    ++m_total_entries;

                    // if MapSet has a defined SET_EVENT delegate, run it
                    if (m_set_event_delegate != NULL)
                    {
                        m_set_event_delegate( element->first, element->second.second );
                    }
                }
                else
                {
                    // UPDATE ELEMENT - update an existing element ONLY if the element in the shared is newer
                    UInt64 shared_map_entry_time = element->second.first;
                    UInt64 master_map_entry_time = m_data_map[element->first].first;

                    if ( shared_map_entry_time > master_map_entry_time )
                    {
                        m_data_map[element->first] = element->second;

                        // if MapSet has a defined UPDATE_EVENT delegate, run it
                        if (m_upd_event_delegate != NULL)
                        {
                            m_upd_event_delegate( element->first, element->second.second );
                        }
                    }
                }
            }

            // APPLY Delete (iterate on m_deleted_data_map to apply logical delete to m_data_map)
            for ( typename std::unordered_map<KEY_CLASS, bool>::iterator element = it->second->m_deleted_data_map.begin(); element != it->second->m_deleted_data_map.end(); ++element )
            {
                if ( element->second )
                {
                    // if MapSet has a defined DELETE_EVENT delegate, run it (we run event before deleting element)
                    if (m_del_event_delegate != NULL)
                    {
                        m_del_event_delegate( element->first, m_data_map[element->first].second );
                    }

                    m_data_map.erase( element->first );
                    --m_total_entries;
                }
            }

            // clear the shared map (because we ported any add/delete/modify to master)
            it->second->m_data_map.clear();
            it->second->m_deleted_data_map.clear();
        }

        // register last SYNCH timestamp
        m_last_synch_timestamp = m_virtual_timer_function();

        // update the total number of SYNCH loops done
        ++m_total_synch_loop_done;
    }

    // --------------------------------------------------------------------------------------------------

    template<class KEY_CLASS, class VALUE_CLASS>
    void MasterMap<KEY_CLASS, VALUE_CLASS>::purgeMapset( UInt64 now_time, UInt32 max_age_seconds )
    {
        // max age cannot be 0
        if (max_age_seconds == 0) return;

        // get max_age in UInt64 format
        UInt64 max_age( static_cast<UInt64>(max_age_seconds) << 32 );

        // get a unique WRITE LOCK to MasterMap mutex
        boost::unique_lock<boost::shared_mutex> master_map_write_lock( m_master_map_mutex );

        // PURGE elements from MasterMap
        for ( typename MapType::iterator element = m_data_map.begin(); element != m_data_map.end(); )
        {
            // NB: element->second.first = age of the stored entry
            UInt64 element_time( element->second.first );

            // DEBUG
            //UInt32 now_time_seconds = static_cast<UInt32>( now_time >> 32 );
            //UInt32 element_time_seconds = static_cast<UInt32>( element_time >> 32 );
            //UInt32 element_age_seconds = now_time_seconds - element_time_seconds;
            // END DEBUG
        
            if ( now_time - element_time > max_age )
            {
                // if MapSet has a defined DELETE_EVENT delegate, run it (we run event before deleting element)
                if (m_del_event_delegate != NULL)
                {
                    m_del_event_delegate( element->first, m_data_map[element->first].second );
                }

                m_data_map.erase( element++ );
                --m_total_entries;
                ++m_total_purged_elements;
            }
            else
            {
                element++;
            }
        }

        // PURGE elements from all SharedMaps
        for ( typename std::unordered_map<UInt64, SharedMapTypePtr>::iterator it = m_per_thread_shared_maps.begin(); it != m_per_thread_shared_maps.end(); ++it )
        {
            // get a unique WRITE LOCK to SharedMap mutex
            boost::unique_lock<boost::shared_mutex> shared_map_write_lock( it->second->m_shared_map_mutex );

            // iterate between elements of this map
            for ( typename MapType::iterator element = it->second->m_data_map.begin(); element != it->second->m_data_map.end(); )
            {
                // NB: element->second.first = age of the stored entry
                UInt64 element_time( element->second.first );

                if ( now_time - element_time > max_age )
                {
                    // NB: it is a SharedMap so we do not decrement/increment m_total_entries and m_total_purged_elements
                    it->second->m_data_map.erase( element++ );
                }
                else
                {
                    element++;
                }
            }
        }

        // register last purge timestamp and max_age
        m_last_purge_timestamp = now_time;
        m_last_purge_max_age_seconds = max_age_seconds;

        // update the total number of PURGE loops done
        ++m_total_purge_loop_done;
    }

    // --------------------------------------------------------------------------------------------------
    //                                       *** SharedMapManager ***
    // --------------------------------------------------------------------------------------------------

    template<class KEY_CLASS, class VALUE_CLASS>
    bool SharedMapManager::createMapset(const std::string& mapset_name, UInt32 sync_interval, UInt32 purge_interval, UInt32 purge_max_age_seconds, fastdelegate::FastDelegate0<UInt64> virtual_timer_function)
    {
        // if no m_virtual_timer_function is given uses the default one
        if ( !virtual_timer_function )
        {
            virtual_timer_function = fastdelegate::FastDelegate0< UInt64>( SharedMapManager::Instance().getPtr(), &SharedMapManager::getVirtualTime );
        }

        // get a unique WRITE LOCK to SharedMapManager mutex
        boost::unique_lock<boost::shared_mutex> write_lock( m_shared_map_manager_mutex );

        // return value
        bool rv(false);

        if ( !m_master_maps.count(mapset_name) )
        {
            // create new MASTER MAP
            std::shared_ptr< MasterMap< KEY_CLASS, VALUE_CLASS > > master_map
                ( new MasterMap< KEY_CLASS, VALUE_CLASS >(sync_interval, purge_interval, purge_max_age_seconds, virtual_timer_function) );

            // insert new MASTER map in the m_master_maps map
            m_master_maps.insert( std::make_pair(mapset_name, std::static_pointer_cast<void>(master_map)) );

            // ok we got it
            rv = true;
        }

        return rv;
    }

    // --------------------------------------------------------------------------------------------------

    template<class KEY_CLASS, class VALUE_CLASS>
    std::shared_ptr< SharedMap<KEY_CLASS, VALUE_CLASS> > SharedMapManager::getSharedMap( const std::string& mapset_name )
    {
        // get a shared READ LOCK to SharedMapManager mutex
        boost::shared_lock<boost::shared_mutex> read_lock( m_shared_map_manager_mutex );

        // before to get a shared_map we MUST create the relative MapSet (so exists relative MASTER map)
        assert( m_master_maps.count(mapset_name) );

        // down cast to master_map
        std::shared_ptr< MasterMap<KEY_CLASS, VALUE_CLASS> > master_map
            ( std::static_pointer_cast< MasterMap<KEY_CLASS, VALUE_CLASS> >(m_master_maps[mapset_name]) );

        
#if defined (DARWIN)
            // CREATE OR RETURN shared_map (asking to master_map to do it)
            std::shared_ptr< SharedMap<KEY_CLASS, VALUE_CLASS> > shared_map
                = master_map->getPerThreadSharedMap( cal::Printable( QAppNG::ThreadCounter::Instance().getThreadId() ) );
#else
            // CREATE OR RETURN shared_map (asking to master_map to do it)
            std::shared_ptr< SharedMap<KEY_CLASS, VALUE_CLASS> > shared_map
                = master_map->getPerThreadSharedMap( QAppNG::ThreadCounter::Instance().getThreadId() );
#endif

        // return the shared pointer
        return shared_map;
    }

    // --------------------------------------------------------------------------------------------------

    template<class KEY_CLASS, class VALUE_CLASS>
    void SharedMapManager::setSetEventDelegate( const std::string& mapset_name, fastdelegate::FastDelegate2< const KEY_CLASS&, VALUE_CLASS&,void > set_event_delegate )
    {
        // get a shared READ LOCK to SharedMapManager mutex
        boost::shared_lock<boost::shared_mutex> read_lock( m_shared_map_manager_mutex );

        if ( m_master_maps.count(mapset_name) )
        {
            // down cast to master_map
            std::shared_ptr< MasterMap<KEY_CLASS, VALUE_CLASS> > master_map
                ( std::static_pointer_cast< MasterMap<KEY_CLASS, VALUE_CLASS> >( m_master_maps[mapset_name] ) );

            // get a shared READ LOCK to MasterMap mutex
            boost::shared_lock<boost::shared_mutex> master_map_read_lock( master_map->m_master_map_mutex );

            // set SET_EVENT_DELEGATE for the given MasterMap
            master_map->m_set_event_delegate = set_event_delegate;
        }
    }

    // --------------------------------------------------------------------------------------------------

    template<class KEY_CLASS, class VALUE_CLASS>
    void SharedMapManager::setDelEventDelegate( const std::string& mapset_name, fastdelegate::FastDelegate2< const KEY_CLASS&, VALUE_CLASS&,void > del_event_delegate )
    {
        // get a shared READ LOCK to SharedMapManager mutex
        boost::shared_lock<boost::shared_mutex> read_lock( m_shared_map_manager_mutex );

        if ( m_master_maps.count(mapset_name) )
        {
            // down cast to master_map
            std::shared_ptr< MasterMap<KEY_CLASS, VALUE_CLASS> > master_map
                ( std::static_pointer_cast< MasterMap<KEY_CLASS, VALUE_CLASS> >( m_master_maps[mapset_name] ) );

            // get a shared READ LOCK to MasterMap mutex
            boost::shared_lock<boost::shared_mutex> master_map_read_lock( master_map->m_master_map_mutex );

            // set SET_EVENT_DELEGATE for the given MasterMap
            master_map->m_del_event_delegate = del_event_delegate;
        }

    }

    // --------------------------------------------------------------------------------------------------

    template<class KEY_CLASS, class VALUE_CLASS>
    void SharedMapManager::setUpdEventDelagate( const std::string& mapset_name, fastdelegate::FastDelegate2< const KEY_CLASS&, VALUE_CLASS&, void > upd_event_delegate )
    {
        // get a shared READ LOCK to SharedMapManager mutex
        boost::shared_lock<boost::shared_mutex> read_lock( m_shared_map_manager_mutex );

        if ( m_master_maps.count(mapset_name) )
        {
            // down cast to master_map
            std::shared_ptr< MasterMap<KEY_CLASS, VALUE_CLASS> > master_map
                ( std::static_pointer_cast< MasterMap<KEY_CLASS, VALUE_CLASS> >( m_master_maps[mapset_name] ) );

            // get a shared READ LOCK to MasterMap mutex
            boost::shared_lock<boost::shared_mutex> master_map_read_lock( master_map->m_master_map_mutex );

            // set SET_EVENT_DELEGATE for the given MasterMap
            master_map->m_upd_event_delegate = upd_event_delegate;
        }
    }

    // --------------------------------------------------------------------------------------------------

}
#endif //Q_SHARED_MAP_MGR_IMPL_H_NG

