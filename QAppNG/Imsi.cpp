// ===========================================================================
/// @file
/// @brief This file contains the implementation of the CImsi class. This class
///  was created in order to deal with the multitude of different ways in which
///  we receive and require IMSIs. Now that it has been created, this class should
///  be the only way we store IMSIs in code.
///  There are functions to set and retrieve the IMSI in various different formats
///  and these should be the only entry (e.g. from a PDU) or exit (e.g. to a ticket)
///  point for IMSI data into or out of our system. If a format is needed which is
///  not covered then appropriate functions should be added to this class, rather
///  than doing a conversion elsewhere in code.
///
/// @copyright
/// @history
/// REF#        Who              When        What
/// 2530        Paul Flynn   October-2008 Original development
/// @endhistory
///
// ===========================================================================

#include <QAppNG/Imsi.h>
#include "assert.h"
#include <cstring>

#define EVEN_DIGIT_IMSI 1
#define ODD_DIGIT_IMSI 9

namespace QAppNG
{

    /// @brief Constructor
    ///
    /// Sets m_numDigits to 0
    CImsi::CImsi()
        : m_numDigits(0)
    {
    }

    /// @brief Destructor
    CImsi::~CImsi()
    {
    }

    /// @brief SetUInt64
    ///
    /// Sets the IMSI from a UInt64 representing the IMSI
    ///
    /// @param[in] imsi64 the IMSI as a UInt64
    /// @return true if the data was set correctly
    bool CImsi::SetUInt64(UInt64 imsi64)
    {
        UInt8 digitBuffer[15];
        int i;

        for (i = 14; i >= 0; i--)
        {
            digitBuffer[i] = static_cast<UInt8>(imsi64 % 10);
            imsi64 /= 10;
            if (imsi64 == 0)
            {
                break;
            }
        }

        SetDigitBuffer(digitBuffer + i, 15 - i);
        
        return true;
    }

    /// @brief Set24008Format
    ///
    /// Sets the IMSI data in the format specified in 3GPP spec 24.008.
    /// This format is used by a number of protocols, including MM, GMM,
    /// SM, CC, BSSGP and BSSMAP.
    ///
    /// @param[in] buf* buffer containing the IMSI data
    /// @param[in] len length of the buffer
    /// @return true if the data was set correctly
    bool CImsi::Set24008Format(UInt8 *buf, UInt8 len)
    {
        // the first half octet contains the TOI and the odd/even indicator
        UInt8 firstHalfOctet = (*buf & 0x0F);

        if (firstHalfOctet == EVEN_DIGIT_IMSI)
        {
            if (len-1 > MAX_IMSI_DIGIT_BUFFER_LEN)
            {
                // Not a valid IMSI
                Reset();
                return false;
            }
            m_numDigits = (len-1) * 2;
            for (UInt8 i = 0; i < (len-1); i++)
            {
                m_digits[i] = (buf[i] >> 4) | (buf[i+1] << 4);
            }
            return true;
        }
        else if (firstHalfOctet == ODD_DIGIT_IMSI)
        {
            if (len > MAX_IMSI_DIGIT_BUFFER_LEN)
            {
                // Not a valid IMSI
                Reset();
                return false;
            }
            m_numDigits = (len * 2) - 1;
            for (UInt8 i = 0; i < (len-1); i++)
            {
                m_digits[i] = (buf[i] >> 4) | (buf[i+1] << 4);
            }
            m_digits[len-1] = (buf[len-1] >> 4) | 0xF0;
            return true;
        }
        else
        {
            // not an IMSI
            Reset();
            return false;
        }
    }

    /// @brief SetRanapFormat
    ///
    /// Sets the IMSI data in the format used by the RANAP protocol.
    /// This format is also used by a number of other protocols, including MAP.
    ///
    /// @param[in] buf* buffer containing the IMSI data
    /// @param[in] len length of the buffer
    /// @return true if the data was set correctly
    bool CImsi::SetRanapFormat(UInt8 *buf, UInt8 len)
    {
        if (len > MAX_IMSI_DIGIT_BUFFER_LEN)
        {
            // Not a valid IMSI
            Reset();
            return false;
        }

        memcpy(m_digits, buf, len);
        if ((m_digits[len-1] & 0xF0) == 0xF0)
        {
            m_numDigits = (2 * len) - 1;
        }
        else
        {
            m_numDigits = (2 * len);
        }
        return true;
    }

    /// @brief SetDigitBuffer
    ///
    /// Sets the IMSI data from a buffer of digits, which represent the IMSI.
    /// This format is used by a number of other protocols, including RRC.
    ///
    /// @param[in] buf* buffer containing the IMSI data
    /// @param[in] len length of the buffer
    /// @return true if the data was set correctly
    bool CImsi::SetDigitBuffer(UInt8 *buf, UInt8 a_numDigits)
    {
        m_numDigits = a_numDigits;
        UInt8 len = a_numDigits / 2;

        if (len > MAX_IMSI_DIGIT_BUFFER_LEN)
        {
            // Not a valid IMSI
            Reset();
            return false;
        }

        for (UInt8 i = 0; i < len; i++)
        {
            m_digits[i] = buf[2*i] | (buf[(2*i)+1] << 4);
        }
        // odd number of digits
        if ((a_numDigits & 0x01) == 1)
        {
            m_digits[len] = buf[a_numDigits-1] | 0xF0;
        }
        return true;
    }

    /// @brief GetUInt64
    ///
    /// Retrieves the IMSI data as a UInt64 representing the IMSI
    ///
    /// @param[out] ret the IMSI as a UInt64
    /// @return true if the data was retrieved correctly
    bool CImsi::GetUInt64(UInt64 &ret) const
    {
        ret = 0;
        for (UInt8 i = 1; i <= m_numDigits; i++)
        {
            ret *= 10;
            ret += GetDigit(i);
        }
        return true;
    }

    /// @brief GetHumanReadable
    ///
    /// Similar to GetUInt64 but easier to use, as 
    ///   - the bool returned from GetUInt64 is extra (always true)
    ///   - no temporary UInt64 placeholder is needed in order to call
    /// Retrieves the IMSI data as a UInt64 representing the IMSI
    ///
    /// @return the IMSI as a human readable (in UInt64) as opposed to Key()
    UInt64 CImsi::GetHumanReadable() const
    {
        UInt64 ret = 0;
        for (UInt8 i = 1; i <= m_numDigits; i++)
        {
            ret *= 10;
            ret += GetDigit(i);
        }
        return ret;
    }

    /// @brief GetMccMncRes
    ///
    /// Retrieves the IMSI data broken into MCC, MNC and rest
    ///
    /// @param[out] mcc the mobile country code
    /// @param[out] mnc the mobile network code
    /// @param[out] res the rest of the IMSI
    /// @return true if the data was retrieved correctly
    bool CImsi::GetMccMncRes(UInt16 &mcc, UInt16 &mnc, UInt64 &res)
    {
        mcc = (GetDigit(1) * 100) + (GetDigit(2) * 10) + GetDigit(3);

        // RIK & ADV the MNC was completly wrong
        /*
        if (IsThreeDigitMncs(mcc))
        {
            mnc = (GetDigit(4) * 100) + (GetDigit(5) * 10) + GetDigit(6);
            res = 0;
            for (UInt8 i = 7; i <= m_numDigits; i++)
            {
                res *= 10;
                res += GetDigit(i);
            }
        }
        else
        {
            mnc = (GetDigit(4) * 10) + GetDigit(5);
            res = 0;
            for (UInt8 i = 6; i <= m_numDigits; i++)
            {
                res *= 10;
                res += GetDigit(i);
            }
        }
        */
        if ( IsThreeDigitMncs(mcc) )
        {
            mnc = (GetDigit(4) * 100) + (GetDigit(5) * 10) + GetDigit(6);
            res = 0;
            for (UInt8 i = 7; i <= m_numDigits; i++)
            {
                res *= 10;
                res += GetDigit(i);
            }
        }
        else
        {
            mnc = (GetDigit(4) * 10) + GetDigit(5);
            res = 0;
            for (UInt8 i = 6; i <= m_numDigits; i++)
            {
                res *= 10;
                res += GetDigit(i);
            }
        }

        return true;
    }

    /// @brief Get24008Format
    ///
    /// Retrieves the IMSI data in the format specified in 3GPP spec 24.008.
    /// This format is used by a number of protocols, including MM, GMM,
    /// SM, CC, BSSGP and BSSMAP.
    ///
    /// @param[in] buf* address of buffer in which the IMSI data will be written
    /// @param[out] len length of the buffer
    /// @return true if the data was retrieved correctly
    bool CImsi::Get24008Format(UInt8 *buf, UInt8 &len)
    {
        // even number of digits
        if ((m_numDigits & 0x01) == 0)
        {
            len = (m_numDigits / 2) + 1;
            buf[0] = (m_digits[0] << 4) | EVEN_DIGIT_IMSI;
            for (UInt8 i = 1; i < (len-1); i++)
            {
                buf[i] = (m_digits[i-1] >> 4) | (m_digits[i] << 4);
            }
            buf[len-1] = (m_digits[len-2] >> 4) | 0xF0;
        }
        // odd number of digits
        else
        {
            len = (m_numDigits+1) / 2;
            buf[0] = (m_digits[0] << 4) | ODD_DIGIT_IMSI;
            for (UInt8 i = 1; i < len; i++)
            {
                buf[i] = (m_digits[i-1] >> 4) | (m_digits[i] << 4);
            }
        }
        return true;
    }

    /// @brief GetRanapFormat
    ///
    /// Retrieves the IMSI data in the format used by the RANAP protocol.
    /// This format is also used by a number of other protocols, including MAP.
    ///
    /// @param[in] buf* address of buffer in which the IMSI data will be written
    /// @param[out] len length of the buffer
    /// @return true if the data was retrieved correctly
    bool CImsi::GetRanapFormat(UInt8 *buf, UInt8 &len)
    {
        len = (m_numDigits + 1) / 2;
        memcpy(buf, m_digits, len);
        return true;
    }

    /// @brief Equality Operator
    ///
    /// @return true iff the object represents the same IMSI as this does
    bool CImsi::operator==(const CImsi& toCompare) const
    {
        if ( (m_numDigits == toCompare.m_numDigits) && ((memcmp(m_digits, toCompare.m_digits, (m_numDigits+1)/2) == 0)) )
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    // Return true if IMSIs are different
    bool CImsi::operator!=(const CImsi& toCompare) const
    {
        if ( (m_numDigits != toCompare.m_numDigits) || ((memcmp(m_digits, toCompare.m_digits, (m_numDigits+1)/2) != 0)) )
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    /// @brief GetDigit
    ///
    /// Get a particular digit of the IMSI. E.g. GetDigit(4) returns the fourth digit.
    ///
    /// @param[in] digit_num the position of the digit to return,
    ///     note the first digit is 1, not 0.
    /// @return the value of the digit
    UInt8 CImsi::GetDigit(UInt8 digit_num) const
    {
        if ((digit_num > m_numDigits) || (digit_num == 0))
        {
            return 0xFF;
        }
        else
        {
            // odd number digit
            if ((digit_num & 0x01) == 1)
            {
                return m_digits[digit_num / 2] & 0x0F;
            }
            // even number digit
            else
            {
                return m_digits[(digit_num-1)/2] >> 4;
            }
        }
    }

    /// @brief DisableIdentity
    ///
    /// @return an IMSI which retains the MCC and MNC from *this but with the rest set to 0.
    CImsi CImsi::DisableIdentity()
    {
        CImsi ret = *this;
    
        //3 leaves mcc and mnc (and possibly first digit of res) intact
        memset(&(ret.m_digits[3]), 0, 5);
        return ret;
    }

    /// @brief isValid
    ///
    /// @return true iff the IMSI represented by this is a valid IMSI
    bool CImsi::isValid()
    {
        // PJF This is possibly overkill for some uses of this function. Perhaps there should be 
        // another function, e.g. isSet() which returns true if numDigits != 0.
        if ((m_numDigits > 5) && (m_numDigits <= 16))
        {
            for (UInt8 i = 1; i <= m_numDigits; i++)
            {
                if (GetDigit(i) > 9)
                {
                    return false;
                }
            }
            return true;
        }
        else
        {
            return false;
        }
    }

    /// @brief Key
    ///
    ///@return a UInt64 unique to this IMSI to be used as a key for maps// Returns a UInt64 
    UInt64 CImsi::Key()
    {
        UInt8 len = (m_numDigits+1) / 2;
        UInt64 ret = static_cast<UInt64>(m_digits[0]);
        for (UInt8 i = 1; i < len; i++)
        {
            ret |= (static_cast<UInt64>(m_digits[i])) << (8 * i);
        }
        return ret;
    }

    /// @brief IsThreeDigitMncs
    ///
    /// Performs a lookup to check whether a particular country uses 3-digit MNCs
    ///
    /// @param[in] mcc the Mobile Country Code of the country in question
    /// @return true iff mcc is a country that uses 3 digit MNCs
    bool CImsi::IsThreeDigitMncs(UInt16 &mcc)
    {
        // we tried to get the digit number from odd/even bit but it didn't work
        // so we need a map
        switch(mcc)
        {
        case 302:
        case 310:
        case 311:
        case 312:
        case 313:
        case 314:
        case 315:
        case 316:
        case 338:
        case 342:
        case 344:
        case 346:
        case 348:
        case 352:
        case 358:
        case 360:
        case 364:
        case 365:
        case 722:
        case 732:
        case 890:
            return true;
        default:
            return false;
        }
    }

    /// @brief Reset
    /// 
    /// Sets m_numDigits to 0
    void CImsi::Reset()
    {
        m_numDigits = 0;
    }

}
