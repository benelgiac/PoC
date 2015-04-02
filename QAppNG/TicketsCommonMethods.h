// ===========================================================================
/// @file
/// @brief This file contains the declaration of tickets common methods
///
/// @copyright
/// @history
/// REF#        Who                 When            What
/// 5886        F.Gragnani          28-March-2011   Original Development
/// @endhistory
///
    // ===========================================================================
#ifndef INCLUDE_TICKETS_COMMON_METHODS_NG
#define INCLUDE_TICKETS_COMMON_METHODS_NG

/*  ...Include files
*/
#include <QAppNG/core.h>
#include "QAppNG/Singleton.h"

namespace QAppNG
{

    class TicketsCommonMethods : public Singleton<TicketsCommonMethods>
    {
    public:

        TicketsCommonMethods(){};
        virtual ~TicketsCommonMethods(void){};

        enum eTypeOfNumber //   3GPP TS 24.008 version 11.9.0 Release 11 [par 10.5.4.7]
        {
            eTypeOfNumber_unknown = 0,
            eTypeOfNumber_international_number,
            eTypeOfNumber_national_number,
            eTypeOfNumber_network_specific_number,
            eTypeOfNumber_dedicated_access,
            eTypeOfNumber_reserved,
            eTypeOfNumber_reserved_2,
            eTypeOfNumber_reserved_for_extension
        };


        std::vector<UInt8> ConvertRawNumbergToVector(unsigned char* number);
        std::vector<UInt8> ConvertRawStringToVector(std::string string_number);
        void CleanAndConvertStringNumber(std::string &string_number, std::vector<UInt8> &vector_number);
        void CleanAndConvertVectorToString(std::vector<UInt8> vector_number, std::string &string_number);
        void CleanStringNumber( std::string &string_number);
        std::string NormalizeNationalNumber( std::string string_number, UInt16 international_prefix);
        void ConvertVectorNumber( UInt64 &number, std::vector<UInt8> &vector_number );
        std::string ConvertIPtoString( const UInt32 &ip );
        
    private:

    };


}
#endif //INCLUDE_TICKETS_COMMON_METHODS_NG
