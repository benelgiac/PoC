#ifndef QSHARE_KEYVALUE_COUNTERS
#define QSHARE_KEYVALUE_COUNTERS
/** ===================================================================================================================
  * @file    QSharedKeyValueCounter HEADER FILE
  *
  * @brief   Template class to handle quasi-lock-free threadsafe key-value storage
  *
  * @copyright
  *
  * @history
  * REF#        Who                                                              When          What
  * #8770       A. Della Villa, F.Lasagni                                        Jun-2013      Original development
  *
  * @endhistory
  * ===================================================================================================================
  */

#include <string>
#include <vector>
#include <unordered_map>
#include <type_traits>

#include <algorithm>
#include <iomanip>

#include <memory>
#include <boost/thread/mutex.hpp>

#include "QAppNG/Singleton.h"

// --------------------------------------------------------------------------------------------------------------------

#define MAX_ROWS_IN_MATRIX 5000

// --------------------------------------------------------------------------------------------------------------------
// Utility Functions to print a formatted dump of QSharedKeyValueCounters instance

// Returns a 24 characters long, left-aligned, blank string
// It should be specialized with the key type, if key name is needed in the dump Header
template< typename KEY_CLASS >
inline std::string getFormattedKeyName()
{
    static std::ostringstream out;
    static bool key_name_string_initilized(false);

    if (!key_name_string_initilized )
    {
        out << std::setfill(' ');
        out << std::setiosflags(std::ios::left);

        out << std::setw(24) << " ";

        key_name_string_initilized = true;
    }

    return out.str();
}

// Returns an empty string
// It should be specialized with the counter type, if Counter Name is needed in the dump Header
template< typename VALUE_CLASS >
inline std::string getFormattedCounterName()
{
    return std::string();
}

// Returns a 24 characters long, left-aligned string containing key value.
// It must be specialized if key type is not integral
template< typename KEY_CLASS >
inline std::string getFormattedKeyValue(const KEY_CLASS& key)
{
    static_assert(std::is_integral<KEY_CLASS>::value, " Specialization of template< typename KEY_CLASS > std::string getFormattedKeyValue() must be provided for not integral type");

    std::ostringstream out;
    
    out << std::setfill(' ');
    out << std::setiosflags(std::ios::left);

    out << std::setw(24) << key;

    return out.str();
}

// Returns a 24 characters long, left-aligned string containing counter value.
// It must be specialized if counter type is not integral
template< typename VALUE_CLASS >
inline std::string getFormattedCounterValue(const VALUE_CLASS& value)
{
    static_assert(std::is_integral<VALUE_CLASS>::value, " Specialization of template< typename VALUE_CLASS > std::string getFormattedCounterValue() must be provided for not integral type");

    static bool value_is_integral(std::is_integral<VALUE_CLASS>::value);

    std::ostringstream out;

    out << std::setfill(' ');
    out << std::setiosflags(std::ios::left);

    out << std::setw(24) << value;

    return out.str();
}

// --------------------------------------------------------------------------------------------------------------------

namespace QAppNG
{

    template< class KEY_CLASS, class VALUE_CLASS >
    class QSharedKeyValueCounters : public  QAppNG::Singleton< QSharedKeyValueCounters< KEY_CLASS,VALUE_CLASS > >
    {

        // declare friend classes
        friend class  QAppNG::Singleton< QSharedKeyValueCounters< KEY_CLASS,VALUE_CLASS > >;

        public:
            //Dtor
            ~QSharedKeyValueCounters()
            {}

            // Register subscriber and return a shared pointer to its specific data structure
            size_t registerSubscriber( void* i_subscriber_id)
            {
                //Acquire the lock
                boost::unique_lock<boost::recursive_mutex> lock(m_mutex);

                size_t a_current_index = m_subscribers_number;

                //Cast identifier to uint64
                UInt64 a_numeric_id = reinterpret_cast< UInt64 >( i_subscriber_id );
            
                //Assign an index to the subscriber and fill the corresponding entry into m_subscriber_index_map 
                m_subscriber_id_index_map[a_numeric_id] = a_current_index;

                //Allocate new column in master matrix
                for (size_t index = 0 ; index < m_element_number; ++index)
                {
                    m_master_value_matrix[index].push_back( std::shared_ptr< VALUE_CLASS >( new VALUE_CLASS() ) );
                }

                //Allocate subscriber-specific map
                m_subscriber_specific_cache_maps.push_back( std::shared_ptr< DataKeyIndexMapType >( new DataKeyIndexMapType() ) );

                ++m_subscribers_number;

                return a_current_index;
            }

            // Wrapper around getOrInitCounter for subscribers unaware of self-index
            std::shared_ptr<VALUE_CLASS> getOrInitCounterBySubscriberID( const KEY_CLASS& i_key, const void* i_subscriber_id )
            {
                //Convert subscriber ID to its internal index
                std::unordered_map< UInt64 , UInt8 >::const_iterator id_iter = m_subscriber_id_index_map.find(reinterpret_cast< UInt64 >(i_subscriber_id));
                if( id_iter == m_subscriber_id_index_map.end() )
                {
                    //UNKNOWN SUBSCRIBER
                    return std::shared_ptr<VALUE_CLASS>();
                }
                UInt64 a_subscriber_index = id_iter->second;

                return getOrInitCounterByIndex( i_key, a_subscriber_index);
            }

            // Gets the counter corresponding to the given key for the subscriber identified by i_subscriber_index if already defined or defines it
            std::shared_ptr<VALUE_CLASS> getOrInitCounterByIndex( const KEY_CLASS& i_key, const size_t i_subscriber_index )
            {
                //Initialize index to that of next element
                size_t a_data_index = m_element_number;

                // Get the pointer to the subscriber owned map
                std::shared_ptr< DataKeyIndexMapType > a_subscriber_owned_map_ptr = m_subscriber_specific_cache_maps[i_subscriber_index];

                // Let's check if there's an entry for the given key in the subscribers specific map
                if( a_subscriber_owned_map_ptr->count(i_key) )
                {
                    /**************************
                             CACHE HIT!
                    ***************************/
                    a_data_index = a_subscriber_owned_map_ptr->at(i_key);
                }
                else
                {
                    /**************************
                             CACHE MISS!
                    ***************************/
                    //Acquire the lock to synchronize on master map 
                    boost::unique_lock<boost::recursive_mutex> lock(m_mutex);

                    //Check in master map
                    if( m_master_key_map.count(i_key) )
                    {
                        /**************************
                               MASTER MAP HIT!
                        ***************************/
                        //Update index
                        a_data_index = m_master_key_map[i_key];
                    }
                    else
                    {
                        /***************************
                        MASTER MAP MISS - UNKNOWN KEY
                        ***************************/
                        setSingleCounter(i_key);
                    }

                    //Let's synchronize subscriber-specific map
                    (*a_subscriber_owned_map_ptr)[i_key] = a_data_index;
                }

                return m_master_value_matrix[a_data_index][i_subscriber_index];
            }

            //Dump the whole master matrix
            std::string dumpAllCounters( )
            {
                std::stringstream str;

                static std::string header_string = header();

                if (!header_string.empty())
                {
                    str << header_string << std::endl;
                }
                
                //Acquire the lock to synchronize on master map 
                boost::unique_lock<boost::recursive_mutex> lock(m_mutex);

                typename DataKeyIndexMapType::iterator key_iter = m_master_key_map.begin();

                // print with some ordering or not
                for ( ; key_iter != m_master_key_map.end() ; ++key_iter )
                {
                    str << getFormattedKeyValue(key_iter->first) << getFormattedCounterValue( getSingleCounter(key_iter->second) ) << std::endl;
                }

                return str.str();
            }

            // Master Getter
            VALUE_CLASS sumUpSparseCounter( const KEY_CLASS& i_key )            
            {
                VALUE_CLASS aCounter;

                size_t a_data_index(0);

                //Acquire the lock to synchronize on master map 
                boost::unique_lock<boost::recursive_mutex> lock(m_mutex);                

                if( m_master_key_map.count(i_key) )
                {
                    a_data_index = m_master_key_map[i_key];

                    aCounter = getSingleCounter(a_data_index);
                }

                return aCounter;
            }

            //Dump the whole master matrix
            std::string dumpAllCountersSorted()
            {
                std::stringstream               str;

                static std::string header_string = header();

                if (!header_string.empty())
                {
                    str << header_string << std::endl;
                }

                //Acquire the lock to synchronize on master map 
                boost::unique_lock<boost::recursive_mutex> lock(m_mutex);

                typename DataKeyIndexMapType::iterator key_iter = m_master_key_map.begin();

                // print with some ordering or not by VALUE_CLASS
                std::vector< std::pair< VALUE_CLASS, KEY_CLASS > > sortable_values;

                for ( ; key_iter != m_master_key_map.end() ; ++key_iter )
                {
                    sortable_values.push_back( std::make_pair( getSingleCounter( key_iter->second ), key_iter->first ) );
                }

                std::sort( sortable_values.begin(), sortable_values.end() );

                for ( size_t i = 0 ; i < sortable_values.size() ; ++i )
                {
                    str << getFormattedKeyValue(sortable_values[i].second) << getFormattedCounterValue( sortable_values[i].first ) << std::endl;
                }

                return str.str();
            }
        private:
            //Typedefs
            typedef std::vector < std::shared_ptr< VALUE_CLASS > >                      MasterVectorType;
            typedef std::unordered_map< KEY_CLASS , UInt16 >                              DataKeyIndexMapType;

            //Members
            boost::recursive_mutex                                                        m_mutex;
            size_t                                                                        m_element_number;
            size_t                                                                        m_subscribers_number;

            //Subscribers-id / Subscribers internal-index map
            std::unordered_map< UInt64 , UInt8 >                                        m_subscriber_id_index_map;
            //Master data container
            std::vector < MasterVectorType >                                              m_master_value_matrix;
            //Master datakey/ index map
            DataKeyIndexMapType                                                           m_master_key_map;
            //Subscriber-specific datakey/ index map
            std::vector < std::shared_ptr< DataKeyIndexMapType > >                      m_subscriber_specific_cache_maps;

            //Private Methods

            //Private Ctors and assignment operator
            QSharedKeyValueCounters()
                : m_subscribers_number(0)
                , m_element_number(0)
            {
                m_master_value_matrix.reserve(MAX_ROWS_IN_MATRIX);
            }

            QSharedKeyValueCounters(const QSharedKeyValueCounters& iCopy)
            {}

            QSharedKeyValueCounters& operator=( const QSharedKeyValueCounters& iAssignedObject )
            {}

            // Master Setter
            void setSingleCounter( const KEY_CLASS& i_key )
            {
                //Create a new row in the master matrix
                MasterVectorType a_row_vector(m_subscribers_number);
                for ( size_t a_column = 0; a_column < m_subscribers_number; ++a_column)
                {
                    //Initialize a column for each subscriber
                    a_row_vector[a_column] = std::shared_ptr< VALUE_CLASS >(new VALUE_CLASS());
                }

                //Acquire the lock to synchronize on master map 
                boost::unique_lock<boost::recursive_mutex> lock(m_mutex);

                //Append the new row to the master matrix
                m_master_value_matrix.push_back(a_row_vector);

                //Update master key-index map
                m_master_key_map[i_key] = m_element_number;

                //Increase counter
                ++m_element_number;
            }

            // Master Getter
            VALUE_CLASS getSingleCounter( const UInt16 i_key_index )
            {
                VALUE_CLASS aCounter;

                for ( size_t subscriber = 0; subscriber < m_subscribers_number; ++subscriber )
                {
                    aCounter += *m_master_value_matrix[i_key_index][subscriber];
                }

                return aCounter;
            }

            std::string header()
            {
                std::ostringstream result;
                result.clear();
                result.str("");

                result << getFormattedKeyName<KEY_CLASS>() << getFormattedCounterName<VALUE_CLASS>();

                return result.str();
            }

    }; //QSharedKeyValueCounters

} //QAppNG
#endif //QSHARE_KEYVALUE_COUNTERS
// --------------------------------------------------------------------------------------------------------------------
// End of file
// --------------------------------------------------------------------------------------------------------------------



