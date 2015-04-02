#pragma once
/** ===================================================================================================================
  * @file    MultiKeyMap HEADER FILE
  *
  * @brief   easy-to-use multi key map. To use it you have to inherit from MultiKeyMap::Value
  *          and MultiKeyMap::MultiKeyMap<number_of_indexes, value_class>, for instance:
  *
  *              class MyValue : public MultiKeyMap::Value
  *              {
  *              public:
  *                  MyValue() : m_my_data(0) {};
  *
  *                  UInt32 m_my_data;
  *              };
  *
  *              class MyMultiKeyMap : public MultiKeyMap::MultiKeyMap<2, std::shared_ptr<MyValue> >
  *              {
  *              public:
  *                  // insert element
  *                  void insertElementByTlli( const UInt32& tlli, std::shared_ptr<MyValue>& val, const UInt32& current_time_sec = 0 )
  *                      { insertElementByKey<0, UInt32>( tlli, val, current_time_sec ); }
  *                  void insertElementByTlliRai( const UInt64& tlli_rai, std::shared_ptr<MyValue>& val, const UInt32& current_time_sec = 0 )
  *                      { insertElementByKey<1, UInt64>( tlli_rai, val, current_time_sec ); }
  *
  *                  // get element
  *                  bool getGbContextIdByTlli( const UInt32& tlli, std::shared_ptr<MyValue>& val )        { return getElementByKey<0, UInt32>( tlli, val ); }
  *                  bool getGbContextIdByTlliRai( const UInt64& tlli_rai, std::shared_ptr<MyValue>& val ) { return getElementByKey<1, UInt64>( tlli_rai, val ); }
  *
  *                  // set key
  *                  void setTlli( const UInt32& tlli, std::shared_ptr<MyValue>& val )        { setKey<0, UInt32>(tlli, val); }
  *                  void setTlliRai( const UInt64& tlli_rai, std::shared_ptr<MyValue>& val ) { setKey<1, UInt64>(tlli_rai, val); }
  *
  *                  // rem key
  *                  void remTlli( const UInt32& tlli, std::shared_ptr<MyValue>& val )        { remKey<0, UInt32>(tlli, val); }
  *                  void remTlliRai( const UInt64& tlli_rai, std::shared_ptr<MyValue>& val ) { remKey<1, UInt64>(tlli_rai, val); }
  *
  *                  // erase element
  *                  void eraseElement( std::shared_ptr<MyValue>& val )
  *                  {
  *                      remKeys<0, UInt32>( val );
  *                      remKeys<1, UInt64>( val );
  *                  }
  *
  *                  // purge old elements
  *                  void purge( const UInt32& current_time_sec, const UInt32& max_inactivity_time_sec )
  *                  {
  *                      purgeIndexingMap<0, UInt32>( current_time_sec, max_inactivity_time_sec );
  *                      purgeIndexingMap<1, UInt64>( current_time_sec, max_inactivity_time_sec );
  *                  }
  *              };
  *
  * @copyright
  *
  * @history
  * REF#        Who                                             When          What
  * #9510       Alessandro Della Villa, Francesco Guzzardi      Nov-2013      Original Development
  *
  * @endhistory
  * ===================================================================================================================
  */
  
// include
#include <tuple>
#include <unordered_map>
#include <memory>
#include <atomic>

#include <QAppNG/QLogManager.h>
// --------------------------------------------------------------------------------------------------------------------

namespace QAppNG
{
namespace MultiKeyMap
{
    // --------------------------------------------------------------------------------------------------------------------

    // forward declarations
    template <size_t NUMBER_OF_INDEXES, typename VALUE_CLASS>
    class MultiKeyMap;

    // --------------------------------------------------------------------------------------------------------------------

    class Value
    {
        //friend class MultiKeyMap<NUMBER_OF_INDEXES, VALUE_CLASS>;
        template<size_t NUMBER_OF_INDEXES, typename VALUE_CLASS>
        friend class MultiKeyMap;

    public:
        // CTOR
        Value() : m_value_logically_deleted(false), m_last_activity_time_sec(0), m_last_activity_time_nsec(0) 
        {

            //Global counter (moved here from MultiKeyMap class to ensure every object Value has a valid unique id)
            //WARNING: it' static, so multiple different instances of MultiKeyMap will share the same counter
            static std::atomic_size_t unique_id_counter(0);

            //Assign a unique ID to the current object
            m_unique_id = unique_id_counter++;
        }

        // DTOR
        virtual ~Value() {}

        size_t getUniqueId() { return m_unique_id; }

        void touch( const UInt32& sec, const UInt32& nsec ) { m_last_activity_time_sec = sec; m_last_activity_time_nsec = nsec; };
        const UInt32& getLastActivityTimeSec() { return m_last_activity_time_sec; }
        const UInt32& getLastActivityTimeNSec() { return m_last_activity_time_nsec; }

    private:

        // generic KEY MATRIX
        std::vector< std::vector<void*> > m_key;

        bool m_value_logically_deleted;

        template< size_t KEY_NUMBER, typename KEY_CLASS >
        void addKey( const KEY_CLASS& key )
        {
            // add new key to key-matrix. It is supposed to have NO REPETITION at this stage

            // allocate, generalize pointer and push_back in vector (that is part of key matrix)
            m_key[KEY_NUMBER].push_back( reinterpret_cast<void*>( new KEY_CLASS(key) ) );
        }

        template< size_t KEY_NUMBER, typename KEY_CLASS >
        void remKey( const KEY_CLASS& key )
        {
            // search and remove key
            for ( auto it = m_key[KEY_NUMBER].begin(); it != m_key[KEY_NUMBER].end(); )
            {
                if ( *reinterpret_cast<KEY_CLASS*>( *it ) == key )
                {
                    KEY_CLASS* specialized_pointer = reinterpret_cast<KEY_CLASS*>( *it );
                    delete specialized_pointer;
                    it = m_key[KEY_NUMBER].erase( it );
                }
                else 
                {
                    ++it;
                }
            }
        }
    protected:
        // check if for a given indexing map at least one Key has been defined
        template< size_t KEY_NUMBER, typename KEY_CLASS >
        bool hasKey()
        {
            return m_key[KEY_NUMBER].size() > 0;
        }

        // check if for a given indexing map a specific Key has been defined
        template< size_t KEY_NUMBER, typename KEY_CLASS >
        bool hasKey( const KEY_CLASS& key )
        {
            for ( size_t i = 0; i < m_key[KEY_NUMBER].size(); i++ )
            {
                if ( *reinterpret_cast<KEY_CLASS*>( m_key[KEY_NUMBER][i] ) == key )
                {
                    return true;
                }
            }
            return false;
        }

        // check if at least a Key for at least a map is still associated to current Value object
        bool hasKeys()
        {
            for ( size_t i = 0; i < m_key.size(); i++ )
            {
                if ( m_key[i].size() ) return true;
            }
            return false;
        }

        template< size_t KEY_NUMBER, typename KEY_CLASS >
        size_t getKeyVector( std::vector<KEY_CLASS>& key_vector_for_this_key_class )
        {
            key_vector_for_this_key_class.clear();

            for ( size_t i = 0; i < m_key[KEY_NUMBER].size(); i++ )
            {
                key_vector_for_this_key_class.push_back( *reinterpret_cast<KEY_CLASS*>( m_key[KEY_NUMBER][i] ) );
            }
            return key_vector_for_this_key_class.size();
        }

    private:
        size_t m_unique_id;
        UInt32 m_last_activity_time_sec;
        UInt32 m_last_activity_time_nsec;
    };

    // --------------------------------------------------------------------------------------------------------------------

    template <size_t NUMBER_OF_INDEXES, typename VALUE_CLASS>
    class MultiKeyMap<NUMBER_OF_INDEXES, std::shared_ptr<VALUE_CLASS> >
    {
    public:
        // CTOR
        MultiKeyMap()
        {
            for (size_t i=0; i<NUMBER_OF_INDEXES; i++)
            {
                m_indexing_map_vector.push_back(NULL);
                m_indexing_map_init_done.push_back(false);
            }
        }

        // DTOR
        virtual ~MultiKeyMap()
        {
            for (size_t i=0; i<NUMBER_OF_INDEXES; i++)
            {
            }
        }

        template < typename KEY_TYPE>
        bool mapValuePerKeySanityCheck( std::shared_ptr<VALUE_CLASS>& val, size_t key_index)
        {
            if ( !val )
            {
                return true;
            }

            if ( !m_global_map.count(val->m_unique_id) && val->hasKeys() )
            {
                QAppNG::QLogManager::Instance().log("CHECK_THREAD_MAP", QAppNG::QLogManager::Priority::ERROR, "MultiKeyMap - Sanity check failed! Element having keys not in global map");
                return false;
            }

            if ( m_global_map.count(val->m_unique_id) && !val->hasKeys() )
            {
                QAppNG::QLogManager::Instance().log("CHECK_THREAD_MAP", QAppNG::QLogManager::Priority::ERROR, "MultiKeyMap - Sanity check failed! Element not having keys in global map");
                return false;
            }

            if ( m_indexing_map_init_done.size() > key_index && m_indexing_map_init_done[key_index] )
            {
                auto indexing_map_ptr = specializePointer<KEY_TYPE>( m_indexing_map_vector[key_index] );

                // search and remove all keys
                for (size_t i = 0; i < val->m_key[key_index].size(); i++)
                {
                    KEY_TYPE* specialized_pointer = reinterpret_cast<KEY_TYPE*>(val->m_key[key_index][i]);

                    if ( !indexing_map_ptr->count(*specialized_pointer) && val->hasKeys() )
                    {
                        QAppNG::QLogManager::Instance().log("CHECK_THREAD_MAP", QAppNG::QLogManager::Priority::ERROR, "MultiKeyMap - Sanity check failed! Element having keys not in indexing map");
                        return false;
                    }

                    if ( indexing_map_ptr->count(*specialized_pointer) && !val->hasKeys() )
                    {
                        QAppNG::QLogManager::Instance().log("CHECK_THREAD_MAP", QAppNG::QLogManager::Priority::ERROR, "MultiKeyMap - Sanity check failed! Element not having keys in indexing map");
                        return false;
                    }

                    if ( indexing_map_ptr->count(*specialized_pointer)
                        && indexing_map_ptr->at(*specialized_pointer) != val->m_unique_id )
                    {
                        QAppNG::QLogManager::Instance().log("CHECK_THREAD_MAP", QAppNG::QLogManager::Priority::ERROR, "MultiKeyMap - Sanity check failed! Element with invalid unique id");
                        return false;
                    }

                    if ( indexing_map_ptr->count(*specialized_pointer)
                        && !m_global_map.count(val->m_unique_id) )
                    {
                        QAppNG::QLogManager::Instance().log("CHECK_THREAD_MAP", QAppNG::QLogManager::Priority::ERROR, "MultiKeyMap - Sanity check failed! Element in indexing map not in global map");
                        return false;
                    }

                    if ( indexing_map_ptr->count(*specialized_pointer)
                        && m_global_map.count(val->m_unique_id) && !m_global_map.at( val->m_unique_id ) )
                    {
                        QAppNG::QLogManager::Instance().log("CHECK_THREAD_MAP", QAppNG::QLogManager::Priority::ERROR, "MultiKeyMap - Sanity check failed! Key pointing to invalid entry in global map");
                        return false;
                    }
                }
            }

            return true;
        }

        template < typename KEY_TYPE, typename... KEY_TYPES >
        bool sanityCheck( std::shared_ptr<VALUE_CLASS>& val, size_t key_index = 0 )
        {
            if ( !val )
            {
                return true;
            }

            if ( key_index == NUMBER_OF_INDEXES - 1  )
            {
                return mapValuePerKeySanityCheck<KEY_TYPE>( val, key_index );
            }

            return mapValuePerKeySanityCheck<KEY_TYPE>( val, key_index )
                    && sanityCheck< KEY_TYPE, KEY_TYPES... >( val, key_index + 1 );
        }


        // insert element
        template< size_t KEY_NUMBER, typename KEY_CLASS >
        void insertElementByKey( const KEY_CLASS& key, std::shared_ptr<VALUE_CLASS>& val, const UInt32& current_time_sec = 0, const UInt32& current_time_nsec = 0 )
        {
            // init INDEX MATRIX in value object
            for ( size_t i = 0; i < NUMBER_OF_INDEXES; i++ )
            {
                val->m_key.push_back( std::vector<void*>() );
            }

            // insert object in global map
            m_global_map[val->m_unique_id] = val;

            // add value object key to indexing map
            setKey< KEY_NUMBER, KEY_CLASS >( key, val );

            // init last activity time
            val->m_last_activity_time_sec = current_time_sec;
            val->m_last_activity_time_nsec = current_time_nsec;
        }

        // get element by KEY
        template< size_t KEY_NUMBER, typename KEY_CLASS >
        bool getElementByKey( const KEY_CLASS& key, std::shared_ptr<VALUE_CLASS>& element_value )
        {
            checkOrInitIndexingMap<KEY_NUMBER, KEY_CLASS>();

            // get pointer to right indexing map
            auto indexing_map_ptr = specializePointer<KEY_CLASS>(m_indexing_map_vector[KEY_NUMBER]);

            if ( (*indexing_map_ptr).count(key) && m_global_map.count( indexing_map_ptr->at(key) ) )
            {
                element_value = m_global_map.at( indexing_map_ptr->at(key) );
                return true;
            }
            else
            {
                return false;
            }
        }

        bool getElementByUniqueId( const size_t& unique_id, std::shared_ptr<VALUE_CLASS>& element_value )
        {
            if ( m_global_map.count(unique_id) )
            {
                element_value = m_global_map.at( unique_id );
                return true;
            }
            else
            {
                return false;
            }
        }

        // set KEY to element
        template< size_t KEY_NUMBER, typename KEY_CLASS >
        void setKey( const KEY_CLASS& key, std::shared_ptr<VALUE_CLASS>& val )
        {
            checkOrInitIndexingMap<KEY_NUMBER, KEY_CLASS>();

            // get pointer to right indexing map
            auto indexing_map_ptr = specializePointer<KEY_CLASS>(m_indexing_map_vector[KEY_NUMBER]);

            // check if val is in global map. No need to check if val id exactly the one in global map
            // since unique_id is unique... so if it is the same vals are the same
            if ( !m_global_map.count( val->m_unique_id ) )
            {
                insertElementByKey< KEY_NUMBER, KEY_CLASS >( key, val );
            }

            // NB: "When the name of a member template specialization appears after . or -> in a postfix-expression,
            //      or after nested-name-specifier in a qualified-id, and the postfix-expression or qualified-id explicitly depends
            //      on a template-parameter (14.6.2), the member template name must be prefixed by the keyword template.
            //      Otherwise the name is assumed to name a non-template."

            if ( val->template hasKey<KEY_NUMBER, KEY_CLASS>( key ) )
            {
                // element already has this key for this indexing map: DO NOTHING
            }
            else if ( !indexing_map_ptr->count(key) )
            {
                // key not used: ASSIGN KEY TO ELEMENT
                val->template addKey<KEY_NUMBER, KEY_CLASS>( key );
                indexing_map_ptr->operator[](key) = val->m_unique_id;
            }
            else if ( indexing_map_ptr->count(key) && m_global_map.count( indexing_map_ptr->at(key) ) )
            {
                // key used by another element: REMOVE KEY FROM OLD ELEMENT (checking if it is to be deleted) AND REASSIGN
                remKey<KEY_NUMBER, KEY_CLASS>( key, m_global_map.at( indexing_map_ptr->at(key) ) );
                val->template addKey<KEY_NUMBER, KEY_CLASS>(key);
                indexing_map_ptr->operator[](key) = val->m_unique_id;
            }
        }

        // check KEY for element
        template< size_t KEY_NUMBER, typename KEY_CLASS >
        bool hasKey( std::shared_ptr<VALUE_CLASS>& val )
        {
            return val->hasKey<KEY_NUMBER, KEY_CLASS>();
        }

        // check specific KEY for element
        template< size_t KEY_NUMBER, typename KEY_CLASS >
        bool hasKey( const KEY_CLASS& key, std::shared_ptr<VALUE_CLASS>& val )
        {
            return val->template hasKey<KEY_NUMBER, KEY_CLASS>( key );
        }

        template< size_t KEY_NUMBER, typename KEY_CLASS >
        bool getKey( KEY_CLASS& key, std::shared_ptr<VALUE_CLASS>& val, size_t key_index )
        {
            if ( val->m_key[KEY_NUMBER].size() > key_index )
            {
                key = *reinterpret_cast<KEY_CLASS*>( val->m_key[KEY_NUMBER][key_index] );
                return true;
            }
            else
            {
                return false;
            }
        }

        template< size_t KEY_NUMBER, typename KEY_CLASS >
        void getKeys( std::vector<KEY_CLASS>& key_vector, std::shared_ptr<VALUE_CLASS>& val )
        {
            val->template getKeyVector<KEY_NUMBER, KEY_CLASS>( key_vector );
        }

        // rem KEY from element
        template< size_t KEY_NUMBER, typename KEY_CLASS >
        void remKey( const KEY_CLASS& key, std::shared_ptr<VALUE_CLASS>& val )
        {
            checkOrInitIndexingMap<KEY_NUMBER, KEY_CLASS>();

            if (!m_global_map.count(val->m_unique_id))
            {
                return;
            }

            // remove key from element
            val->template remKey<KEY_NUMBER, KEY_CLASS>( key );

            // remove key from indexing map
            auto indexing_map_ptr = specializePointer<KEY_CLASS>(m_indexing_map_vector[KEY_NUMBER]);
            indexing_map_ptr->erase( key );

            // check if element has no keys and eventually delete it
            if ( !val->hasKeys() ) m_global_map.erase( val->m_unique_id );
        }

        // rem all KEYs of a given indexing map from element
        template< size_t KEY_NUMBER, typename KEY_CLASS >
        void remKeys( std::shared_ptr<VALUE_CLASS>& val )
        {
            if ( !m_global_map.count(val->m_unique_id) )
            {
                return;
            }
            
            auto indexing_map_ptr = specializePointer<KEY_CLASS>(m_indexing_map_vector[KEY_NUMBER]);

            // search and remove all keys
            for ( size_t i = 0; i < val->m_key[KEY_NUMBER].size(); i++ )
            {
                KEY_CLASS* specialized_pointer = reinterpret_cast<KEY_CLASS*>( val->m_key[KEY_NUMBER][i] );
                
                // remove key from indexing map
                indexing_map_ptr->erase( *specialized_pointer );

                // deallocate key
                delete specialized_pointer;
            }
            val->m_key[KEY_NUMBER].clear();

            // check if element has no keys and eventually delete it
            if ( !val->hasKeys() ) m_global_map.erase( val->m_unique_id );
        }

        // update last_activity_time_sec of given element
        void touch( std::shared_ptr<VALUE_CLASS>& val, const UInt32& current_time_sec )
        {
            val->m_last_activity_time_sec = current_time_sec;
        }

        // purge given indexing map if inactivity time greater than given value
        template< size_t KEY_NUMBER, typename KEY_CLASS >
        size_t purgeIndexingMap( const UInt32& current_time_sec, const UInt32& max_inactivity_time_sec )
        {
            size_t number_of_purged_elements(0);

            for ( auto it = m_global_map.begin(); it != m_global_map.end(); )
            {
                if (  it->second->m_last_activity_time_sec
                   && current_time_sec > it->second->m_last_activity_time_sec
                   && current_time_sec - it->second->m_last_activity_time_sec > max_inactivity_time_sec )
                {
                    auto next = std::next(it);

                    remKeys<KEY_NUMBER, KEY_CLASS>(it->second);
                    ++number_of_purged_elements;

                    it = std::move(next);
                }
                else
                {
                    ++it;
                }
            }

            return number_of_purged_elements;
        }

        const std::unordered_map< size_t, std::shared_ptr<VALUE_CLASS> >& getGlobalMap() const
        {
            return m_global_map;
        }

    private:
        // main data map
        std::unordered_map< size_t, std::shared_ptr<VALUE_CLASS> > m_global_map;
        
        // indexing map
        std::vector<bool> m_indexing_map_init_done;
        std::vector<void*> m_indexing_map_vector;

        // utility functions
        template< typename KEY_CLASS >
        inline std::unordered_map<KEY_CLASS, size_t>* specializePointer( void* raw_pointer )
        {
            return reinterpret_cast< std::unordered_map<KEY_CLASS, size_t>* >( raw_pointer );
        }

        template< typename KEY_CLASS >
        inline void* generalizePointer( std::unordered_map<KEY_CLASS, size_t>* specialized_pointer )
        {
            return reinterpret_cast< void* >( specialized_pointer );
        }

        template< size_t KEY_NUMBER, typename KEY_CLASS >
        inline void checkOrInitIndexingMap()
        {
            if ( !m_indexing_map_init_done[KEY_NUMBER] )
            {
                // INIT indexing map
                std::unordered_map<KEY_CLASS, size_t>* new_indexing_map = new std::unordered_map<KEY_CLASS, size_t>();

                m_indexing_map_vector[KEY_NUMBER] = generalizePointer(new_indexing_map);

                m_indexing_map_init_done[KEY_NUMBER] = true;
            }
        }
    };
} // namespace MultiKeyMap
} // namespace QAppNG

// --------------------------------------------------------------------------------------------------------------------
// End of file

