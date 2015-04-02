/*===============================================================
 *
 *    Copyright: (C) 2005-2006  CommProve Ltd.
 *    Filename: nl_consumable.h
 *    Project: Netledge / RPS
 *    Description: Defines the base class for the generic structure 
 *              that is passed into the consumer queue.
 *    Author: Aidan Kenny
 *    Created: 28/09/06
 *    Document: 
 *  
 *    Notes:
 * 
 * 
 *    History:
 *
 ================================================================*/

#ifndef __NL_CONSUMABLE_H___NG
#define __NL_CONSUMABLE_H___NG

#include <cal/threadpool/Consumable.h>

namespace QAppNG
{
    typedef enum
    {
       eFRAME_CONTAINER_CONSUMABLE,
       eRECORDER_CONSUMABLE,
       ePDU_CONSUMABLE,
       eIUPS_STACK_DECODER_REFRESH_COMSUMABLE,
       eIUPS_USERID_MESSAGE_COMSUMABLE,
       eIUPS_USER_CXT_REFRESH_CONSUMABLE,
       eIUPS_USER_CXT_CELLID_INFO_CONSUMABLE,
       eIUPS_USER_CXT_RABID_INFO_CONSUMABLE,
       eIUPS_PDP_CXT_REFRESH_COMSUMABLE,
       eIUPS_PDP_CXT_MESSAGE_CONSUMABLE,
       eSM_TICKET_CONSUMABLE,
       eRETMGR_SEQ_NUM_CONSUMABLE,
       eIUB_NODEB_CONFIG,
       eXDRTICKETFIELDS_CONSUMABLE,
       eTSPROCEDURE_CONSUMABLE,
       eCELL_AND_USER_ID_OBJECT_PDU,
       eCELL_AND_USER_ID_OBJECT_IUPROC,
       eCELL_AND_USER_ID_OBJECT_IUBPROC,
       eGB_UNUSED_CONTEXT_ID_CONSUMABLE,
       eIUB_USER_CONTEXT_EVENT_CONSUMABLE,
       ePROTOCOL_EVENT_CONSUMABLE,
    } EConsumableType;

    typedef cal::threadpool::Consumable<EConsumableType> CNlConsumable;


}
#endif // __NL_CONSUMABLE_H___NG

