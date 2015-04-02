// ===========================================================================
/// @file
/// @brief This file contains the inline ProbeReasmCxt class methods
///
/// Detailed information would go here, whatever might be appropriate for
/// the file.
///
/// @copyright
/// @history
/// REF#        Who              When        What
/// 555         Steve Mellon     16-May-2007 Original development
/// @endhistory
///
// ===========================================================================
#if !defined ( ProbeReasmCxtImpl_H_NG )
#define ProbeReasmCxtImpl_H_NG

/*  ...Include files
*/

namespace QAppNG
{

    /*  ...Class definitions
    */

    inline CProbeReasmCxt::CProbeReasmCxt ( UInt32 config_flags ) :
        CReasmCxt ( config_flags )
        {
        }

    inline CProbeReasmCxt::~CProbeReasmCxt ( )
        {
        CSegment *      pNode;

        while (( pNode = m_reassembly_list.RemoveFirst ( )) != NULL)
            delete pNode;

        }

    inline std::shared_ptr<CNlPdu>  CProbeReasmCxt::Flush ( )
        {
        return ( std::shared_ptr<CNlPdu>()  );
        }


}
#endif // ProbeReasmCxtImpl_H_NG

    /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    //  ...End of file */



