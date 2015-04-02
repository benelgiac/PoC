/*===============================================================
 *
 *    Copyright: (C) 2005-2006  CommProve Ltd.
 *    Filename: nl_osal.h
 *    Project: Netledge
 *    Description: Declaration of OS Abstraction Layer
 *    Author: Aidan Kenny
 *    Created: 11/01/06
 *    Document: 
 *  
 *    Notes:
 * 
 * 
 *    History:
 *
 ================================================================*/

#ifndef __NL_OSAL_INCLUDE___NG
#define __NL_OSAL_INCLUDE___NG

#include <QAppNG/core.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
#include <WinSock2.h>
#else
#include <netinet/in.h>
#include <time.h>
//#include <netdb.h>
//#include <sys/socket.h>
//#include <dirent.h>
//#include <semaphore.h>
//#include <thread.h>
//#include <netinet/in.h>
//#include <sys/types.h>
//#include <sys/wait.h> // wait (solaris)
//#include <unistd.h> // NULL (solaris)
//#include <strings.h> // memset
//#include <sys/sem.h>
#endif

#ifdef WIN32
   #define nl_osal_snprintf _snprintf_s
#else
   #define nl_osal_snprintf snprintf 

#endif

namespace QAppNG
{
    int nl_osal_sleep(long sec, long ns=0);
    int nl_printable(unsigned char c);
    void nl_hexdump(unsigned char *buffer, int len, int bytesperline);

}
#endif /* __NL_OSAL_INCLUDE___NG */
