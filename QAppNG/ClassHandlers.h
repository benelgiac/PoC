//
// C++ Interface: ClassHandler
//
// Description:
//
//
// Author: 
//
//
//
#ifndef INCLUDE_CLASSHANDLERS_NG
#define INCLUDE_CLASSHANDLERS_NG

// Include CAL files
#include "cal/defs.h"

// Include STL & BOOST
#include <map>
#include <string>
#include <vector>

namespace QAppNG
{

    // using namespaces and classes
    using std::map;
    using std::string;
    using std::vector;

    // --------------------------------------------------------------------------------------------------------

    class UInt64Handler
    {
    public:
        template<class UINT_CLASS>
        UINT_CLASS get(const UInt64& uint, size_t right_shift) 
        { 
            return UINT_CLASS(uint >> right_shift);
        };

        template<class UINT_CLASS>
        void set (UInt64& uint, UINT_CLASS data, size_t left_shift) 
        {
            UInt64 casted_data = UInt64(data);
            casted_data << left_shift;
            uint &= casted_data;
        };
    private:
    };


    // --------------------------------------------------------------------------------------------------------

}
#endif //CLASSHANDLERS

