/*===============================================================
 *
 *    Copyright: (C) 2006 CommProve / Simtel
 *    Filename:  nl_exception.h 
 *    Project: Netledge / RPS
 *
 ================================================================*/

/** \ingroup rps_main
 * @file
 */

#ifndef NL_EXCEPTION_INCLUDE_NG
#define NL_EXCEPTION_INCLUDE_NG
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <QAppNG/nl_osal.h>

#define THROW_EXCEPTION(STRING, ERRORNUM) \
        char except_errbuf[256]; \
        nl_osal_snprintf(except_errbuf, 256, "Exception:  File %s Line %d: %s", __FILE__, __LINE__, STRING); \
        throw CException(except_errbuf, ERRORNUM) 


namespace QAppNG
{
    class CException
    {
      private: 
          char m_error_buf[256];
          int m_error_num;
    
      public:
          CException(char *error_string, int errornum) {
            nl_osal_snprintf(m_error_buf, 256, "%s\n", error_string);
            m_error_num = errornum;
        }
    
         ~CException() { }
    
        void Dump() {
            //NL_LOG(LOG_DEBUG, "%s", m_error_buf);
            //NL_LOG(LOG_DEBUG, "error number: %d\n", m_error_num);
        }
        int GetErrorNum() {
            return m_error_num;
        }
          void LogError()
        {
          //NL_LOG(LOG_DEBUG,"%s", m_error_buf);
    
    #if 0      
          FILE *fp;
          time_t time_sec;
    
          char errlogFname[64];
            errlogFname[0] = '\0';
            CRPS_Config::Instance()->GetErrlogFname(errlogFname);
          if (CRPS_Config::Instance() != NULL)
          {
            if ( errlogFname != NULL)
            {
              fp = FOPEN(errlogFname, "a");
              if(fp == NULL)
              {
                NL_LOG(LOG_DEBUG, "Error, could not open logfile to write exception!\n");
                return;
              }
              nl_osal_time_r(&time_sec);
              char timeStr[64];
              fprintf(fp, "Date: %s", nl_osal_ctime_r(&time_sec,timeStr)); 
              fprintf(fp, m_error_buf);
              fclose(fp);
            }
            else
            {
              NL_LOG(LOG_DEBUG, "CException error: config error logfilename is not specified in config file\n");
            }
          }
          else
          {
            NL_LOG(LOG_DEBUG, "CException error: config is NULL\n");
          }
    #endif
        }            
    };
}
#endif
