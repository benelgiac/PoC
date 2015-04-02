/* 
 * File:   CalTypes.h
 * Author: giacomo_benelli
 *
 * Created on 10 febbraio 2015, 15.06
 */

#ifndef CALTYPES_H
#define	CALTYPES_H

#ifdef	__cplusplus
extern "C" {
#endif

//using non-portable definitions because boost/cstdint.hpp includes does not 
//    compile

typedef unsigned long int UInt64;
typedef long int        Int64;
typedef unsigned int    UInt32;		
typedef int             Int32;		
typedef unsigned short  UInt16;		
typedef short           Int16;		
typedef	unsigned char   UInt8;		
typedef char            Int8;		


#ifdef	__cplusplus
}
#endif

#endif	/* CALTYPES_H */

