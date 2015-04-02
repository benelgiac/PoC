#ifndef __128BIT_STLMAPKEY_H___NG
#define __128BIT_STLMAPKEY_H___NG
// ===========================================================================
/// @file
/// @brief This file contains 128BitMapKey definition
///
/// Detailed information would go here, whatever might be appropriate for
/// the file.
///
/// @copyright
/// @history
/// REF#        Who              When        What
///             ???              ???         Original
/// 555         Steve Mellon     17-May-2007 added C128BitMapKeyEqual and C128BitMapKeyHash
///                                          for HashMap usage
/// @endhistory
///
// ===========================================================================


#include <stdio.h>
#include <memory.h>

#include "netledge_types.h"

#include "core.h"

namespace QAppNG
{


    typedef std::pair<UInt64, UInt64> C128BitMapKey;

    typedef UInt64 C64BitMapKey;

    typedef UInt32 C32BitMapKey;

    inline UInt16 make16bit(const UInt8 first, const UInt8 second) { return (UInt16(first) << 8) | second; }
    inline UInt32 make32bit(const UInt16 first, const UInt16 second) { return (UInt32(first) << 16) | second; }
    inline UInt32 make32bit(const UInt16 first, const UInt8 second, const UInt8 third) { return make32bit(first, make16bit(second, third)); }
    inline UInt32 make32bit(const UInt8 first, const UInt8 second, const UInt8 third, const UInt8 fourth) { return make32bit( make16bit(first, second), make16bit(third, fourth) ); }
    inline UInt64 make64bit(const UInt32 first, const UInt32 second) { return (UInt64(first) << 32) | second; }
    inline UInt64 make64bit(const UInt32 first, const UInt16 second, const UInt16 third) { return make64bit(first, make32bit(second, third)); }
    inline UInt64 make64bit(const UInt16 first, const UInt16 second, const UInt16 third, const UInt16 fourth) { return make64bit( make32bit(first, second), make32bit(third, fourth) ); }
    inline C128BitMapKey make128bit(const UInt64 first, const UInt64 second) { return C128BitMapKey(first, second); }
    inline C128BitMapKey make128bit(const UInt64 first, const UInt32 second, const UInt32 third) { return make128bit(first, make64bit(second, third)); }
    inline C128BitMapKey make128bit(const UInt32 first, const UInt32 second, const UInt32 third, const UInt32 fourth) { return make128bit(make64bit(first, second), make64bit(third, fourth)); }
}
#endif
