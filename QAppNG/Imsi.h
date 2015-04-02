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
/// 3581        A. Della Villa    June-2009 Added GetNumDigit method
/// @endhistory
///
// ===========================================================================

#ifndef IMSI_H_NG
#define IMSI_H_NG

#define MAX_IMSI_DIGIT_BUFFER_LEN 8

#include <QAppNG/core.h>

namespace QAppNG
{

    class CImsi
    {
    public:
        /// @brief Constructor
        ///
        /// Sets m_numDigits to 0
        CImsi();
        /// @brief Destructor
        virtual ~CImsi();

        /// @brief SetUInt64
        ///
        /// Sets the IMSI from a UInt64 representing the IMSI
        ///
        /// @param[in] imsi64 the IMSI as a UInt64
        /// @return true if the data was set correctly
        bool SetUInt64(UInt64 imsi64);

        /// @brief Set24008Format
        ///
        /// Sets the IMSI data in the format specified in 3GPP spec 24.008.
        /// This format is used by a number of protocols, including MM, GMM,
        /// SM, CC, BSSGP and BSSMAP.
        ///
        /// @param[in] buf* buffer containing the IMSI data
        /// @param[in] len length of the buffer
        /// @return true if the data was set correctly
        bool Set24008Format(UInt8 *buf, UInt8 len);

        /// @brief SetRanapFormat
        ///
        /// Sets the IMSI data in the format used by the RANAP protocol.
        /// This format is also used by a number of other protocols, including MAP.
        ///
        /// @param[in] buf* buffer containing the IMSI data
        /// @param[in] len length of the buffer
        /// @return true if the data was set correctly
        bool SetRanapFormat(UInt8 *buf, UInt8 len);
        /// @brief SetDigitBuffer
        ///
        /// Sets the IMSI data from a buffer of digits, which represent the IMSI.
        /// This format is used by a number of other protocols, including RRC.
        ///
        /// @param[in] buf* buffer containing the IMSI data
        /// @param[in] len length of the buffer
        /// @return true if the data was set correctly
        bool SetDigitBuffer(UInt8 *buf, UInt8 len);

        /// @brief GetUInt64
        ///
        /// Retrieves the IMSI data as a UInt64 representing the IMSI
        ///
        /// @param[out] ret the IMSI as a UInt64
        /// @return true if the data was retrieved correctly
        bool GetUInt64(UInt64 &ret) const;
        /// @brief GetHumanReadable
        ///
        /// Similar to GetUInt64 but more practical, as 
        ///   - the returned bool is extra (always true)
        ///   - no temporary placeholder modified by ref is needed in order to call
        /// Retrieves the IMSI data as a UInt64 representing the IMSI
        ///
        /// @return the IMSI as a UInt64
        UInt64 GetHumanReadable() const;
        /// @brief GetMccMncRes
        ///
        /// Retrieves the IMSI data broken into MCC, MNC and rest
        ///
        /// @param[out] mcc the mobile country code
        /// @param[out] mnc the mobile network code
        /// @param[out] res the rest of the IMSI
        /// @return true if the data was retrieved correctly
        bool GetMccMncRes(UInt16 &mcc, UInt16 &mnc, UInt64 &res);
        /// @brief Get24008Format
        ///
        /// Retrieves the IMSI data in the format specified in 3GPP spec 24.008.
        /// This format is used by a number of protocols, including MM, GMM,
        /// SM, CC, BSSGP and BSSMAP.
        ///
        /// @param[in] buf* address of buffer in which the IMSI data will be written
        /// @param[out] len length of the buffer
        /// @return true if the data was retrieved correctly
        bool Get24008Format(UInt8 *buf, UInt8 &len);
        /// @brief GetRanapFormat
        ///
        /// Retrieves the IMSI data in the format used by the RANAP protocol.
        /// This format is also used by a number of other protocols, including MAP.
        ///
        /// @param[in] buf* address of buffer in which the IMSI data will be written
        /// @param[out] len length of the buffer
        /// @return true if the data was retrieved correctly
        bool GetRanapFormat(UInt8 *buf, UInt8 &len);

        /// @brief Equality Operator
        ///
        /// @return true iff the object represents the same IMSI as this does
        bool operator==(const CImsi& toCompare) const;
        /// @brief Inequality Operator
        ///
        /// @return true iff the object does not represent the same IMSI as this does
        bool operator!=(const CImsi& toCompare) const;

        /// @brief GetDigit
        ///
        /// Get a particular digit of the IMSI. E.g. GetDigit(4) returns the fourth digit.
        ///
        /// @param[in] digit_num the position of the digit to return,
        ///     note the first digit is 1, not 0.
        /// @return the value of the digit
        UInt8 GetDigit(UInt8 digit_num) const;

        /// @brief DisableIdentity
        ///
        /// @return an IMSI which retains the MCC and MNC from *this but with the rest set to 0.
        CImsi DisableIdentity();

        /// @brief isValid
        ///
        /// @return true iff the IMSI represented by this is a valid IMSI
        bool isValid();

        /// @brief Key
        ///
        ///@return a UInt64 unique to this IMSI to be used as a key for maps
        UInt64 Key();

        /// @brief IsThreeDigitMncs
        ///
        /// Performs a lookup to check whether a particular country uses 3-digit MNCs
        ///
        /// @param[in] mcc the Mobile Country Code of the country in question
        /// @return true iff mcc is a country that uses 3 digit MNCs
        /// 
        /// RIK & ADV: we modify "bool IsThreeDigitMncs(UInt16 mcc)" removing mcc from the signature
        /// the idea of Paul was to implement an hash table with MCC as key to understand if 2-digit
        /// or 3-digit MNC is used (so with respect to the country we should know the number of digits.
        /// We instead tried to undertand the number of digits from the ODD/EVEN field.
        bool IsThreeDigitMncs(UInt16 &mcc);

        /// @brief Reset
        /// 
        /// Sets m_numDigits to 0
        void Reset();

        /// @brief GetNumDigit
        /// 
        /// Return the number of Digit of the IMSI; if 0 IMSI is not set
        UInt8 GetNumDigit() {return m_numDigits;};

    protected:
        UInt8 m_numDigits;
        UInt8 m_digits[MAX_IMSI_DIGIT_BUFFER_LEN];
    };

}
#endif
