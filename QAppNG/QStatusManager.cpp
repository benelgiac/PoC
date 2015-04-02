/** ===================================================================================================================
  * @file    QStatusManager Implementation
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
  * #5774       Stanislav Timinsky                                               Jan-2011      Added stop method and call
  *                                                                                            the statusDump when remStatus
  *                                                                                            is invoked   
  * #7988       C.Guidoccio                                                      Dec-2012      Ticket #7988 - QoSAppMonitoring
  * #8547       F.Lasagni, C.Guidoccio                                           Apr-2013      removed use of legacy version
  *
  * @endhistory
  * ===================================================================================================================
  */

#include <QAppNG/QStatusManager.h>

#include <QAppNG/QCountersBase.h>
#include <QAppNG/QCounters.h> // was legacy

namespace QAppNG
{
    // --------------------------------------------------------------------------------------------------------------------
    void QStatusManager::start( UInt16 status_rate )
    {
        // set start up time
        m_start_time = boost::posix_time::second_clock::local_time();

        // set timestamp for status rate
        m_status_current_timestamp = boost::posix_time::second_clock::local_time();
        m_status_previous_timestamp=m_status_current_timestamp;

        // set and start TIMER
        m_main_timer.start( status_rate, fastdelegate::MakeDelegate(this, &QStatusManager::statusDump) );

        m_started = true;

        // read host name and application name
        int theHostNameLen = 512;
        char theHostName[512];
        memset(theHostName,0,512);
        if( gethostname(theHostName,theHostNameLen) == 0)
        {
            // replace postgresql invalid characters in hostname
            int k=0;
            while(theHostName[k]!=0) 
            {
                if(!isalnum(theHostName[k]))
                {
                    int ascii = (int) theHostName[k];
                    std::cout << " --> QStatusManager::start() !alnum[" << theHostName[k] << "] = " << ascii  << std::endl; // test code #9531
                    theHostName[k] = '_';
                }
                k++;
            }
            // set the postgresql valid hostname
            m_hostname = theHostName;
        }
        else
        {
            m_hostname = "undefhost";
        }
        std::cout << " --> QStatusManager::start() hostname[" << m_hostname << "]" << std::endl; // test code #9531
    }

    void QStatusManager::stop(void)
    {
        m_main_timer.stop();

        statusDump();

        m_started = false;
    }

    // --------------------------------------------------------------------------------------------------------------------
    bool QStatusManager::setCounter(const std::string &file_name, const std::shared_ptr< QAppNG::QCountersBase > theCounterBase)
    {
        if ( !m_file_table.count(file_name) )
        {
            return false;
        }
        m_file_table[file_name].countersBaseList.push_back(theCounterBase);
        return true;
    }

    // --------------------------------------------------------------------------------------------------------------------
    bool QStatusManager::addStatus(const std::string &file_name, StatusDelegate status_function )
    {
        // #7988 inserted check on null or empty file_name
        if(file_name.length() == 0)
        {
            return false;
        }

        // get lock
        boost::unique_lock<boost::mutex> lock(m_mutex);
        struct StatusDelegateDescriptor xStatusDelegateDescriptor;

        // if status_function already present, the addStatus(...) fails
        if ( m_function_table.count(status_function) )
        {
            m_function_table[status_function].ref_counter++;
            return false;
        }

        // if file_name is not already present, open it and add it to file_table
        if ( !m_file_table.count(file_name) )
        {
            struct StatusFileDescriptor xStatusFileDescriptor;
            std::shared_ptr<std::ofstream> status_file( new std::ofstream() );

            xStatusFileDescriptor.stream = status_file;
            xStatusFileDescriptor.ref_counter = 1;
            xStatusFileDescriptor.db_write_decimation_factor = 0;
            xStatusFileDescriptor.skip_file_write = false;
            xStatusFileDescriptor.noappend_mode = false;
            //status_file->open( file_name.c_str(), std::ios::app );

            m_file_table.insert( boost::unordered_map< std::string, struct StatusFileDescriptor >::value_type( file_name, xStatusFileDescriptor) );
        }
        else
        {
            m_file_table[file_name].ref_counter++;
        }

        xStatusDelegateDescriptor.file = file_name;
        xStatusDelegateDescriptor.ref_counter = 1;

        // add pair (status_function, file_name) to m_function_table
        m_function_table.insert( std::map< StatusDelegate, struct StatusDelegateDescriptor >::value_type( status_function,  xStatusDelegateDescriptor) );

        return true;
    }

    // --------------------------------------------------------------------------------------------------------------------
    bool QStatusManager::addRotatingStatus(const std::string &file_name, StatusDelegate status_function )
    {
        if ( !addStatus(file_name, status_function ) )
        {
            return false;
        }

        // Check if rotate mode is not already set
        if ( !m_file_table[file_name].noappend_mode )
        {
            // Chek if someone is using the same file in append mode
            if ( m_file_table[file_name].ref_counter > 1 )
            {
                // Cannot enable rotate mode on the given file.Revert! 
                
                //Get lock
                boost::unique_lock<boost::mutex> lock(m_mutex);
                
                --m_file_table[file_name].ref_counter;
                m_function_table.erase(status_function);

                //status not correctly addedd
                return false;
            }
            else
            {
                // Enable rotate mode
                m_file_table[file_name].noappend_mode = true;
                return true;
            }
        }

        // Someone else is already using thi given file in rotate mode.Nothing to do here
        return true;
    }

    // --------------------------------------------------------------------------------------------------------------------

    bool QStatusManager::remStatus(StatusDelegate status_function)
    {
        //#9263: avoid status dump if not started
        if ( !m_started )
        {
            return true;
        }

        // let's dump the status last time (or may be first)
        statusDumpForDelegate(status_function); 

        // get lock
        boost::unique_lock<boost::mutex> lock(m_mutex);

        if ( m_function_table.count(status_function) )
        {
            // first remove the entry from file table
            if (m_file_table[m_function_table[status_function].file].ref_counter <= 1)
            {
                m_file_table.erase(m_function_table[status_function].file);
            }
            else
            {
                m_file_table[m_function_table[status_function].file].ref_counter--;
            }

            // remove entry from status table
            if (m_function_table[status_function].ref_counter <= 1)
            {
                m_function_table.erase(status_function);
            }
            else
            {
                m_function_table[status_function].ref_counter--;
            }

            return true;
        }
        else
        {
            return false;
        }
    }

    // --------------------------------------------------------------------------------------------------------------------

    std::string QStatusManager::getStatus( const std::string &file_name )
    {
        std::stringstream output; output.str(""); output.clear();

        // add status header
        output << getStatusHeader();

        // get lock
        boost::unique_lock<boost::mutex> lock(m_mutex);

        // search all the status delegate associated to "file_name"
        std::map< StatusDelegate, struct StatusDelegateDescriptor >::iterator it;
        for ( it = m_function_table.begin(); it != m_function_table.end(); ++it )
        {
            // get file_name
            std::string found_file_name( it->second.file );

            if (found_file_name == file_name)
            {
                // add status
                output << it->first();
            }
        }

        return output.str();
    }

    // --------------------------------------------------------------------------------------------------------------------
    void QStatusManager::setExecutableName(const char* executableName)
    {
        const std::string toBeSplitted(executableName);

#ifdef WIN32
        boost::char_separator<char> pathSep("\\");
#else
        boost::char_separator<char> pathSep("/");
#endif
        boost::tokenizer< boost::char_separator< char > > tokens(toBeSplitted, pathSep);
        std::string exeWithExtension;

        // copy values until last element
        for ( boost::tokenizer<boost::char_separator<char> >::iterator it = tokens.begin();
            it != tokens.end();
            ++it)
        {
            exeWithExtension = *it;
        }

#ifdef WIN32
        m_executableName = exeWithExtension.substr(0, exeWithExtension.find(".", 0));
#else
        m_executableName = exeWithExtension;
#endif
    }

    // --------------------------------------------------------------------------------------------------------------------
    void QStatusManager::statusDump()
    {
        // get lock
        boost::unique_lock<boost::mutex> lock(m_mutex);

        m_status_current_timestamp = boost::posix_time::microsec_clock::local_time();  // #9078 was "second_clock"
        m_status_difftime = m_status_current_timestamp - m_status_previous_timestamp;

        std::map< StatusDelegate, struct StatusDelegateDescriptor >::iterator it;
        for ( it = m_function_table.begin(); it != m_function_table.end(); ++it )
        {
            // get file_name
            std::string file_name( it->second.file );
            statusDumpToFile(file_name,it->first);
        }

        // close all open files
        boost::unordered_map< std::string, struct StatusFileDescriptor >::iterator file_it;
        for ( file_it = m_file_table.begin(); file_it != m_file_table.end(); ++file_it )
        {
            std::string file_name( file_it->first );
            closeFile(file_name);
        }

        m_status_previous_timestamp = m_status_current_timestamp;
    }

    // --------------------------------------------------------------------------------------------------------------------
    // #7988 method to remove some duplicate code
    inline void QStatusManager::closeFile(std::string& fileName)
    {
        if(fileName.length()==0 || !m_file_table.count(fileName))
        {
            return;
        }

        *( m_file_table[fileName].stream ) << "*************************************************************" << std::endl << std::endl;
        m_file_table[fileName].stream->close();
    }

    // --------------------------------------------------------------------------------------------------------------------
    // #7988 method to remove some duplicate code
    inline bool QStatusManager::statusDumpToFile(std::string& fileName, StatusDelegate status_function)
    {
        // check args
        if(fileName.length()==0 || !m_file_table.count(fileName))
        {
            return false;
        }

        if(m_file_table[fileName].skip_file_write)
        {
            // only invoke status function to eventually update counters
            status_function();
        }
        else // write on status file
        {
            if ( !m_file_table[fileName].stream->is_open() )
            {
                if( m_file_table[fileName].noappend_mode )
                {
                    // open file stream in rotate mode
                    m_file_table[fileName].stream->open( fileName.c_str() );
                }
                else
                {
                    // open file stream in append mode
                    m_file_table[fileName].stream->open( fileName.c_str(), std::ios::app );
                }

                // dump the header
                *( m_file_table[fileName].stream ) << getStatusHeader();
            }

            // dump status
            *( m_file_table[fileName].stream ) << status_function();
        }

        return true;
    }

    // --------------------------------------------------------------------------------------------------------------------
    void QStatusManager::statusDumpForDelegate(StatusDelegate status_function)
    {
        // get lock
        boost::unique_lock<boost::mutex> lock(m_mutex);

        if (m_function_table.count(status_function))
        {
            // get file_name
            std::string file_name(m_function_table[status_function].file);

            if(statusDumpToFile(file_name,status_function))
            {
                closeFile(file_name);
            }
        }
    }

    // --------------------------------------------------------------------------------------------------------------------
    std::string QStatusManager::getStatusHeader()
    {
        std::stringstream header; 
        header.clear(); 

        boost::posix_time::ptime  now_time = boost::posix_time::second_clock::local_time();

        // prepare timestamp format
        boost::posix_time::time_facet *facet = new boost::posix_time::time_facet("%Y/%m/%d %H:%M:%S");
        auto testHook = header.imbue( std::locale(std::cout.getloc(), facet) );

        // set member variable to use in database write 
        header << now_time;
        m_status_current_header_time = header.str();
        header.str("");

        boost::posix_time::time_duration UpTime  = now_time - m_start_time;

        UInt32 UpTimeSeconds  = static_cast<UInt32>(UpTime.total_seconds());

        UInt32 UpDays  = (UpTimeSeconds/86400);
        UInt32 UpHours = (UpTimeSeconds%86400)/3600;
        UInt32 UpMins  = (UpTimeSeconds%3600)/60;
        UInt32 UpSecs  = (UpTimeSeconds%60);

        header << "*************************************************************" << std::endl;
        //////header << "QStatusManager: " << boost::posix_time::to_simple_string(now_time) << std::endl;
        header << "QStatusManager: " << now_time << std::endl;
        header << "Up since: " << UpDays  << "d:" << UpHours << "h:" << UpMins  << "m:" << UpSecs  << "s. "
            << "Started @" << m_start_time << std::endl;
        //////    << "Started @" << boost::posix_time::to_simple_string(m_start_time) << std::endl;
        header << "*************************************************************" << std::endl;
        header << std::endl;
        return header.str();
    };

    // --------------------------------------------------------------------------------------------------------------------
    void QStatusManager::StatusUnitConverter( double &val, std::string &unitname, bool byte_type/*=true */ )
    {
        if (byte_type)
        {
            if(val > STATUS_KBYTE && val <= STATUS_MBYTE)
            {
                val /= STATUS_KBYTE;
                unitname="K";
            }
            else if(val > STATUS_MBYTE && val <= STATUS_GBYTE)
            {
                val /= STATUS_MBYTE;
                unitname="M";
            }
            else if(val > STATUS_GBYTE)
            {
                val /= STATUS_GBYTE;
                unitname="G";
            }
        }
        else
        {
            if(val > STATUS_KILO && val <= STATUS_MEGA)
            {
                val /= STATUS_KILO;
                unitname="K";
            }
            else if(val > STATUS_MEGA && val <= STATUS_GIGA)
            {
                val /= STATUS_MEGA;
                unitname="M";
            }
            else if(val > STATUS_GIGA)
            {
                val /= STATUS_GIGA;
                unitname="G";
            }
        }
    }

// --------------------------------------------------------------------------------------------------------------------
}

// --------------------------------------------------------------------------------------------------------------------
// End of file
// --------------------------------------------------------------------------------------------------------------------

