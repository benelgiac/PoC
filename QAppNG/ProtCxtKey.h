#if !defined ( ProtCxtKey_H_NG )
#define ProtCxtKey_H_NG
// ===========================================================================
/// @file
/// @brief This file contains the ProtCxtKey class definition
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

namespace QAppNG
{
   /*  ...Class definitions
   */
   typedef enum
      {
      TYPE_SEGMENT        = 0,
      TYPE_ACK            = 1
      } enumCxtKeyType;
   
   class CProtCxtKey
      {
   public:
   
      /// @brief Constructor
      CProtCxtKey ( );
   
      /// @brief Destructor
      virtual ~CProtCxtKey ( );
   
      /// @brief getSequenceNumber
      ///
      /// getSequenceNumber desciption
      ///
      /// @retval     int
      virtual int                 getSequenceNumber ( ) = 0;
   
      /// @brief getSec
      ///
      /// getSec desciption
      ///
      /// @retval     Int32
      virtual Int32               getSec ( ) = 0;
   
      /// @brief getNSec
      ///
      /// getNSec desciption
      ///
      /// @retval     Int32
      virtual Int32               getNSec ( ) = 0;
   
      };
}
#include    "detail/ProtCxtKeyImpl.h"

#endif // ProtCxtKey_H_NG

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  ...End of file */

