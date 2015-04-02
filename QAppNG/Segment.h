#if !defined ( Segment_H_NG )
#define Segment_H_NG
// ===========================================================================
/// @file
/// @brief This file contains the Segment class definition
///
/// The segment class is the basic unit of information used in probe pdu
/// reassembly.
///
/// @copyright
/// @history
/// REF#        Who              When        What
/// 555         Steve Mellon     16-May-2007 Ported to Quantiqa
/// @endhistory
///
// ===========================================================================

/*  ...Include files
*/
#include    <cal/DList.h>
#include <QAppNG/ProtCxtKey.h>
#include    <CNlPdu.h>

namespace QAppNG
{
    /*  ...Class definitions
    */
    /// @brief CSegment
    class CSegment : public cal::DNode <CSegment>
        {
    public:
        /// @brief Constructor
        ///
        /// The default constructor contains 4 parameters.
        /// @param[in]  a_pdu
        /// @param[in]  a_key
        /// @param[in]  a_payloadbitoffset
        /// @param[in]  a_payload_bytelen
        CSegment
            (
            std::shared_ptr<CNlPdu>&         a_pdu,
            std::shared_ptr<CProtCxtKey>   a_key,
            UInt16          a_payloadbitoffset,
            UInt16          a_payload_bytelen
            );
    
        /// @brief Destructor
        virtual ~CSegment ( );
    
        std::shared_ptr<CNlPdu>             m_pdu;
        std::shared_ptr<CProtCxtKey>        m_key;
        UInt16              m_payloadbitoffset;
        UInt16              m_payloadbytelen;
        bool                m_start_found;
        };
}

#include    "detail/SegmentImpl.h"

#endif // Segment_H_NG

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  ...End of file */



