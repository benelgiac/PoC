// ===========================================================================
/// @file
/// @brief This file contains the implementation of tickets common methods
///
/// @copyright
/// @history
/// REF#        Who                 When            What
/// 5886        F.Gragnani          28-March-2011   Original Development
/// @endhistory
///
// ===========================================================================

#include <QAppNG/TicketsCommonMethods.h>
#include <iostream>

#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

namespace QAppNG
{
    std::vector<UInt8> TicketsCommonMethods::ConvertRawNumbergToVector(unsigned char* number)
    {
        std::string string_number = std::string((const char*)number);
        return ConvertRawStringToVector(string_number);
    }

    std::vector<UInt8> TicketsCommonMethods::ConvertRawStringToVector(std::string string_number)
    {
        std::vector<UInt8> vect_number(string_number.begin(), string_number.end());
        return vect_number;
    }

    void TicketsCommonMethods::CleanAndConvertStringNumber( std::string &string_number, std::vector<UInt8> &vector_number )
    {
        size_t i;
        std::ostringstream tmpstream;
        tmpstream.clear();tmpstream.str("");
        for (i=0; i<string_number.size(); i++) 
        { 
            if(isdigit(static_cast<char>(string_number[i])))
            {
                vector_number.push_back(string_number[i]); 
                tmpstream << string_number[i];
            }
        }
        string_number = tmpstream.str();
    }

    void TicketsCommonMethods::CleanAndConvertVectorToString(std::vector<UInt8> vector_number, std::string &string_number)
    {
        size_t i;
        std::ostringstream tmpstream;
        tmpstream.clear(); tmpstream.str("");
        for (i = 0; i < vector_number.size(); i++)
        {
            if (isdigit(static_cast<char>(vector_number[i])))
            {
                tmpstream << vector_number[i];
            }
        }
        string_number = tmpstream.str();
    }

    void TicketsCommonMethods::CleanStringNumber( std::string &string_number)
    {
        size_t i;
        std::ostringstream tmpstream;
        tmpstream.clear();tmpstream.str("");
        for (i=0; i<string_number.size(); i++) 
        { 
            if(isdigit(string_number[i]))
            {
                tmpstream << string_number[i];
            }
        }
        string_number = tmpstream.str();
    }

    void TicketsCommonMethods::ConvertVectorNumber( UInt64 &number, std::vector<UInt8> &vector_number )
    {
        std::ostringstream tmpstream;
        tmpstream.clear();tmpstream.str("");
        std::vector<UInt8>::iterator it=vector_number.begin();
        while (it!=vector_number.end())
        {
            if(isdigit(*it))
            {
                tmpstream << *it;
            }
            ++it;
        }

        number = atoll(tmpstream.str().c_str());
    }

    std::string TicketsCommonMethods::NormalizeNationalNumber( std::string string_number, UInt16 international_prefix )
    {
        std::ostringstream norm_number_stream;
        norm_number_stream << international_prefix << string_number;
    
        return norm_number_stream.str();
    }

    std::string TicketsCommonMethods::ConvertIPtoString( const UInt32 &ip )
    {
        struct in_addr addr;
        addr.s_addr = ip;
        return inet_ntoa(addr);
    }

}
