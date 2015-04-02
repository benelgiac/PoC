// ===========================================================================
/// @file
/// @brief This file contains the inline ReasmMgrIface methods
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
#if !defined ( ReasmMgrIfaceImpl_H_NG )
#define ReasmMgrIfaceImpl_H_NG


/*  ...Include files
*/


namespace QAppNG
{

    /*  ...Class definitions
    */

    template <class REASSEMCXT>
    CReasmMgrIface<REASSEMCXT>::PuNode::PuNode ( std::shared_ptr<CNlPdu>&  a_pdu )
        : m_pdu ( a_pdu )
        {
        }


    template <class REASSEMCXT>
    CReasmMgrIface<REASSEMCXT>::CReasmMgrIface ( UInt32 config_flags )
        : m_config_flags ( config_flags ),
        m_puList ( )
        {
        }

    template <class REASSEMCXT>
    CReasmMgrIface<REASSEMCXT>::CReasmMgrIface ( )
        : m_config_flags ( 0 ),
        m_puList ( )
        {
        }

    template <class REASSEMCXT>
    CReasmMgrIface<REASSEMCXT>::~CReasmMgrIface ( )
        {
        PuNode *      pNode;
      
        while (( pNode = m_puList.RemoveFirst ( )) != NULL)
            {
            delete pNode;
            }
        }

    template <class REASSEMCXT>
    void CReasmMgrIface<REASSEMCXT>::AddPUToQueue ( std::shared_ptr<CNlPdu>&  punit )
        {
        m_puList.InsertLast ( new PuNode ( punit ));
        }

    template <class REASSEMCXT>
    int CReasmMgrIface<REASSEMCXT>::AddPduToPU(std::shared_ptr<CNlPdu>& pduhdl, UInt16 pudata_bit_offset, 
        UInt16 pudata_byte_len, std::shared_ptr<CProtCxtKey> key)
        {
        Int32 rv = -1;

        std::list< std::shared_ptr<CNlPdu> > reasem_list;

        if (IsContextual(NULL, pudata_byte_len, key))
            {
            std::shared_ptr<CReasmCxt> cxt = GetContext(((UInt8 *)&(pduhdl->m_interface_type)), pudata_byte_len, key);
            if(cxt)
                {
                reasem_list = cxt->ProcessPdu(pduhdl, pudata_bit_offset, pudata_byte_len, key);
                }
            }

        std::list< std::shared_ptr<CNlPdu> >::iterator reasem_list_itr;
        for (reasem_list_itr = reasem_list.begin(); 
            reasem_list_itr != reasem_list.end(); reasem_list_itr++)
            {
            AddPUToQueue((*reasem_list_itr));
            rv = 0;
            }

        return rv;
        }


    template <class REASSEMCXT>
    std::shared_ptr<CNlPdu> CReasmMgrIface<REASSEMCXT>::RetrvPU ()
        {
        std::shared_ptr<CNlPdu>           pPdu;
        PuNode *            pNode = NULL;

        if (( pNode = m_puList.RemoveFirst ( )) != NULL )
            {
            pPdu = pNode->m_pdu;
            delete pNode;
            }

        return pPdu;
        }

}
#endif // ReasmMgrIfaceImpl_H_NG

    /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    //  ...End of file */



