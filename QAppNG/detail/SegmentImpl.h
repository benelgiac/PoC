// ===========================================================================
/// @file
/// @brief This file contains the inline Segment class methods
///
/// Detailed information would go here, whatever might be appropriate for
/// the file.
///
/// @copyright
/// @history
/// REF#        Who              When        What
/// 555         Steve Mellon     16-May-2007 Ported to Quantiqa
/// @endhistory
///
// ===========================================================================
#if !defined ( SegmentImpl_H_NG )
#define SegmentImpl_H_NG

    /*  ...Include files
    */

namespace QAppNG
{
    /*  ...Class definitions
    */

    inline CSegment::CSegment
        (
        std::shared_ptr<CNlPdu>&          a_pdu,
        std::shared_ptr<CProtCxtKey>      a_key,
        UInt16          a_payloadbitoffset,
        UInt16          a_payload_bytelen
        ) :
        m_pdu ( a_pdu ),
        m_key ( a_key ),
        m_payloadbitoffset ( a_payloadbitoffset ),
        m_payloadbytelen ( a_payload_bytelen ),
        m_start_found ( false )
        {
        }

    inline CSegment::~CSegment ( )
        {
        }


}
#endif // SegmentImpl_H_NG

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  ...End of file */



