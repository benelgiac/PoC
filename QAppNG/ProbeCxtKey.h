#if !defined ( ProbeCxtKey_H_NG )
#define ProbeCxtKey_H_NG
// ===========================================================================
/// @file
/// @brief This file contains Probe Context Key class definition.
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
#include <QAppNG/ProtCxtKey.h>
#include "CNlPdu.h"


namespace QAppNG
{
  /*  ...Class definitions
  */
  
  class CProbeCxtKey : public CProtCxtKey
      {
  public:
  
      /// @brief Constructor
      ///
      CProbeCxtKey ( );
  
      /// @brief Constructor
      ///
      CProbeCxtKey
          (
          NL_ATM_PHYS_INFO *  a_atm_phys,
          UInt32              a_sec,
          UInt32              a_nsec,
          UInt8               a_probe_idx,
          UInt8               a_probe_port
          );
  
          CProbeCxtKey
          (
          NL_ATM_PHYS_INFO *  a_atm_phys,
          NL_IP_PHYS_INFO *   a_ip_phys,
          UInt32              a_sec,
          UInt32              a_nsec,
          UInt8               a_probe_idx,
          UInt8               a_probe_port
          );
  
      /// @brief Destructor
      virtual ~CProbeCxtKey ( );
  
      /// @brief getModeType
      ///
      /// This method ...
      /// @retval     enumCxtKeyType
      enumCxtKeyType          getModeType ( );
  
      /// @brief getSequenceNumber
      ///
      /// This method ...
      /// @retval     int
      int                     getSequenceNumber ( );
  
      /// @brief getSec
      ///
      /// This method ...
      /// @retval     Int32
      Int32                   getSec ( );
  
      /// @brief getNSec
      ///
      /// This method ...
      /// @retval     Int32
      Int32                   getNSec ( );
  
  
  
      NL_ATM_PHYS_INFO        m_atm_phys;
      NL_IP_PHYS_INFO         m_ip_phys;
      UInt8                   m_probe_idx;
      UInt8                   m_probe_port;
      Int32                   m_pdu_touch_time_sec;
      Int32                   m_pdu_touch_time_nsec;
      };
}
#include "detail/ProbeCxtKeyImpl.h"

#endif // ProbeCxtKey_H_NG

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  ...End of file */

