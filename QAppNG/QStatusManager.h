#ifndef QSTATUSMANAGER_EVO_NG
#define QSTATUSMANAGER_EVO_NG
/** ===================================================================================================================
  * @file    QStatusManager HEADER FILE
  *
  * @brief   GsmApp Evolution - code refactoring project
  *          Simple class to handle status
  *
  * @copyright
  *
  * @history
  * REF#        Who                                                              When          What
  * #3062       A. Della Villa, A. Manetti, F. Gragnani, R. Buti, S. Luceri      Feb-2009      Original development
  * #5022       A. Della Villa                                                   Jun-2010      Minor adjustments
  * #5486       A. Della Villa, S. Luceri                                        Sep-2010      Added remove status feature
  * #5774       Stanislav Timinsky                                               Jan-2011      Added stop and 
  *                                                                                            statusDumpForOneItem methods
  * #7988       C.Guidoccio                                                      Dec-2012      Ticket #7988 - QoSAppMonitoring
  * #8547       F.Lasagni, C.Guidoccio                                           Apr-2013      removed use of legacy version
  *
  * @endhistory
  * ===================================================================================================================
  */

#include <list>
#include <string>
#include <sstream>
#include <map>
#include <boost/unordered_map.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>

#include "core.h"
#include "Singleton.h"
#include "SimplePeriodicTimer.h"

// --------------------------------------------------------------------------------------------------------------------

#define STATUS_KILO 1000
#define STATUS_MEGA 1000*1000
#define STATUS_GIGA 1000*1000*1000
//#define STATUS_TERA 1000*1000*1000*1000

#define STATUS_KBYTE 1024
#define STATUS_MBYTE 1024*1024
#define STATUS_GBYTE 1024*1024*1024
//#define STATUS_TBYTE 1024*1024*1024*1024

namespace QAppNG
{
    class QCountersBase;

    class QStatusManager : public Singleton<QStatusManager>
    {
    public:
        // define signature for status functions
        typedef fastdelegate::FastDelegate0<std::string> StatusDelegate;

        // ctor
        QStatusManager() 
            : m_executableName("undef")
            , m_started(false)
        {};

        // dtor
        ~QStatusManager()
        {
            //#9263: must be stopped before destroying
            assert( !m_started );
        }

        // start the Status Manager
        void start( UInt16 status_rate = 10 );

        void stop(void);

        // called by client applications to register status delegate functions
        bool addStatus(const std::string &file_name, StatusDelegate status_function);

        // called by client applications to register status delegate functions to a rotating file (in-place overwrite)
        bool addRotatingStatus(const std::string &file_name, StatusDelegate status_function);

        void setSkippingOfFileWrite(std::string& fileName,  bool skip_file_write)
        {
            // check args
            if(fileName.length()==0 || !m_file_table.count(fileName))
                return ;

            m_file_table[fileName].skip_file_write = skip_file_write;
        };

        // called by client applications to remove status. The index to find the status entry is the delegate itself
        bool remStatus(StatusDelegate status_function);

        std::string getHostName()
        {
            return m_hostname;
        }

        std::string getExecutableName()
        {
            return m_executableName;
        }

        // return the status associated to a given file_name
        std::string getStatus( const std::string &file_name );

        void StatusUnitConverter( double &val, std::string &unitname, bool byte_type=true );

        void setExecutableName(const char* executableName);

        bool setCounter(const std::string &file_name, const std::shared_ptr< QAppNG::QCountersBase > theCounterBase);

        double getFromStartTimeMilliseconds() // added in #9078
        {
            boost::posix_time::time_duration diff_start_time = m_status_current_timestamp - m_start_time;
            return static_cast<double>(diff_start_time.total_milliseconds());
        }

        boost::posix_time::ptime getStatusCurrentTimestamp() // added in #9078
        { 
            return m_status_current_timestamp;
        }

        boost::posix_time::time_duration getStatusDumpInterval() // added in #9078
        {
            return m_status_difftime;
        }

    private:
        // status header
        std::string getStatusHeader();

        // dumps status for a particular object
        void statusDumpForDelegate(StatusDelegate status_function);

        // main function that dumps status to file
        void statusDump();

        //#7988 dump status to file
        bool statusDumpToFile(std::string& file_name, StatusDelegate status_function);

        //#7988 close file
        void closeFile(std::string& file_name);

        // table of status files
        struct StatusFileDescriptor
        {
            std::shared_ptr< std::ofstream >               stream;
            UInt16                                           ref_counter;
            std::list <std::shared_ptr< QAppNG::QCountersBase > >  countersBaseList;
            int                                              db_write_decimation_factor;
            bool                                             skip_file_write; // #8793 this variable is used to skip writing of status file and do only database write
            bool                                             noappend_mode; // #8770 this variable is to force in-place overwrite
        };

        // table of status functions
        struct StatusDelegateDescriptor
        {
            std::string                        file;
            UInt16                             ref_counter;
        };

        //TODO: Switch to Simple Periodic Timer to control status dump
        SimplePeriodicTimer                                      m_main_timer;

        std::string                                                               m_executableName;
        
        //#7988
        std::string                                                               m_status_current_header_time;

        bool                                                                      m_started;
        boost::posix_time::ptime                                                  m_start_time;
        boost::posix_time::ptime                                                  m_status_current_timestamp;
        boost::posix_time::ptime                                                  m_status_previous_timestamp;
        boost::posix_time::time_duration                                          m_status_difftime;
        boost::unordered_map< std::string, struct StatusFileDescriptor >          m_file_table;
        
        // we need this lock because we allow to add status function to any moment
        boost::mutex                                                              m_mutex;
        
        std::map< StatusDelegate, struct StatusDelegateDescriptor >               m_function_table;

        std::string m_hostname;
    };

}
#endif //QSTATUSMANAGER_EVO_NG
// --------------------------------------------------------------------------------------------------------------------
// End of file
// --------------------------------------------------------------------------------------------------------------------

