// ===========================================================================
/// @file
/// @brief This file contains...
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
#if !defined ( ProbeReasmMgrImpl_H_NG )
#define ProbeReasmMgrImpl_H_NG


/*  ...Include files
*/


namespace QAppNG
{

    /*  ...Class definitions
    */

    inline CProbeReasmMgr::CProbeReasmMgr ( UInt32 /*config_flags*/ )
        {
        }

    inline  CProbeReasmMgr::CProbeReasmMgr ( )
        {
        }

    inline CProbeReasmMgr::~CProbeReasmMgr ( )
        {
        }

    inline bool CProbeReasmMgr::IsContextual
        (
        UInt8 *         /*pudata*/,
        UInt16          /*pudatalen*/,
        std::shared_ptr<CProtCxtKey> /*key*/
        )
        {
        return ( true );
        }


}
#endif // ProbeReasmMgrImpl_H_NG

    /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    //  ...End of file */



