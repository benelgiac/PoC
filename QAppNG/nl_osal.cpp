/*===============================================================
 *
 *    Copyright: (C) 2005-2006  CommProve Ltd.
 *    Filename: nl_osal.cpp
 *    Project: Netledge
 *    Description: Definition of OS Abstraction Layer
 *    Author: Aidan Kenny, James Coleman, Mike Manchip
 *    Created: 11/01/06
 *    Document: 
 *  
 *    Notes:
 * 
 * 
 *    History:
 *
 ================================================================*/

#include <QAppNG/nl_osal.h>

namespace QAppNG
{

    int nl_osal_sleep(long sec, long ns)
    { 
#ifdef WIN32
       long ms = sec * 1000 + (ns / 1000);  
       Sleep (ms);
       return 0;
#else  
       struct timespec rqtp;
        rqtp.tv_sec = sec;
        rqtp.tv_nsec = ns;
       // return 0 if okay, -1 for interruption or failure
       return nanosleep (&rqtp, NULL);
#endif 
    }

    int nl_printable(unsigned char c){
      if (c>=32 && c<127) return 1;
      return 0;
    }

    void nl_hexdump(unsigned char *buffer, int len, int bytesperline)
    {
      int i,j,p;
      char str_hex[40], str_char[20];
      for(i=0,p=0;p<len;i++){
        printf("%08x: ",p);
        for(j=0;j<bytesperline;j++){ // p<len && 
          if (p<len) {
              sprintf(str_hex+j*2,"%02x",*(buffer+p));
              sprintf(str_char+j,"%c",nl_printable(*(buffer+p))?*(buffer+p):'.');
            } else {
              sprintf(str_hex+j*2,"  ");
              sprintf(str_char+j," ");
            }
            p++;
         }
        printf("%s %s\n", str_hex, str_char);
      }
      if (i%bytesperline == 0) printf("\n");
    }


}
