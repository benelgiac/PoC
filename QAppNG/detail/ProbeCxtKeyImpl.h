#if !defined ( ProbeCxtKeyImpl_H_NG )
#define ProbeCxtKeyImpl_H_NG

namespace QAppNG
{
    // ===========================================================================
    /// @file
    /// @brief This file contains the inline methods for the ProbeCxtKey class
    ///
    /// Detailed information would go here, whatever might be appropriate for
    /// the file.
    ///
    /// @copyright
    /// @history
    /// REF#        Who              When        What
    ///                              ???         Original development
    /// 555         Steve Mellon     16-May-2007 Ported to Quantiqa
    /// @endhistory
    ///
    // ===========================================================================

    /*  ...Include files
    */

    /*  ...Class definitions
    */
    inline CProbeCxtKey::CProbeCxtKey ( )
        {
        };

    inline CProbeCxtKey::CProbeCxtKey
        (
        NL_ATM_PHYS_INFO *      in_atm_phys,
        UInt32                  in_sec,
        UInt32                  in_nsec,
        UInt8                   in_probe_idx,
        UInt8                   in_probe_port
        ) :
        m_atm_phys ( *in_atm_phys ),
        m_probe_idx ( in_probe_idx ),
        m_probe_port ( in_probe_port ),
        m_pdu_touch_time_sec ( in_sec ),
        m_pdu_touch_time_nsec ( in_nsec )
        {
        };

        inline CProbeCxtKey::CProbeCxtKey
        (
        NL_ATM_PHYS_INFO *      in_atm_phys,
        NL_IP_PHYS_INFO *       in_ip_phys,
        UInt32                  in_sec,
        UInt32                  in_nsec,
        UInt8                   in_probe_idx,
        UInt8                   in_probe_port
        ) :
        m_atm_phys ( *in_atm_phys ),
        m_ip_phys  (*in_ip_phys),
        m_probe_idx ( in_probe_idx ),
        m_probe_port ( in_probe_port ),
        m_pdu_touch_time_sec ( in_sec ),
        m_pdu_touch_time_nsec ( in_nsec )
        {
        };

    inline CProbeCxtKey::~CProbeCxtKey ( )
        {
        }

    inline enumCxtKeyType CProbeCxtKey::getModeType ( )
        {
        return ( TYPE_SEGMENT );
        };

    inline int CProbeCxtKey::getSequenceNumber ( )
        {
        return ( 0 );
        };

    inline Int32 CProbeCxtKey::getSec ( )
        {
        return ( m_pdu_touch_time_sec );
        }

    inline Int32 CProbeCxtKey::getNSec ( )
        {
        return ( m_pdu_touch_time_nsec );
        }


}
#endif // ProbeCxtKeyImpl_H_NG

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  ...End of file */


