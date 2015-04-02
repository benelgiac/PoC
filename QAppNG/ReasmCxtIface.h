#if !defined ( ReasmCxtIface_H_NG )
#define ReasmCxtIface_H_NG
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

/*  ...Include files
*/
#include    <memory>
#include "CNlPdu.h"
#include <QAppNG/ProtCxtKey.h>
#include <list>

namespace QAppNG
{
    /*  ...Class definitions
    */
    
    /// @brief CReasmCxt
    
    class CReasmCxt
        {
    public:
        /// @brief Constructor
        CReasmCxt ( UInt32 config_flags );
    
        /// @brief Destructor
        virtual ~CReasmCxt ( );
    
        /// @brief ProcessPdu
        ///
        /// ProcessPdu description.
        ///
        /// @param[in]  a_pduhdl
        /// @param[in]  a_pudata_bit_offset
        /// @param[in]  a_pudata_byte_len
        /// @param[in]  a_key
        /// @retval     std::list< std::shared_ptr<CNlPdu> >
        virtual std::list< std::shared_ptr<CNlPdu> >     ProcessPdu
                                (
                                std::shared_ptr<CNlPdu>&        a_pduhdl,
                                UInt16          a_pudata_bit_offset,
                                UInt16          a_pudata_byte_len,
                                std::shared_ptr<CProtCxtKey> a_key
                                ) = 0;
    
        /// @brief Flush
        ///
        /// Flush description.
        ///
        /// @retval     CNlPdu
        virtual std::shared_ptr<CNlPdu>     Flush ( ) = 0;
    
        UInt32              m_optflag;
    
    protected:
    
        /// @brief Reassemble
        ///
        /// Reassemble description
        ///
        /// @param[in] a_reasm_list  list to add reassembled pdus too
        /// @return void  return description
        virtual void         Reassemble ( std::list< std::shared_ptr<CNlPdu> > &a_reasm_list ) = 0;
    
        };
}
#include    "detail/ReasmCxtIfaceImpl.h"

#endif // ReasmCxtIface_H_NG

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  ...End of file */



