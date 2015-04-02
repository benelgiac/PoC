#if !defined ( ReasmCxtIfaceImpl_H_NG )
#define ReasmCxtIfaceImpl_H_NG
// ===========================================================================
/// @file
/// @brief This file contains inline ReasmCxt methods
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
#include    <time.h>

namespace QAppNG
{
    /*  ...Class definitions
    */

    inline CReasmCxt::CReasmCxt ( UInt32 a_config_flags ) :
        m_optflag ( a_config_flags )
        {
        }

    inline CReasmCxt::~CReasmCxt ( )
        {
        }

}
#endif // ReasmCxtIfaceImpl_H_NG

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  ...End of file */


