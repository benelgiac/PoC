#ifndef __CELL_AND_USER_IDENTIFIED_OBJECT___NG
#define __CELL_AND_USER_IDENTIFIED_OBJECT___NG
// ===========================================================================
/// @file CellAndUserIdentifiedObject.h
/// @brief This file contains the implementation of a consumable which associate an object and a cellid/userid
///
/// @copyright (C) 2008 CommProve Ltd.
/// @history
/// REF#        Who               When        What
/// 2301       Faustino Frechilla 16/7/08     Original development for Netledge / RPS
/// 2101       David O'Loghlin    10/10/08    Templatised
/// @endhistory
///
// ===========================================================================

#include "CNlPdu.h"
#include <QAppNG/nl_consumable.h>
#include "datatype24008.h"
#include "QAppNG/QObservable.h"
#include <memory>
#include "transport_bearer.h"


#define CELL_AND_USER_IDENTIFIED_OBJECT_DEFAULT_MCC     0xffff
#define CELL_AND_USER_IDENTIFIED_OBJECT_DEFAULT_MNC     0xffff
#define CELL_AND_USER_IDENTIFIED_OBJECT_DEFAULT_LAC     0xffff
#define CELL_AND_USER_IDENTIFIED_OBJECT_DEFAULT_RAC     0xffff
#define CELL_AND_USER_IDENTIFIED_OBJECT_DEFAULT_CI      0xffff
#define CELL_AND_USER_IDENTIFIED_OBJECT_DEFAULT_USERID      0
#define CELL_AND_USER_IDENTIFIED_OBJECT_DEFAULT_OLD_USERID  0xffffffffffffffff

namespace QAppNG
{

    template <typename TConsumable>
    class CellAndUserIdentifiedObject :
        public CNlConsumable,
        public QObservable
    {
    public:
        CellAndUserIdentifiedObject(EConsumableType a_type)
            : cal::threadpool::Consumable<EConsumableType>(a_type)
            , QObservable(QAppNG::CELL_AND_USER_IDENTIFIED_OBJECT)
            , m_userContextId(CELL_AND_USER_IDENTIFIED_OBJECT_DEFAULT_USERID)
        {
            m_cellid.mcc = CELL_AND_USER_IDENTIFIED_OBJECT_DEFAULT_MCC;       // 16 bits
            m_cellid.mnc = CELL_AND_USER_IDENTIFIED_OBJECT_DEFAULT_MNC;       // 16 bits
            m_cellid.lac = CELL_AND_USER_IDENTIFIED_OBJECT_DEFAULT_LAC;       // 16 bits
            m_cellid.rac = CELL_AND_USER_IDENTIFIED_OBJECT_DEFAULT_RAC;       // 16 bits
            m_cellid.ci  = CELL_AND_USER_IDENTIFIED_OBJECT_DEFAULT_CI;        // 16 bits

            m_imei.Init();
            m_imei_set = false;

            m_oldUserContextId = CELL_AND_USER_IDENTIFIED_OBJECT_DEFAULT_OLD_USERID; // 64 bits

            m_usc_present = false;
            m_usc_key = 0;        

            switch (a_type)
            {
                case (eCELL_AND_USER_ID_OBJECT_IUBPROC):
                {
                    m_type = UID_IUB_PROC;
                    break;
                }
                case (eCELL_AND_USER_ID_OBJECT_IUPROC):
                {
                    m_type = UID_IU_PROC;
                    break;
                }
                case (eCELL_AND_USER_ID_OBJECT_PDU):
                {
                    m_type = UID_PDU;
                    break;
                }
                default:
                    break;
            }
        };

        virtual ~CellAndUserIdentifiedObject()
        {
        };

        void SetImsi(CImsi imsi)
        {
            m_imsi = imsi;
        };
  
        void SetImei(IMEI imei, UInt8 type)
        {
            m_imei = imei;
            m_imei_set = true;
            m_imei_type = type;
        };

        TConsumable m_consumable;

        CELLID m_cellid;
        CImsi m_imsi;
    
        IMEI  m_imei;
        bool  m_imei_set;
        UInt8 m_imei_type;

        UInt64 m_userContextId;

        /// if the user was identified by a context Id and now that identifier changed
        /// the old key is saved here
        UInt64 m_oldUserContextId;

        // identifiers
        std::set<UInt64>    m_u_rnti_keys;
        std::set<UInt32>    m_c_rnti_keys;
        bool                m_usc_present;
        UInt32              m_usc_key;

        TRANSPORT_AND_LOGICAL_CHANNEL_TYPE m_iub_chan_type;
    };



    // Shorthand for frequently used template
    typedef CellAndUserIdentifiedObject< std::shared_ptr<CNlPdu> >  CellAndUserIdentifiedPdu;


}
#endif // __CELL_AND_USER_IDENTIFIED_OBJECT___NG



