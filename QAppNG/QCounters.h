#ifndef __INCLUDE_QCOUNTERS_H_NG__
#define __INCLUDE_QCOUNTERS_H_NG__

/** ===================================================================================================================
* @file    QCounters HEADER FILE
*
* @brief   Simple Class to handle counters. The counters are UInt64
*
* @copyright
*
* @history
* REF#        Who                                                              When          What
* #6493       A. Della Villa, D. Verna, F. Guzzardi                            Oct-2011      new class to hadle counters
* #7671       A. Della Villa, D. Verna, F. Guzzardi                            Sep-2012      removing atomic structures
* #7988       C.Guidoccio                                                      Dec-2012      Ticket #7988 - use QCounters for QoSAppMonitoring
* #8547       C.Guidoccio                                                      Apr-2013      Ticket #8547 - QCounters performance
*
* @endhistory
* ===================================================================================================================
*/

// STL/Boost includes
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <boost/thread/recursive_mutex.hpp> 
#include <boost/thread/mutex.hpp>

// Base class include
#include <QAppNG/QCountersBase.h>

namespace QAppNG
{

// --------------------------------------------------------------------------------------------------------------------
// class to handle counters
template<typename COUNTERS_ENUMERATOR, COUNTERS_ENUMERATOR enum_max_value> //pre #7988 version: ,typename COUNTER_NUMERIC_CLASS = UInt64>
class QCounters : public QCountersBase 
{
// ----------------------------------------------------------------------------
public:

    //-------------------------------------------------------------------------
    // constructor
    // 
    // --- params 
    //   module: module that handles the counters, it is used like a namespace
    //   tid: thread id of counters handler
    //   
    QCounters(const char* module=DEFAULT_SECTION_NAME, UInt64 tid=MAX_COUNTER_SIZE) 
        : QCountersBase(enumToSize_t(enum_max_value), module, QAppNG::QStatusManager::instance().getExecutableName(), tid)
    {
    }

    //-------------------------------------------------------------------------
    // destructor
    ~QCounters()
    {
        /* FIXME[cg]: rivedere
        if ( m_raw_pointers_to_per_thread_local_storage.size() == 0 )
        {
            return;
        }

        std::vector< QCountersDataStorageBase* >::iterator raw_iter = m_raw_pointers_to_per_thread_local_storage.begin();

        while( raw_iter != m_raw_pointers_to_per_thread_local_storage.end() )
        {
            if( (*raw_iter)->getThreadId() == m_tid )
            {
                raw_iter = m_raw_pointers_to_per_thread_local_storage.erase(raw_iter);
            }
            else
            {
                raw_iter++;
            }
        }
        */

        /*
        if ( m_counters != NULL && m_raw_pointers_to_per_thread_local_storage.size() > 0 )
        {
            m_raw_pointers_to_per_thread_local_storage.operator[](m_counters->getQCounterThread()) = NULL;
            //m_counters->m_raw_pointers_to_per_thread_local_storage_ref.operator[](m_counters->m_q_counter_thread) = NULL;
        }
        */
    }

    //-------------------------------------------------------------------------
    // #7988 unhide base class implementation
    using QCountersBase::setCounterData;
    using QCountersBase::getCounterTitle;
    using QCountersBase::getCounterValue;
    using QCountersBase::setDBWriteFlag;

    //-------------------------------------------------------------------------
    // convert enum to index...
    inline size_t enumToSize_t( COUNTERS_ENUMERATOR counter_enum_val ) const { return static_cast<size_t>(counter_enum_val); }

    //-------------------------------------------------------------------------
    // set COUNTER_DATA fields for a counter
    //
    // --- params 
    //   counter_enum_val: index of the counter 
    //   string_id: counter name for writes in database
    //   counter_title: counter brief description used in status file
    //   write_to_db: enables writes to database
    //   section: the section name in which the counter will be written in status file
    //   write_to_file: enables writes to status file
    //   
    void setCounterData( COUNTERS_ENUMERATOR counter_enum_val, const char* string_id, const char* counter_title , bool write_to_db, const char* section = DEFAULT_SECTION_NAME ,bool write_to_file=true)
    {
        setCounterData(enumToSize_t(counter_enum_val),string_id,counter_title,write_to_db, section, write_to_file);
    }

    //-------------------------------------------------------------------------
    inline void setDBWriteFlag(COUNTERS_ENUMERATOR counter_enum_val, bool flagValue) const
    {
        setDBWriteFlag( enumToSize_t(counter_enum_val), flagValue);
    }

    //-------------------------------------------------------------------------
    inline void incrementCounter(COUNTERS_ENUMERATOR counter_enum_val)
    {
        incrementCounter( enumToSize_t(counter_enum_val) );
    }

    inline void incrementCounter(COUNTERS_ENUMERATOR counter_enum_val, UInt64 val)
    {
        incrementCounter(enumToSize_t(counter_enum_val), val);
    }

    //-------------------------------------------------------------------------
    inline void decrementCounter(COUNTERS_ENUMERATOR counter_enum_val)
    {
        decrementCounter(enumToSize_t(counter_enum_val));
    }

    inline void decrementCounter(COUNTERS_ENUMERATOR counter_enum_val, UInt64 val)
    {
        decrementCounter(enumToSize_t(counter_enum_val),val);
    }

    //-------------------------------------------------------------------------
    inline void setCounterValue(COUNTERS_ENUMERATOR counter_enum_val, UInt64 val)
    {
        setCounterValue(enumToSize_t(counter_enum_val), val);
    }

    //-------------------------------------------------------------------------
    const std::string& getCounterTitle( COUNTERS_ENUMERATOR counter_enum_name ) const
    {
        return m_counterData[ enumToSize_t(counter_enum_name) ].title; 
    }

    //-------------------------------------------------------------------------
    inline UInt64 getCounterValue(COUNTERS_ENUMERATOR counter_enum_name) const
    {
        return getCounterValue(enumToSize_t(counter_enum_name));
    }

    //-------------------------------------------------------------------------
    std::string counterToString(COUNTERS_ENUMERATOR counter_enum_name) const
    {
        return counterToString(enumToSize_t(counter_enum_name));
    }

    //------------------------------------------------------------------------------------------------------------------------------------------------------------------------

    inline void setCounterModule ( const char * module_name )
    {
        m_module = std::string(module_name);
    }

/*
    //-------------------------------------------------------------------------
    // set COUNTER_DATA fields for a counter
    //
    // --- params 
    //   counter_index: index of the counter 
    //   string_id: counter name for writes in database
    //   counter_title: counter brief description used in status file
    //   write_to_db: enables writes to database
    //   section: the section name in which the counter will be written in status file
    //   write_to_file: enables writes to status file
    //   
    void setCounterData( size_t counter_index, const char* string_id, const char* counter_title , bool write_to_db, const char* section = DEFAULT_SECTION_NAME ,bool write_to_file=true)
    {
        // get UNIQUE LOCK
        boost::unique_lock< boost::recursive_mutex > uniqueLock( m_recursive_mutex );

        m_counterData[ counter_index ].strId = string_id;
        m_counterData[ counter_index ].title = counter_title;
        m_counterData[ counter_index ].writeToDB = write_to_db;
        m_counterData[ counter_index ].writeToFile = write_to_file;

        size_t titleLen = strlen(counter_title);
        if( m_titleMaxLen < titleLen )
        {
            m_titleMaxLen = titleLen;
        }

        // insert counter in section ...
        m_sections.insert(std::pair<std::string, size_t>(section, counter_index));
    }
    //-------------------------------------------------------------------------
    const std::string& getCounterStringId( size_t counter_index)
    {
        // get UNIQUE LOCK
        boost::unique_lock< boost::recursive_mutex > uniqueLock( m_recursive_mutex );

        return m_counterData[ counter_index ].strId;
    }

    //-------------------------------------------------------------------------
    const std::string& getCounterTitle( size_t counter_index)
    {
        // get UNIQUE LOCK
        boost::unique_lock< boost::recursive_mutex > uniqueLock( m_recursive_mutex );

        return m_counterData[ counter_index ].title;
    }
    //-------------------------------------------------------------------------
    inline UInt64 getCounterValue(size_t counter_index) const
    {
        UInt64 counter_value(0);

        for (size_t i = 0; i < m_raw_pointers_to_per_thread_local_storage.size(); i++)
        {
            if ( m_raw_pointers_to_per_thread_local_storage[i] != NULL )
            {
                counter_value += m_raw_pointers_to_per_thread_local_storage[i]->getCounter(counter_index);
            }
        }

        return counter_value;
    }
    */
    //-------------------------------------------------------------------------
    inline void incrementCounter(size_t counter_index)
    {
        ++getPerThreadCounter(counter_index);
    }

    inline void incrementCounter(size_t counter_index, UInt64 val)
    {
        // check value 
        if( val == 0) return;
        getPerThreadCounter( counter_index ) += val;
    }

    //-------------------------------------------------------------------------
    inline void decrementCounter(size_t counter_index)
    {
        UInt64& counter = getPerThreadCounter(counter_index);
        assert(counter>0);
        
        --counter;
    }

    inline void decrementCounter(size_t counter_index, UInt64 val)
    {
        // check value
        if( val == 0) return;

        /*
        // update counter
        getPerThreadCounter( counter_index ) -= val;
        */

        UInt64& counter = getPerThreadCounter(counter_index);
        assert(counter >= val);

        counter -= val;
    }

    //-------------------------------------------------------------------------
    inline void setCounterValue(size_t counter_index, UInt64 val)
    {
        getPerThreadCounter( counter_index ) = val;
    }

/*
    //-------------------------------------------------------------------------
    inline void resetCounterValue(size_t counter_index) const
    {
        for (size_t i = 0; i < m_raw_pointers_to_per_thread_local_storage.size(); i++)
        {
            if ( m_raw_pointers_to_per_thread_local_storage[i] != NULL )
            {
                m_raw_pointers_to_per_thread_local_storage[i]->resetCounter(counter_index);
            }
        }
    }
*/

    //-------------------------------------------------------------------------
    // functions to get a formatted counter representation 
    // ready to be written on status file
    std::string getFormattedCountersWithTitle( const std::string& tagid ) const
    {
        std::stringstream titleline (std::stringstream::out | std::stringstream::in );
        std::stringstream valuesline (std::stringstream::out | std::stringstream::in);
        long cur_titleline_pos=0;
        long cur_valuesline_pos=0;
        long spacer = 0;

        std::string stringspace = STATS_MAX_SPACER;

        titleline << tagid << STATS_FORMAT_SPACER;
        valuesline << tagid << STATS_FORMAT_SPACER;

        for (std::size_t i = 0; i < m_number_of_counters;  i++)
        {
            spacer = 0;
            titleline << "#" << m_counterData[i].title << STATS_FORMAT_SPACER;
            cur_titleline_pos = (long) titleline.tellp();

            valuesline << "#" <<  getCounterValue(i) << STATS_FORMAT_SPACER;

            cur_valuesline_pos = (long) valuesline.tellp();

            if (cur_valuesline_pos > cur_titleline_pos)
            {
                spacer = cur_valuesline_pos - cur_titleline_pos;
                titleline << stringspace.substr(0,spacer);
            }
            else
            {
                spacer = cur_titleline_pos - cur_valuesline_pos;
                valuesline << stringspace.substr(0,spacer);
            }
        }
        return titleline.str() + "\n" + valuesline.str() + "\n";  // std::endl make stream flush...
    }

    std::string getFormattedCounters( const std::string& tagid ) const
    {
        std::stringstream valuesline (std::stringstream::out | std::stringstream::in);

        valuesline << tagid << STATS_FORMAT_SPACER;

        for (std::size_t i = 0; i < m_number_of_counters;  i++)
        {
            valuesline << "#" <<  getCounterValue(i) << STATS_FORMAT_SPACER;
        }
        return valuesline.str() + "\n";  // std::endl make stream flush...
    }

    //-------------------------------------------------------------------------
    // returns a formatted string containing title and justified counter value 
    std::string counterToString(size_t counter_index)
    {
        std::stringstream counterString (std::stringstream::out | std::stringstream::in);
        counterString << std::left << std::setw(m_titleMaxLen) << getCounterTitle(counter_index) << " = " << getCounterValue(counter_index);
        return counterString.str();
    }

    //-------------------------------------------------------------------------
    // returns the SQL call to stored procedure that creates the tables and trigger
    // for current host an application name
    std::string getIdentificationDataSQL() const
    {
        std::stringstream dbString (std::stringstream::out | std::stringstream::in);
        dbString << "SELECT add_site_app('" << m_hostname << "','" << m_appname << "');";
        return dbString.str();
    }

    //-------------------------------------------------------------------------
    // returns a SQL command to save counter value to database
    std::string counterToDB(size_t counter_index,std::string& statusCurrentTime) 
    {
        std::stringstream dbString (std::stringstream::out | std::stringstream::in);
        dbString << "INSERT INTO cnt_" << m_hostname << "_" << m_appname << 
            " VALUES('" << statusCurrentTime << "','" << getCounterStringId(counter_index) << "','" << getCounterValue(counter_index) << "','" << m_module << "','" << m_tid << "');";
        return dbString.str();
    }

    //-------------------------------------------------------------------------
    // #9236
    // returns a SQL command to save an external value as a counter to database
    // others methods can be added to handle float and string values
    std::string externalCounterToDB(COUNTERS_ENUMERATOR counter_enum_name,std::string& statusCurrentTime, UInt64 the_value) 
    {
        size_t counter_index = enumToSize_t(counter_enum_name);
        std::stringstream dbString (std::stringstream::out | std::stringstream::in);
        dbString << "INSERT INTO cnt_" << m_hostname << "_" << m_appname << 
            " VALUES('" << statusCurrentTime << "','" << getCounterStringId(counter_index) << "','" << the_value << "','" << m_module << "','" << m_tid << "');";
        return dbString.str();
    }

    //-------------------------------------------------------------------------
    // format external data justifying them like the counters data
    std::string formatAsCounter(const char* aTitle,const std::string aValue) const
    {
        std::stringstream asCounterString (std::stringstream::out | std::stringstream::in);
        asCounterString << std::left << std::setw(m_titleMaxLen) << aTitle << " = " << aValue << "\n";  // std::endl make stream flush...
        return asCounterString.str();
    }

    std::string formatAsCounter(const char* aTitle,const UInt64 aValue) const
    {
        std::stringstream asCounterString (std::stringstream::out | std::stringstream::in);
        asCounterString << std::left << std::setw(m_titleMaxLen) << aTitle << " = " << aValue << "\n";  // std::endl make stream flush...
        return asCounterString.str();
    }

    //-------------------------------------------------------------------------
    // return a list of SQL commands for all counters enabled to be written to database 
    /*
    std::list<std::string> getSQLCommands(std::string& statusCurrentTime)
    {
        std::list<std::string> dbCmdList;
        for ( size_t k = 0; k < m_number_of_counters;k++)
        {
            if(isDBWriteEnabled(k))
            {
                dbCmdList.push_back(counterToDB(k,statusCurrentTime));
            }
        }
        return dbCmdList;
    }
    */

    //-------------------------------------------------------------------------
    // returns a multiline string containing all counters enabled to be written
    // in status file organized in sections
    std::string dump()
    {
        std::stringstream dumpString (std::stringstream::out | std::stringstream::in);
        bool sectionHeaderPrinted = false;
        std::multimap<std::string, size_t>::const_iterator masterIt, slaveIt;

        if( m_sections.size() == 0 )
        {
            dumpString << "<empty section map>\n";
            return dumpString.str();
        }

        for (masterIt = m_sections.begin();  masterIt != m_sections.end();  masterIt = slaveIt)
        {
            std::string theSection = (*masterIt).first;
            std::pair<std::multimap<std::string, size_t>::const_iterator, std::multimap<std::string, size_t>::const_iterator> counterIndexIter = m_sections.equal_range(theSection);

            // Iterate over all map elements with key == theSection
            sectionHeaderPrinted = false;
            for (slaveIt = counterIndexIter.first;  slaveIt != counterIndexIter.second;  ++slaveIt)
            {
                UInt64 theCounterIndex = (*slaveIt).second;
                if(theCounterIndex >= MAX_COUNTER_SIZE)
                {
                    continue;
                }
                if(isFileWriteEnabled((size_t)theCounterIndex))
                {
                    if( !sectionHeaderPrinted && theSection != DEFAULT_SECTION_NAME )
                    {
                        sectionHeaderPrinted = true;
                        dumpString << "\n ------------------------ \n";  // std::endl make stream flush...
                        dumpString << " --- " << theSection << "\n";  // std::endl make stream flush...
                        dumpString << " ------------------------ \n";  // std::endl make stream flush...
                    }
                    dumpString << counterToString((size_t)theCounterIndex) << "\n";  // std::endl make stream flush...
                }
            }
        }
        return dumpString.str();
    }

// -------------------------------------------------------------------------------------------------------------------------------
// private methods - private methods - private methods - private methods - private methods - private methods - private methods - 
// -------------------------------------------------------------------------------------------------------------------------------
private:
    // ------------------------------------------------------------------------
    // return the per thread value of counter
    inline UInt64& getPerThreadCounter( COUNTERS_ENUMERATOR counter_enum_name ) 
    {
        return getPerThreadCounter( enumToSize_t( counter_enum_name ));
    }

    inline UInt64& getPerThreadCounter(size_t counter_index )
    {
        initDataStorage();
        return m_counters->getCounter(counter_index);
    }

    // ------------------------------------------------------------------------
    // init counters data storage                                     #8645
    inline void initDataStorage() 
    {
        if ( m_counters != NULL )
        {
            return;
        }
        // get UNIQUE LOCK: we need it because we are going to modify m_raw_pointers_to_per_thread_local_storage
        boost::unique_lock< boost::recursive_mutex > uniqueLock( m_recursive_mutex );
        if ( m_counters == NULL )
        {
            m_counters = new QAppNG::QCountersDataStorageBase(m_number_of_counters, static_cast<size_t>(m_tid)) ;

            // add raw pointer of current thread to vector
            m_raw_pointers_to_per_thread_local_storage.push_back( m_counters );
        }
    }

    // ------------------------------------------------------------------------
    //  private members - private members - private members - private members - 
    // ------------------------------------------------------------------------

#ifdef WIN32
    static __declspec(thread) QCountersDataStorageBase* m_counters;
#else
    static __thread           QCountersDataStorageBase* m_counters;
#endif
};

// --------------------------------------------------------------------------------------------------------------------
// static members definition for QCountersBase
// --------------------------------------------------------------------------------------------------------------------
#ifdef WIN32
    template<typename COUNTERS_ENUMERATOR, COUNTERS_ENUMERATOR enum_max_value>
    __declspec(thread) QCountersDataStorageBase* QCounters< COUNTERS_ENUMERATOR,enum_max_value >::m_counters = NULL;
#else
    template<typename COUNTERS_ENUMERATOR, COUNTERS_ENUMERATOR enum_max_value>
    __thread           QCountersDataStorageBase* QCounters< COUNTERS_ENUMERATOR,enum_max_value >::m_counters = NULL;
#endif

// --------------------------------------------------------------------------------------------------------------------
}       // end namespace
// --------------------------------------------------------------------------------------------------------------------
#endif // __INCLUDE_QCOUNTERS_H_NG__
// --------------------------------------------------------------------------------------------------------------------
// End of file
// --------------------------------------------------------------------------------------------------------------------