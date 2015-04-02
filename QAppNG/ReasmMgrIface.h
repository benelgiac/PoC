#if !defined ( reasm_mgr_iface_H_NG )
#define reasm_mgr_iface_H_NG
// ===========================================================================
/// @file
/// @brief This file contains the ReasmMgrIface class definition
///
/// Detailed information would go here, whatever might be appropriate for
/// the file.
///
/// @copyright
/// @history
/// REF#        Who              When        What
///                              ???         Original development
/// 555         Steve Mellon     16-May-2007 ported to quanitqa
/// @endhistory
///
// ===========================================================================

/*  ...Include files
*/

/*  ...Class definitions
*/
#include    <cal/DList.h>
#include    <memory>
#include <QAppNG/ReasmCxtIface.h>
#include <QAppNG/128bit_mapkey.h>
#include <QAppNG/ProtCxtKey.h>
#include    <netledge_types.h>

#include    <CNlPdu.h>

namespace QAppNG
{
    /// @brief CReasmMgrIface
    template <class REASSEMCXT>
    class CReasmMgrIface
        {
    public:
        /// @brief Constructor
        ///
        /// config flags from reasm_config.h
        CReasmMgrIface ( UInt32 config_flags );
    
        /// @brief Destructor
        virtual ~CReasmMgrIface ( );
    
        /// @brief AddPduToPU
        ///
        /// entry point for a fragment or a whole packet into the reassembly mgr
        ///
        /// @param[in]  pduhdl
        /// @param[in]  pudata_offset
        /// @param[in]  pudatalen
        /// @param[in]  key
        /// @retval     int
        virtual int     AddPduToPU
                            (
                            std::shared_ptr<CNlPdu>&         pduhdl,
                            UInt16          pudata_offset,
                            UInt16          pudatalen,
                            std::shared_ptr<CProtCxtKey> key
                            );
    
        /// @brief IsContextual
        ///
        /// only looks at packet to see if it is in any way required for a context,
        /// does not do lookup
        ///
        /// @param[in]  pudata
        /// @param[in]  pudatalen
        /// @param[in]  key
        /// @retval     bool
        virtual bool    IsContextual
                            (
                            UInt8 *         pudata,
                            UInt16          pudatalen,
                            std::shared_ptr<CProtCxtKey>   key
                            ) = 0;
    
        /// @brief GetContext
        ///
        ///
        /// @param[in]  pudata
        /// @param[in]  pudatalen
        /// @param[in]  key
        /// @retval     CReasmCxt
        virtual REASSEMCXT  GetContext
                            (
                            UInt8 *         pudata,
                            UInt16          pudatalen,
                            std::shared_ptr<CProtCxtKey> key
                            ) = 0;
    
        /// @brief GetContext
        ///
        /// Adds a completed PU to the queue for retrieval
        ///
        /// @param[in]  punit
        /// @retval     void
        void            AddPUToQueue ( std::shared_ptr<CNlPdu>&  punit );
    
        //
        /// @brief RetrvPU
        ///
        /// exit point from the reasm mgr, returns the CProtoUnit from the head of the queue
        ///
        /// @retval     CNlPdu
        std::shared_ptr<CNlPdu>         RetrvPU();
    
    
    protected:
        /// @brief Constructor
        CReasmMgrIface ( );
    
        UInt32              m_config_flags;
    
        typedef std::unordered_map <C128BitMapKey,
                              REASSEMCXT>
                                              ContextHashMap;
    
        ContextHashMap      m_context_map;
    
    
    private:
    
        struct PuNode : public cal::DNode <PuNode>
            {
        public:
            PuNode ( std::shared_ptr<CNlPdu>&  pdu );
    
            std::shared_ptr<CNlPdu>             m_pdu;
            };
    
        cal::DList <PuNode>    m_puList;
        };
}
#include "detail/ReasmMgrIfaceImpl.h"

#endif // ReasmMgrIntf_H

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  ...End of file */


