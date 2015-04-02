#ifndef __INCLUDE_QCOUNTERSBASE_H_NG__
#define __INCLUDE_QCOUNTERSBASE_H_NG__
/** ===================================================================================================================
* @file    QCountersBase HEADER FILE
*
* @brief   Base class for QCounters, a simple Class to handle counters
*
* @copyright
*
* @history
* REF#        Who                                                              When          What
* #7988       C.Guidoccio                                                      Dec-2012      Ticket #7988 - refactor from QCounters
* #8547       C.Guidoccio                                                      Apr-2013      Ticket #8547 - QCounters performance
*
* @endhistory
* ===================================================================================================================
*/

// STL/Boost includes
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>

#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/mutex.hpp>

#include <QAppNG/QStatusManager.h>

// --------------------------------------------------------------------------------------------------------------------
#define STATS_FORMAT_SPACER  "    "
#define STATS_MAX_SPACER     "                                                                                "

// default section, the header for this section is NOT write on status file
#define DEFAULT_SECTION_NAME "DEFSECT"
#define MAX_COUNTER_SIZE     100000

// --------------------------------------------------------------------------------------------------------------------
// FIXME[cg]: 
// class that contains per thread data storage and references to data, not templated
namespace QAppNG
{
    class QCountersDataStorageBase
    {
    public:
        QCountersDataStorageBase(size_t counters_size, size_t q_counter_thread_id )
            : m_thread_id(0)
        {
            m_dataStorage.resize( counters_size,0 );
            m_thread_id = q_counter_thread_id;
        }

        inline UInt64& getCounter(size_t counter_index) 
        {
            return m_dataStorage[counter_index];
        }

        inline void resetCounter(size_t counter_index) 
        {
            m_dataStorage[counter_index] = 0;
        }

        inline UInt64 getThreadId()
        {
            return m_thread_id;
        }

    private:
        std::vector<UInt64> m_dataStorage;
        UInt64              m_thread_id;
    };
}

// --------------------------------------------------------------------------------------------------------------------
// class to handle counters, not templated
namespace QAppNG
{
class QCountersBase
{
// ----------------------------------------------------------------------------
public:

    // contains data to handle correctly counter writes to status file and/or database
    typedef struct counter_data {
        std::string strId;  // counter string id to be used as counter name in DB
        std::string title;  // status file counter title
        bool        writeToFile; // flag to write counter to status file (def.true)
        bool        writeToDB;   // flag to write counter to DB 
        counter_data() : strId(""),title(""),writeToFile(true),writeToDB(false){}
        counter_data(std::string aStrId,std::string aTitle, bool toFile, bool toDB) : strId(aStrId),title(aTitle),writeToFile(toFile),writeToDB(toDB){};
    } COUNTER_DATA;

    //-------------------------------------------------------------------------
    // constructor
    //
    // --- params 
    //   countersNumber: dimension of counter array
    //   module: module that handles the counters, it is used like a namespace 
    //           to ensure uniqueness of counter
    //   appname: real application name, to identify database table 
    //   tid: thread id of counters handler
    //   
    QCountersBase(size_t countersNumber,const char* module,std::string appname, UInt64 tid)
        : m_number_of_counters( countersNumber )
        , m_counterData(m_number_of_counters)
        , m_titleMaxLen(0)
        , m_module(module)
        , m_hostname("unkHost")
        , m_appname(appname)
        , m_tid(tid)
    {
        // init section map with default value
        // max value is used as a flag to not write default section name on status file
        m_sections.insert(std::pair<std::string, size_t>(DEFAULT_SECTION_NAME, MAX_COUNTER_SIZE));

        m_hostname = QAppNG::QStatusManager::instance().getHostName();
    }

    //-------------------------------------------------------------------------
    inline void setCounterModule ( const char * module_name )
    {
        m_module = std::string(module_name);
    }

    //-------------------------------------------------------------------------
    inline std::string getCounterModule ( )
    {
        return m_module;
    }

    //-------------------------------------------------------------------------
    inline size_t getNumberOfCounters() const 
    {
        return m_number_of_counters;
    }

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
    inline const std::string& getCounterStringId( size_t counter_index)
    {
        // get UNIQUE LOCK
        boost::unique_lock< boost::recursive_mutex > uniqueLock( m_recursive_mutex );

        return m_counterData[ counter_index ].strId;
    }

    //-------------------------------------------------------------------------
    inline const std::string& getCounterTitle( size_t counter_index)
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
            if ( m_raw_pointers_to_per_thread_local_storage.at(i) != NULL )
            {
                counter_value += m_raw_pointers_to_per_thread_local_storage.at(i)->getCounter(counter_index);
            }
        }

        return counter_value;
    } 

    //-------------------------------------------------------------------------
    inline void resetCounterValue(size_t counter_index) const
    {
        for (size_t i = 0; i < m_raw_pointers_to_per_thread_local_storage.size(); i++)
        {
            if ( m_raw_pointers_to_per_thread_local_storage.at(i) != NULL )
            {
                m_raw_pointers_to_per_thread_local_storage.at(i)->resetCounter(counter_index);
            }
        }
    }

    //-------------------------------------------------------------------------
    inline bool isFileWriteEnabled(size_t counter_index) const
    {
        return m_counterData[counter_index].writeToFile;
    }

    //-------------------------------------------------------------------------
    inline void setDBWriteFlag(size_t counter_index, bool flagValue)
    {
        m_counterData[counter_index].writeToDB = flagValue;
    }

    //-------------------------------------------------------------------------
    inline bool isDBWriteEnabled(size_t counter_index) const
    {
        return m_counterData[counter_index].writeToDB;
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
    // return a list of SQL commands for all counters enabled to be written to database
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
// ----------------------------------------------------------------------------
protected:
    // store number of counters
    size_t m_number_of_counters;

    // ptr to raw pointer vector
    // FIXME[cg] subst with std::shared_ptr ???
    std::vector< QAppNG::QCountersDataStorageBase* > m_raw_pointers_to_per_thread_local_storage;

    // mutex only used on counter creation and on counter title setting
    boost::recursive_mutex m_recursive_mutex;

    // counter data
    std::vector< COUNTER_DATA > m_counterData;

    // used to format correctly status file
    size_t m_titleMaxLen;

    // map to handle status file sections
    std::multimap<std::string, size_t> m_sections;

    std::string m_module; 
    std::string m_hostname;
    std::string m_appname;

    UInt64      m_tid; 
};

// --------------------------------------------------------------------------------------------------------------------
}       // end namespace

// --------------------------------------------------------------------------------------------------------------------
#endif // __INCLUDE_QCOUNTERSBASE_H_NG__
// --------------------------------------------------------------------------------------------------------------------
// End of file
// --------------------------------------------------------------------------------------------------------------------
