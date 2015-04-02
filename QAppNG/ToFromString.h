#if !defined ( ToFromString_H_NG )
#define ToFromString_H_NG

#include <sstream>
#include <boost/regex.hpp>
#include <boost/foreach.hpp>

namespace QAppNG
{

    template <class T>
    inline std::string to_string (const T& t)
    {
        std::stringstream ss;
        ss << t;
        return ss.str();
    }

    inline std::string ipv4_toString (UInt32 ip)
    {
        std::stringstream ss;
        ss << ((ip >> 24) & 0xFF) << "."
           << ((ip >> 16) & 0xFF) << "."
           << ((ip >> 8)  & 0xFF) << "."
           << ((ip >> 0)  & 0xFF);
        return ss.str();
    }

    inline bool ipv4_fromString(std::string& s, UInt32& ipv4)
    {
        bool rv = false;
        // expected format is "192.168.1.1"
        boost::regex re("(\\d+)\\.(\\d+)\\.(\\d+)\\.(\\d+)");
        boost::smatch match;
        if (boost::regex_match(s, match, re))
        {
            UInt32 d1 = std::atoi(match[1].str().c_str());
            UInt32 d2 = std::atoi(match[2].str().c_str());
            UInt32 d3 = std::atoi(match[3].str().c_str());
            UInt32 d4 = std::atoi(match[4].str().c_str());
            if ((d1 < 256) && (d2 < 256) && (d3 < 256) && (d4 < 256))
            {
                ipv4 = (d1<<24) | (d2<<16) | (d3<<8) | d4;

                rv = true;
            }
        }
        return rv;
    }

}
#endif