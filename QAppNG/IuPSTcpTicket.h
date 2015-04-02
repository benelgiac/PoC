#ifndef IUPS_TCP_TICKET_H_NG
#define IUPS_TCP_TICKET_H_NG

#include <algorithm>

#include "cal/defs.h"
#include <QAppNG/QLogManager.h>
#include <cal/time/Duration.h>
#include <cal/time/Timestamp.h>
#include "QAppNG/PerThreadSingleton.h"
#include <QAppNG/QSubjectObserver.h>
#include <cal/FastLock.h>
#include <cal/StatsCounters.h>
#include <QAppNG/QObservable.h>
#include <QAppNG/StructDefs.h>

#define TEST_COUNTERS
#define CGI_DELIMITER "-"

namespace QAppNG
{

    template <typename DATA>
    class MyCounter: public cal::detail::LocalCounter<DATA>
    {
    public:
        MyCounter() {};
        void Lock() const
            {
                m_mutex.lock();
            }
        void Unlock() const
            {
                m_mutex.unlock();
            }
    private:
         cal::FastLock m_mutex;
    };

#ifdef TEST_COUNTERS  
    extern MyCounter<UInt64> g_tickets_nb;
    extern MyCounter<UInt64> g_tickets_pd;
    extern MyCounter<UInt64> g_tickets_inmem;
#endif

    static char* DeleteCommas( char* a_arr, int len )
    {
        int i;

        for(i = 0; i < len && a_arr[i]; ++i)
        {
            if(a_arr[i] == ',') 
            {
                a_arr[i] = ' ';
            }     
        }
        return a_arr;
    };

    class IuPSTcpTicket: public QObservable
    {
    public:
        IuPSTcpTicket(const IuPSFilter::TCP * tcp)
            : QObservable(TCP_XDR_TICKET)
            , m_flag(false)
        {
            m_tcp_fields.lastPduTime                = tcp->lastPduTime;
            m_tcp_fields.imsi                       = tcp->imsi;
            m_tcp_fields.imei.tac                   = tcp->imei.tac;
            m_tcp_fields.imei.snr                   = tcp->imei.snr;
            m_tcp_fields.imei.svn                   = tcp->imei.svn;
            m_tcp_fields.bytesDownlink              = tcp->bytesDownlink;
            m_tcp_fields.bytesRtxDownlink           = tcp->bytesRtxDownlink;
            m_tcp_fields.bytesUplink                = tcp->bytesUplink;
            m_tcp_fields.bytesRtxUplink             = tcp->bytesRtxUplink;
            m_tcp_fields.TCP_MS_LatencyMsec         = tcp->TCP_MS_LatencyMsec;
            m_tcp_fields.TCP_NW_LatencyMsec         = tcp->TCP_NW_LatencyMsec;
            m_tcp_fields.TCP_SessionDurationMsec    = tcp->TCP_SessionDurationMsec;
            m_tcp_fields.sai.sac                    = tcp->sai.sac;
            m_tcp_fields.sai.mcc                    = tcp->sai.mcc; 
            m_tcp_fields.sai.mnc                    = tcp->sai.mnc;  
            m_tcp_fields.sai.lac                    = tcp->sai.lac; 
            m_tcp_fields.ci                         = tcp->ci;
        
            strcpy(m_tcp_fields.userAgent,            tcp->userAgent);
            strcpy(m_tcp_fields.longestPath,          tcp->longestPath);

            setQObservableTimestamp(tcp->lastPduTime);

#ifdef TEST_COUNTERS  
            g_tickets_inmem.Increment( 1 );
#endif
        }
    
        virtual ~IuPSTcpTicket() 
        {
#ifdef TEST_COUNTERS
            if(m_flag)
            {
                g_tickets_nb.Increment( 1 ); 
            }
            g_tickets_pd.Increment( 1 );
            g_tickets_inmem.Decrement( 1 );

#endif
        }

      const char * FormatCSV(char * a_buffer)
        {
            sprintf(a_buffer, "%llu,TCP xDR,%llu,%u-%u-%u,%llu,%llu,%llu,%llu,%u,%u,%u,%u,'%s','%s',%d-%d-%d-%d,%d-%d-%d-%d", 
                                static_cast< unsigned long long > (m_tcp_fields.lastPduTime),
                                static_cast< unsigned long long > (m_tcp_fields.imsi),
                                m_tcp_fields.imei.tac, 
                                m_tcp_fields.imei.snr,
                                (UInt16)m_tcp_fields.imei.svn, 
                                static_cast< unsigned long long > (m_tcp_fields.bytesDownlink),
                                static_cast< unsigned long long > (m_tcp_fields.bytesRtxDownlink),                   
                                static_cast< unsigned long long > (m_tcp_fields.bytesUplink),                       
                                static_cast< unsigned long long > (m_tcp_fields.bytesRtxUplink),                    
                                m_tcp_fields.TCP_MS_LatencyMsec,                
                                m_tcp_fields.TCP_NW_LatencyMsec,                 
                                m_tcp_fields.TCP_SessionDurationMsec,            
                                m_tcp_fields.TCP_SessionDurationMsec,            
                                DeleteCommas(m_tcp_fields.userAgent, 129),
                                DeleteCommas(m_tcp_fields.longestPath, 65),        
                                m_tcp_fields.sai.mcc,                   
                                m_tcp_fields.sai.mnc,                   
                                m_tcp_fields.sai.lac,                   
                                m_tcp_fields.sai.sac,         
                                m_tcp_fields.sai.mcc,                
                                m_tcp_fields.sai.mnc,                
                                m_tcp_fields.sai.lac,                 
                                m_tcp_fields.ci );
            return a_buffer;
        }  
   
        UInt64 GetTimestamp() { return m_tcp_fields.startTime; }
        IuPSFilter::TCP GetTcpFields() { return m_tcp_fields; }
        void SetProcessed() { m_flag = true; }
    
        UInt64 getCGI() { return ((UInt64)m_tcp_fields.sai.mcc << 48) | 
                                 ((UInt64)m_tcp_fields.sai.mnc << 32) |
                                 ((UInt64)m_tcp_fields.sai.lac << 16) |
                                 ((UInt64)m_tcp_fields.ci); }

    private:
        IuPSFilter::TCP m_tcp_fields;
        bool m_flag;
    };

    class IuPSUpdateProcedure: public QObservable
    {
    public:
        IuPSUpdateProcedure(UInt64 cgi, UInt8 status)
            : QObservable(IUPS_FILTER_PROCEDURE)
            , m_cgi(cgi)
            , m_status(status) {}
    
        virtual ~IuPSUpdateProcedure() {}

        UInt64 getCGI() { return m_cgi; }
        UInt8 getStatus() { return m_status; }
    
        std::string getCgiString()
        {
            std::stringstream output("");
        
            output << ((m_cgi      ) >> 48) << CGI_DELIMITER 
                   << ((m_cgi << 16) >> 48) << CGI_DELIMITER 
                   << ((m_cgi << 32) >> 48) << CGI_DELIMITER 
                   << ((m_cgi << 48) >> 48);
        
            return output.str();
        }
    
    private:    
        UInt64 m_cgi;
        UInt8  m_status;
    };

}
#endif // IUPS_TCP_TICKET_H_NG


