#if !defined ( AppConfigManagerImpl_H_NG )
#define AppConfigManagerImpl_H_NG
// ===========================================================================
/// @file
/// @brief This file contains inline functions for AppConfigManager class
///
/// @copyright
/// @history
/// REF#        Who              When        What
/// 768         Aidan Kenny      14-Jun-2007 Original development
/// 954         Steve Mellon     13-Jul-2007 Use ConfigFile class to find files
/// 1039        Brian Molina     12-Jul-2007 QuidWaiter timer and max entries configurable.
/// 983         Brian Molina     01-Aug-2007 QQ List Partial Writes configurable.
/// 1166       Bennett Schneider 08-Aug-2007 Use ConfigFile class for all files
/// @endhistory
///
// ===========================================================================

/*  ...Include files
*/
#include    "cal/ConfigFile.h"
#include <stdexcept>

namespace QAppNG
{

    /*  ...Class definitions
    */

    // ===========================================================================
    // class constructor
    //
    // History
    // REF#        Who              When        What
    // 768         Aidan Kenny      14-jun-2007 Original development
    //
    // ===========================================================================

    inline AppConfigManager::AppConfigManager ( )
        {
        }

    // ===========================================================================
    // class destructor
    //
    // History
    // REF#        Who              When        What
    // 768         Aidan Kenny      14-Jun-2007 Original development
    //
    // ===========================================================================

    inline AppConfigManager::~AppConfigManager ()
        {
        }

    // ===========================================================================
    // member accessor methods
    //
    // History
    // REF#        Who              When        What
    // 768         Aidan Kenny      14-Jun-2007 Original development
    // 954         Steve Mellon     13-Jul-2007 Use ConfigFile to find files in Sets
    //
    // ===========================================================================



    inline const char * AppConfigManager::GetTicketDefPath ( )
    {
       return m_ticketDefPath.c_str ( );
    };

    inline const char * AppConfigManager::GetTargetCfgFile ( )
    {
       return m_targetCfgFile.c_str ( );
    };

    inline const char * AppConfigManager::GetSlabConfigFile ( )
        {
        return m_slabConfigFile.c_str ( );
        };

    inline const char * AppConfigManager::GetXdrConfigFile ( )
        {
        return m_xdrConfigFile.c_str();
        };

    inline const char * AppConfigManager::GetKpmConfigFile ( )
        {
        return m_kpmConfigFile.c_str();
        };

    inline const bool AppConfigManager::IsXdrEnabled()
        {
        return m_xdrTicketProductionEnabled;
        };

    inline const bool AppConfigManager::IsXdrPartialOnly()
        {
        return m_xdrTicketPartialOnly;
        };

    inline const bool AppConfigManager::IsXdrObfuscated()
        {
        return m_xdrTicketObfuscated;
        };

    inline const char * AppConfigManager::GetKpmRuleConfigFile ( )
        {
        return m_kpmRuleConfigFile.c_str ( );
        };

    inline const char * AppConfigManager::GetPlayFile ( )
        {
        if(m_playFile.size() == 0)
            {
            return(NULL);
            }
        return m_playFile.c_str ( );
        };

    inline UInt16 AppConfigManager::GetPort ( )
        {
        return m_port;
        }

    inline UInt16 AppConfigManager::GetNumPduCreationThreads ( )
        {
        return m_num_pdu_creation_threads;
        }

    inline UInt16 AppConfigManager::GetPduCreationMaxQueueSize ( )
        {
        return m_pdu_creation_max_queue_size;
        }

    inline UInt16 AppConfigManager::GetNumUserContextThreads ( )
        {
        return m_num_user_context_threads;
        }

    inline UInt16 AppConfigManager::GetUserContextMaxQueueSize ( )
        {
        return m_user_context_max_queue_size;
        }

    inline UInt16 AppConfigManager::GetNumPduDecodeThreads ( )
        {
        return m_num_pdu_decode_threads;
        }

    inline UInt16 AppConfigManager::GetPduDecodeMaxQueueSize ( )
        {
        return m_pdu_decode_max_queue_size;
        }

    inline UInt16 AppConfigManager::GetPrimarySeqWinFwdSize ( )
        {
        return m_prim_seq_win_fwd_size;
        }

    inline UInt16 AppConfigManager::GetPrimarySeqWinBwdSize ( )
        {
        return m_prim_seq_win_bwd_size;
        }

    inline UInt16 AppConfigManager::GetSecondarySeqWinFwdSize ( )
        {
        return m_secondary_seq_win_fwd_size;
        }

    inline UInt16 AppConfigManager::GetSecondarySeqWinBwdSize ( )
        {
        return m_secondary_seq_win_bwd_size;
        }

    inline UInt16 AppConfigManager::GetTertiarySeqWinFwdSize ( )
        {
        return m_tertiary_seq_win_fwd_size;
        }

    inline UInt16 AppConfigManager::GetTertiarySeqWinBwdSize ( )
        {
        return m_tertiary_seq_win_bwd_size;
        }

    inline bool AppConfigManager::GetEnableTicketIndexing ( )
        {
        return m_enable_ticket_indexing;
        }

    inline const char * AppConfigManager::GetMcastIntf ( )
        {
        return m_multicastInterface.c_str ( );
        }

    // =============================================================================
    // Get LAC File

    inline const char* AppConfigManager::GetLacFile ( )
        {
        return m_lacFile.c_str();
        }

    // =============================================================================
    // Get Quid Wait Timer

    inline UInt16 AppConfigManager::GetQuidWaitTimer( void )
        {
        return m_quidWaitTimer;
        }

    // =============================================================================
    // Get Quid Wait Entries

    inline UInt64 AppConfigManager::GetQuidWaitEntries( void )
        {
        return m_quidWaitEntries;
        }

    // =============================================================================
    // Get Quid List Flush Size

    inline UInt16 AppConfigManager::GetQuidListFlushSize( void )
        {
        return m_quidListFlushSize;
        }

    // =============================================================================
    // Get Quid List Directory ID

    inline Int8 AppConfigManager::GetQuidListDirId( void )
        {
        return m_quidListDirId;
        }

    // =============================================================================
    // Get Log Config

    inline const char* AppConfigManager::GetLogCfg ( )
        {
        return m_logCfgFile.c_str();
        }

    inline UInt8 AppConfigManager::GetIubProbeFilterEnableFlag ( )
        {
        return m_iubProbeFilterEnable;
        }

    inline UInt8 AppConfigManager::GetIupsIucsProbeFilterEnableFlag ( )
        {
        return m_iupsIucsProbeFilterEnable;
        }

    inline UInt8 AppConfigManager::GetDaemon ( )
        {
        return m_daemonize;
        }

    inline UInt8 AppConfigManager::GetFPCRCErrorsEnableLog ( )
        {
        m_debugFpCrcLogLock.Lock();
        UInt8 rv = m_debugFpCrcLog;
        m_debugFpCrcLogLock.Unlock();
        return rv;
        }

    inline UInt8 AppConfigManager::GetALCAPERQECFEnableLog ( )
        {
        m_debugAlcapErqEcfLogLock.Lock();
        UInt8 rv = m_debugAlcapErqEcfLog;
        m_debugAlcapErqEcfLogLock.Unlock();
        return rv;
        }

    inline UInt8 AppConfigManager::GetEnableSSCOPSequenceNumberChecks ( )
        {
        m_sscopSequenceNumberChecksLock.Lock();
        UInt8 rv = m_sscopSequenceNumberChecks;
        m_sscopSequenceNumberChecksLock.Unlock();
        return rv;
        }

    inline UInt8 AppConfigManager::GetEnableVpiVciCidCounters ( )
        {
        m_debugVpiVciCidCountersLock.Lock();
        UInt8 rv = m_debugVpiVciCidCounters;
        m_debugVpiVciCidCountersLock.Unlock();
        return rv;
        }

    inline std::vector<Ticket::CommDestination> & AppConfigManager::GetKpmDest ( )
        {
        return m_KpmDest;
        }
    
    inline cal::CommParameters & AppConfigManager::GetKpmSrc ( )
        {
        return m_KpmSrc;
        }

    inline std::string & AppConfigManager::GetKpmMcastAddr ( )
        {
        return m_McastAddr;
        }

    inline AppConfigManager::KPMSTATUS AppConfigManager::GetKpmStatus ( )
        {
        return m_KpmStatus;
        }

    inline TicketDefinition::SUPPRESS_DATA AppConfigManager::GetSuppressDataFlag(void)
        {
        return m_SuppressData;
        }

    inline UInt8 AppConfigManager::GetReceiveFromPlayer ( )
        {
        return m_receiveFromPlayer;
        }

    inline UInt16 AppConfigManager::GetGrmode ( )
        {
        return m_Grmode;
        };

    inline UInt16 AppConfigManager::GetNSmode ( )
        {
        return m_NSmode;
        };

    inline void AppConfigManager::SetRetxPort( UInt16 a_retxPort )
        {
        m_retxPort = a_retxPort;
        }

    inline void AppConfigManager::SetEnableRTX ( UInt8 a_enable_rtx)
        {
    printf("setting to %d", a_enable_rtx);    
        m_enable_rtx = a_enable_rtx;
        }

    inline void AppConfigManager::SetMaxRecordingFileLength( UInt32 a_length )
        {
        m_maxRecordingFileLengthSecs = a_length;
        }

    inline void AppConfigManager::SetMaxRecordingFileSize( UInt32 a_size )
        {
        m_maxRecordingFileSizeMB = a_size;
        }

    inline void AppConfigManager::SetPrimarySequencerType( const std::string a_primarySequencerType )
        {
        m_primarySequencerType = a_primarySequencerType;
        }

    inline void AppConfigManager::SetSecondarySequencerType( const std::string a_secondarySequencerType )
    {
        m_secondarySequencerType = a_secondarySequencerType;
    }

    inline UInt32 AppConfigManager::GetMaxRecordingFileLength()
        {
        return m_maxRecordingFileLengthSecs;
        }

    inline UInt32 AppConfigManager::GetMaxRecordingFileSize()
        {
        return m_maxRecordingFileSizeMB;
        }

    inline std::string AppConfigManager::GetPrimarySequencerType()
        {
            return m_primarySequencerType;
        }

    inline std::string AppConfigManager::GetSecondarySequencerType()
    {
        return m_secondarySequencerType;
    }

    inline UInt8 AppConfigManager::GetEnableREC ( )
        {
        return m_enable_rec;
        }


    inline UInt8 AppConfigManager::GetEnableRTX ( )
        {
        return m_enable_rtx;
        }


    inline UInt8 AppConfigManager::GetEnableSEQE ( )
        {
        return m_enable_seqe;
        }

    inline UInt8 AppConfigManager::GetEnableDecode ( )
        {
        return m_enable_decode;
        }

    inline UInt8 AppConfigManager::GetEnableSSEQ ( )
        {
        return m_enable_sseq;
        }

    inline UInt8 AppConfigManager::GetEnableSSEQE ( )
        {
        return m_enable_sseqe;
        }

    inline UInt8 AppConfigManager::GetEnableUserContext ( )
        {
        return m_enable_usercontext;
        }


    inline UInt8 AppConfigManager::GetEnableTSEQ ( )
        {
        return m_enable_tseq;
        }

    inline UInt8 AppConfigManager::GetEnableTSEQE ( )
        {
        return m_enable_tseqe;
        }

    inline UInt8 AppConfigManager::GetEnableCellContext ( )
        {
        return m_enable_cellcontext;
        }

    inline UInt16 AppConfigManager::GetRetxPort ( )
        {
        return m_retxPort;
        }

    inline const char * AppConfigManager::GetKpmPath ()
    {
           return m_kpmPath.c_str ( ); 
    };

    inline UInt16 AppConfigManager::GetCommandHandlerPort ( )
       {
       return m_CommandHandlerPort;
       };

    inline UInt16 AppConfigManager::GetEnableAlarm()
    {
        return m_EnableAlarm;
    };

    inline std::string AppConfigManager::GetAlarmProbeConfFile()
    {
        return m_AlarmProbeConfFile;

    };

    inline UInt8 AppConfigManager::GetSkipAbisProcess()
    {
        return m_SkipAbisProcess;
    };

    inline std::string AppConfigManager::GetProbeTypeConfFile()
    {
        return m_ProbeTypeConfFile;

    };

    inline int AppConfigManager::GetProbePairType()
    {
        return m_ProbePairType;
    };

     inline void AppConfigManager::SetProbePairType(int type)
     {
         m_ProbePairType=type;
     };



     inline std::string AppConfigManager::GetOutputTicketConfFile()
     {
         return m_OutputTicketConfFile;
     };

     inline UInt8 AppConfigManager::GetEnableOutputTicket()
     {
         return m_EnableOutputTicket;
     };


    inline void AppConfigManager::SetKpmPath ( const std::string& a_kpmPath )
    {
        m_kpmPath = a_kpmPath;
    };


    inline size_t AppConfigManager::GetCommQueueSize( )
        {
        return m_CommQueueSize;
        }

    inline UInt16 AppConfigManager::GetImsiStatFrequency ()
        {
        return m_ImsiStatFrequency;
        }

    inline void AppConfigManager::SetConfigFile ( const std::string& a_configFile )
        {
        if ( cal::ConfigFile::Instance().FindFile ( a_configFile, m_configFile ) == cal::Status_Error)
            // ...print out the contents of the ConfigFile object to help user
            std::cout  << "could not find config file! " << cal::ConfigFile::Instance();

        };

    inline void AppConfigManager::SetKpmConfigFile ( const std::string& a_kpmConfigFile )
        {
        if ( cal::ConfigFile::Instance().FindFile ( a_kpmConfigFile, m_kpmConfigFile ) == cal::Status_Error)
            // ...print out the contents of the ConfigFile object to help user
            std::cout  << "could not find KpmConfig file '" << a_kpmConfigFile << "'! " << cal::ConfigFile::Instance();

        };

    inline void AppConfigManager::SetPlayFile ( const std::string& a_playFile )
        {
        if ( cal::ConfigFile::Instance().FindFile ( a_playFile, m_playFile ) == cal::Status_Error)
            // ...print out the contents of the ConfigFile object to help user
            std::cout  << "could not find play file! " << cal::ConfigFile::Instance();
        };

    inline void AppConfigManager::SetTicketDefPath ( const std::string& a_ticketDefPath)
        {
        if ( cal::ConfigFile::Instance().FindDir ( a_ticketDefPath, m_ticketDefPath ) == cal::Status_Error)
            // ...print out the contents of the ConfigFile object to help user
            std::cout  << "could not find TicketDefPath! " << cal::ConfigFile::Instance();
        };

    inline void AppConfigManager::SetXdrConfigFile(const std::string& a_xdrConfigFile)
    {
        m_xdrConfigFile=a_xdrConfigFile;
    }

    inline void AppConfigManager::SetIsXdrEnabled(const bool a_enable)
    {
        m_xdrTicketProductionEnabled = a_enable;
    }

    inline void AppConfigManager::SetIsXdrPartialOnly(const bool a_enable)
    {
        m_xdrTicketPartialOnly = a_enable;
    }

    inline void AppConfigManager::SetIsXdrObfuscated(const bool a_enable)
    {
        m_xdrTicketObfuscated = a_enable;
    }

    inline void AppConfigManager::SetSlabConfigFile ( const std::string& a_slabConfigFile)
        {
        if ( cal::ConfigFile::Instance().FindFile ( a_slabConfigFile, m_slabConfigFile ) == cal::Status_Error)
            {
            // ...print out the contents of the ConfigFile object to help user
            std::cout  << "could not find slab config file! " << cal::ConfigFile::Instance();
            }
        };

    inline void AppConfigManager::SetKpmRuleConfigFile ( const std::string& a_kpmRuleConfigFile)
        {
        if ( cal::ConfigFile::Instance().FindFile ( a_kpmRuleConfigFile
                                                  , m_kpmRuleConfigFile ) == cal::Status_Error)
            {
            // ...print out the contents of the ConfigFile object to help user
            std::cout  << "could not find kpm rule config file! " << cal::ConfigFile::Instance();
            throw std::runtime_error("could not rules config file!");
            }
    
        m_KpmStatus = KPMSTATUS (m_KpmStatus | KPMSTATUS_LOCAL );
        };

    inline void AppConfigManager::SetTargetCfgFile ( const std::string& a_targetCfgFile )
        {
        if ( cal::ConfigFile::Instance().FindFile ( a_targetCfgFile, m_targetCfgFile ) == cal::Status_Error)
            // ...print out the contents of the ConfigFile object to help user
            std::cout  << "could not find target config file! " << cal::ConfigFile::Instance();
        }

    inline void AppConfigManager::SetPort ( UInt16 a_port )
        {
        m_port = a_port;
        };

    // =============================================================================
    // Set Multicast Interface

    inline void AppConfigManager::SetMcastIntf ( const std::string& a_multicastInterface )
        {
        m_multicastInterface = a_multicastInterface;
        };

    // =============================================================================
    // Set Quid Wait Timer

    inline void AppConfigManager::SetQuidWaitTimer( UInt16 a_timeVal )
        {
        m_quidWaitTimer = a_timeVal;
        }

    // =============================================================================
    // Set Quid Wait Entries

    inline void AppConfigManager::SetQuidWaitEntries( UInt64 a_numEntries )
        {
        m_quidWaitEntries = a_numEntries;
        }

    // =============================================================================
    // Set Quid List Flush Size

    inline void AppConfigManager::SetQuidListFlushSize( UInt16 a_flushSize )
        {
        m_quidListFlushSize = a_flushSize;
        }

    // =============================================================================
    // Set Quid List Dir ID

    inline void AppConfigManager::SetQuidListDirId( UInt8 a_dirId )
        {
        m_quidListDirId = a_dirId;
        }

    // =============================================================================
    // Set Log Config

    inline void AppConfigManager::SetLogCfg ( const std::string& a_logCfg )
        {
        if ( cal::ConfigFile::Instance().FindFile ( a_logCfg, m_logCfgFile) == cal::Status_Error)
            {
            std::cout  << "could not find config file! " << cal::ConfigFile::Instance();
            }
        }

    inline void AppConfigManager::SetLacFile ( const std::string& a_lacFile )
        {
        if ( cal::ConfigFile::Instance().FindFile ( a_lacFile, m_lacFile ) == cal::Status_Error)
            // ...print out the contents of the ConfigFile object to help user
            std::cout  << "could not find config file! " << cal::ConfigFile::Instance();
        };

    inline void AppConfigManager::SetIubProbeFilterEnableFlag ( UInt8 a_probeFilterEnable )
        {
        m_iubProbeFilterEnable = a_probeFilterEnable;
        };

    inline void AppConfigManager::SetIupsIucsProbeFilterEnableFlag ( UInt8 a_probeFilterEnable )
        {
        m_iupsIucsProbeFilterEnable = a_probeFilterEnable;
        };

    inline void AppConfigManager::SetFPCRCErrorsEnableLog ( UInt8 a_flag )
        {
        m_debugFpCrcLogLock.Lock();
        m_debugFpCrcLog = a_flag;
        m_debugFpCrcLogLock.Unlock();
        };

    inline void AppConfigManager::SetALCAPERQECFEnableLog ( UInt8 a_flag )
        {
        m_debugAlcapErqEcfLogLock.Lock();
        m_debugAlcapErqEcfLog = a_flag;
        m_debugAlcapErqEcfLogLock.Unlock();
        };

    inline void AppConfigManager::SetEnableSSCOPSequenceNumberChecks ( UInt8 a_flag )
        {
        m_sscopSequenceNumberChecksLock.Lock();
        m_sscopSequenceNumberChecks = a_flag;
        m_sscopSequenceNumberChecksLock.Unlock();
        };

    inline void AppConfigManager::SetEnableVpiVciCidCounters ( UInt8 a_flag )
        {
        m_debugVpiVciCidCountersLock.Lock();
        m_debugVpiVciCidCounters = a_flag;
        m_debugVpiVciCidCountersLock.Unlock();
        };

    inline void AppConfigManager::SetDaemon ( UInt8 a_daemonize )
        {
        m_daemonize = a_daemonize;
        };

    inline void AppConfigManager::SetReceiveFromPlayer( UInt8 a_recvFromPlayerFlag )
        {
        m_receiveFromPlayer = a_recvFromPlayerFlag;
        }

    inline
    void AppConfigManager::SetKpmDest( const Ticket::CommDestination a_commParams )
        {
        m_KpmDest.push_back ( a_commParams );
        m_KpmStatus = KPMSTATUS ( m_KpmStatus | KPMSTATUS_REMOTE );
        }

    inline
    void AppConfigManager::SetKpmSrc( const cal::CommParameters & a_commParams )
        {
        m_KpmSrc = a_commParams;
        }

    inline void AppConfigManager::SetKpmIntf ( const std::string& a_srcIp )
        {
        m_KpmSrc.m_addr.SetHost( a_srcIp.c_str() );
        //m_KpmStatus = KPMSTATUS ( m_KpmStatus | KPMSTATUS_REMOTE );
        }

    inline void AppConfigManager::SetKpmPort ( const int a_destPort )
        {
        m_KpmSrc.m_addr.SetService( static_cast<UInt16> ( a_destPort) );
        //m_KpmStatus = KPMSTATUS ( m_KpmStatus | KPMSTATUS_REMOTE );
        }

    inline void AppConfigManager::SetKpmMbox ( const int a_destMbox )
        {
        m_KpmSrc.m_addr.SetMailboxId( static_cast<cal::ThreadAddress::MailboxId> ( a_destMbox ) );
        //m_KpmStatus = KPMSTATUS ( m_KpmStatus | KPMSTATUS_REMOTE );
        }

    inline void AppConfigManager::SetKpmMcastAddr ( const std::string & a_address )
        {
        m_McastAddr = a_address;
        }

    inline void AppConfigManager::SetRecordingFile ( const std::string& a_destfilename)
        {
        m_recordingfile=a_destfilename;
        }
    inline const char * AppConfigManager::GetRecordingFile ( )
    {
        return m_recordingfile.c_str();
    }


    inline void AppConfigManager::SetCommQueueSize( const size_t a_queueSize )
        {
        m_CommQueueSize = a_queueSize;
        }

    inline void AppConfigManager::SetCommandHandlerPort ( UInt16 a_port )  
       {
       m_CommandHandlerPort = a_port;
       };

    inline void AppConfigManager::SetSkipAbisProcess ( UInt8 a_value )
        {
        m_SkipAbisProcess = a_value;
        }

    inline std::string AppConfigManager::GetConfigFileDirectory ( )
    {
        std::string rv;
#ifdef WIN32
        int l =(int) m_configFile.rfind('\\', -1);
#else
        int l = m_configFile.rfind('/', -1);
#endif

        if (l != -1)
        {
            rv = m_configFile.substr(0, l+1);
        }
        return rv;
    }

    /// Set Gr and deciphering enabled/disabled
    inline void AppConfigManager::SetGrmode ( UInt16 a_grmode )
        {
        m_Grmode = a_grmode;
        };

    /// Set NS layer mode
    inline void AppConfigManager::SetNSmode ( UInt16 a_nsmode )
        {
        m_NSmode = a_nsmode;
        };
}
#endif // AppConfigManagerImpl_H_NG

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  ...End of file */



