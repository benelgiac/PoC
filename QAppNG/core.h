#ifndef INCLUDE_ECUBACORE_NG
#define INCLUDE_ECUBACORE_NG

// --------------------------------------------------------------------------------------------------------

// Basic Types
#include <cstdint>
#include <string>

// Delegates
#include "Delegate/FastDelegate.h"

// Memory Managment
#include <memory>

// Threads
#include <thread>
#include <chrono>
#include <mutex>

// Utility
#include <boost/bind.hpp>

// I/O
#include <boost/asio.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
// --------------------------------------------------------------------------------------------------------

// Macro to remove warning in case of unused function args
#ifdef UNUSED
#elif defined(__GNUC__)
# define UNUSED(x) /*@unused@*/ (void) x
#elif defined(__LCLINT__)
# define UNUSED(x) /*@unused@*/ x
#else
# define UNUSED(x) (void) x
#endif

// --------------------------------------------------------------------------------------------------------

namespace cal
{
enum Status
    {
    Status_Ok   = 0,    ///< Status return is "okay"
    Status_Error= -1    ///< Status return is "bad"
    };
}

typedef cal::Status CalStatus;

    
    /// @brief Bit defines to improve readability
enum BIT_DEFS
    {
    LOW_1_BITS = 0x01,
    LOW_2_BITS = 0x03,
    LOW_3_BITS = 0x07,
    LOW_4_BITS = 0x0F,
    LOW_5_BITS = 0x1F,
    LOW_6_BITS = 0x3F,
    LOW_7_BITS = 0x7F,
    HIGH_1_BITS = 0x80,
    HIGH_2_BITS = 0xC0,
    HIGH_3_BITS = 0xE0,
    HIGH_4_BITS = 0xF0,
    HIGH_5_BITS = 0xF8,
    HIGH_6_BITS = 0xFC,
    HIGH_7_BITS = 0xFE,
    BIT_1 = 0x01,
    BIT_2 = 0x02,
    BIT_3 = 0x04,
    BIT_4 = 0x08,
    BIT_5 = 0x10,
    BIT_6 = 0x20,
    BIT_7 = 0x40,
    BIT_8 = 0x80
    };

// --------------------------------------------------------------------------------------------------------

typedef uint64_t UInt64;
typedef uint32_t UInt32;
typedef uint16_t UInt16;
typedef uint8_t  UInt8;
typedef int64_t  Int64;
typedef int32_t  Int32;
typedef int16_t  Int16;
typedef int8_t   Int8;

// --------------------------------------------------------------------------------------------------------------------

// define hash functions for needed types used as std::unordered_map keys
namespace std
{
    // Hash values concatenating function declarations
    template <typename T>
    inline void hash_concatenate(size_t & seed, const T & t);

    template <typename T, typename... Ts>
    
    inline void hash_concatenate(size_t & seed, const T & t, const Ts & ... ts);
    
    // std::hash specialization for std::pair
    template<typename X, typename Y>
    class hash< std::pair< X, Y> >
    {
    public:
        size_t operator()(const std::pair<X, Y> & key) const
        {
            size_t seed(0);

            hash_concatenate(seed, key.first, key.second);

            return seed;
        }
    };

    template<>
    class hash< boost::asio::ip::address >
    {
    public:
        size_t operator()( const boost::asio::ip::address & key ) const
        {
            size_t seed = 0;

            if (key.is_v4())
            {
                seed = key.to_v4().to_ulong();
            }
            else if (key.is_v6())
            {
                UInt64 firt_sub_result = 0;
                UInt64 second_sub_result = 0;

                boost::asio::ip::address_v6::bytes_type tmp = key.to_v6().to_bytes();

                // IPv6 address consists of 16 bytes
                for (size_t i = 0; i < 8; i += 4)
                {
                    firt_sub_result <<= 32;
                    firt_sub_result += ((tmp[i] << 24) | (tmp[i + 1] << 16) | (tmp[i + 2] << 8) | (tmp[i + 3]));
                }

                for (size_t i = 8; i < 16; i += 4)
                {
                    second_sub_result <<= 32;
                    second_sub_result += ((tmp[i] << 24) | (tmp[i + 1] << 16) | (tmp[i + 2] << 8) | (tmp[i + 3]));
                }

                hash_concatenate(seed, firt_sub_result, second_sub_result);
            }

            return seed;
        }        
    };

    // Hash values combining functions definition
    template <typename T>
    inline void hash_concatenate(size_t & seed, const T & t)
    {
        std::hash<T> hasher;

        seed ^= hasher(t) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    template <typename T, typename... Ts>
    inline void hash_concatenate(size_t & seed, const T & t, const Ts & ... ts)
    {
        hash_concatenate(seed, t);
        hash_concatenate(seed, ts...);
    }

    template<typename X,typename Y,typename W,typename Z>
    class hash< std::tuple< X, Y, W, Z > >
    {
    public:
        size_t operator()(const std::tuple< X, Y, W, Z > & key) const
        {
            size_t seed(0);

            hash_concatenate(seed,std::get<0>(key),std::get<1>(key),std::get<2>(key),std::get<3>(key));

            return seed;
        }
    };
}



namespace QAppNG
{
    //TODO implement
    inline std::string get_process_name()
    {
        return "";
    }
}
#endif // INCLUDE_ECUBACORE_NG


