#ifndef INCLUDE_CONTEXT_IDENTITY_CONTENT_NG
#define INCLUDE_CONTEXT_IDENTITY_CONTENT_NG
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
///          Alessandro Manetti&Lorenzo Capecchi   15-June-2007 Original development
/// @endhistory
///
// ===========================================================================

/*  ...Include files
*/
#include "cal/defs.h"
#include <QAppNG/Imsi.h>
#include "netledge_types.h"
#include "datatype24008.h"
#include "TmsiComm.h"
#include <QAppNG/TmsiImsiContent.h>

namespace QAppNG
{

    // --------------------------------------------------------------------------------------------------------------------

    class ContextIdentityContent
    {
    public:
        ContextIdentityContent() 
            : m_toinsert(false), m_new_tmsi(0), m_old_tmsi(0), m_new_lac(0), m_old_lac(0), m_new_mcc(0)
            , m_old_mcc(0), m_new_mnc(0), m_old_mnc(0), m_new_rac(0), m_old_rac(0)
            , m_cell_id(0)
            , m_imeiType(IMEI_type_UNSET)
        , m_identityaccepted(false)
            , m_checkedInsert(false)
         , m_locUpdateType(TmsiMgrComm::LocationUpdate_Unknown)
        , m_rauUpdateType(TmsiMgrComm::RoutingUpdate_Unknow)
        , is_imsi_present_by_signaling(false)
        , is_imsi_present_by_tmsimgr(false)
        {};

        void SetTmsi(UInt32 tmsi) { m_new_tmsi = tmsi; m_old_tmsi = tmsi; };
        void SetPtmsi(UInt32 tmsi) { m_new_tmsi = tmsi; m_old_tmsi = tmsi; };
        void SetChangeTmsi(UInt32 newtmsi) { m_new_tmsi = newtmsi; };
        void SetLac(UInt16 lac) { m_new_lac = lac; m_old_lac = lac; };
        void SetRac(UInt8 rac) { m_new_rac = rac; m_old_rac = rac; };
        void SetLai(UInt16 mcc, UInt16 mnc, UInt16 lac)
        {
            m_new_lac = lac ;
            m_old_lac = lac;
            m_new_mcc = mcc;
            m_old_mcc = mcc; 
            m_new_mnc = mnc ;
            m_old_mnc = mnc;  
        };
        void SetLai(const LAI &lai)
        {
            m_new_lac = lai.lac ;
            m_old_lac = lai.lac;
            m_new_mcc = lai.mcc;
            m_old_mcc = lai.mcc; 
            m_new_mnc = lai.mnc ;
            m_old_mnc = lai.mnc;  
        };
        void SetRai(UInt16 mcc, UInt16 mnc, UInt16 lac, UInt8 rac)
        {
            m_new_rac = rac;
            m_old_rac = rac;
            m_new_lac = lac ;
            m_old_lac = lac;
            m_new_mcc = mcc;
            m_old_mcc = mcc; 
            m_new_mnc = mnc ;
            m_old_mnc = mnc;  
        };
        void SetRai(const RAI &rai)
        {
            m_new_rac = (UInt8)rai.rac ;
            m_old_rac = (UInt8)rai.rac;
            m_new_lac = rai.lac ;
            m_old_lac = rai.lac;
            m_new_mcc = rai.mcc;
            m_old_mcc = rai.mcc; 
            m_new_mnc = rai.mnc ;
            m_old_mnc = rai.mnc;  
        };

        inline void SetImsiBySignaling(CImsi imsi) 
        {
            if (imsi.isValid())
            {
                m_imsi = imsi; 
                is_imsi_present_by_signaling = true;
                is_imsi_present_by_tmsimgr = false;
            }
        };

        inline void SetImsiByTmsiMgr(CImsi imsi)
        {
            if (is_imsi_present_by_signaling)
                return;

            if (imsi.isValid())
            {
                m_imsi = imsi;
                is_imsi_present_by_tmsimgr = true;
            }
        };

        void SetChangeLai(const LAI &lai) { m_new_lac = lai.lac; m_new_mcc = lai.mcc; m_new_mnc = lai.mnc ;  };
        void SetChangeRai(const RAI &rai) { m_new_rac = (UInt8)rai.rac; m_new_lac = rai.lac; m_new_mcc = rai.mcc; m_new_mnc = rai.mnc ;  };
        void SetOldLai(const LAI &lai) { m_old_lac = lai.lac; m_old_mcc = lai.mcc; m_old_mnc = lai.mnc ;  };
        void SetOldRai(const RAI &rai) { m_old_rac= (UInt8)rai.rac; m_old_lac = rai.lac; m_old_lac = rai.lac; m_old_mcc = rai.mcc; m_old_mnc = rai.mnc ;  };
        void SetChangeLac(UInt16 newlac) {m_new_lac = newlac;};
        void SetChangeRac(UInt8 newrac) {m_new_rac = newrac;};
        void SetImei(const IMEI &imei) {m_imei = imei; m_imeiType = IMEI_type_IMEI;};
        void SetImeisv(const IMEI &imei) {m_imei = imei; m_imeiType = IMEI_type_IMEISV;};
    
        bool IsChangedSomething() const
        {
            if ( m_old_tmsi && m_old_lac && ( (m_new_tmsi!=m_old_tmsi) || (m_new_lac!=m_old_lac) || (m_new_mnc!=m_old_mnc) || (m_new_mcc!=m_old_mcc) ) )
            {
                return true;
            }

            return false;
        };

        bool IsChangedSomethingPS() const
        {
            if ( m_old_tmsi && m_old_lac && m_old_rac && ( (m_new_tmsi!=m_old_tmsi) || (m_new_lac!=m_old_lac) || (m_new_rac!=m_old_rac) || (m_new_mnc!=m_old_mnc) || (m_new_mcc!=m_old_mcc) ) )
            {
                return true;
            }

            return false;
        };

        bool AreNewTmsiLacValid(void) const
        {
            if ( (m_new_tmsi!=0) && (m_new_lac!=0)  && (m_identityaccepted) )
            {
                return true;
            }

            return false;
        };

        bool AreNewPtmsiRacValid(void) const
        {
            if ( (m_new_tmsi!=0) && (m_new_rac!=0)  && (m_identityaccepted) )
            {
                return true;
            }

            return false;
        };

        CImsi GetImsi() const{ return m_imsi; };
        bool GetImei(IMEI& a_in) const { if (m_imeiType != IMEI_type_IMEI) return false;  a_in = m_imei;  return true; };
        bool GetImeisv(IMEI& a_in) const { if (m_imeiType != IMEI_type_IMEISV) return false;  a_in = m_imei;  return true; };
        UInt32 GetNewTmsi() const { return m_new_tmsi; };
        UInt16 GetNewLac() const { return m_new_lac; };
        UInt8 GetNewRac() const { return m_new_rac; };
        UInt32 GetOldTmsi() const { return m_old_tmsi; };
        UInt16 GetOldLac() const { return m_old_lac; };
        UInt8 GetOldRac() const { return m_old_rac; };
        void SetToInsert(bool val) { m_toinsert = val; };
        bool GetToInsert() const { return m_toinsert; };
        void SetIdentityAccepted(bool val) { m_identityaccepted = val; };
        bool GetIdentityAccepted() const { return m_identityaccepted; };
        void SetCheckedInsert(bool val) { m_checkedInsert = val; };
        bool GetCheckedInsert() const { return m_checkedInsert; };
        TmsiMgrComm::LocationUpdate GetLocationUpdateType() const{ return m_locUpdateType; };
        void SetLocationUpdateType(TmsiMgrComm::LocationUpdate a_type) { m_locUpdateType = a_type; };
        TmsiMgrComm::RoutingUpdate GetRoutingUpdateType() const{ return m_rauUpdateType; };
        void SetRoutingUpdateType(TmsiMgrComm::RoutingUpdate a_type) { m_rauUpdateType = a_type; };
        void    SetCellId(UInt16 cellId, CTmsiImsiContent::CellIdType type) {m_cell_id = cellId; m_cell_type = type;};
        void    SetCellType(CTmsiImsiContent::CellIdType type) {m_cell_type = type;};
        UInt16  GetCellId() const {return m_cell_id;};
        CTmsiImsiContent::CellIdType   GetCellType() const {return m_cell_type;};
    private:
        bool    m_toinsert;
        CImsi    m_imsi;
        UInt32    m_new_tmsi;
        UInt32    m_old_tmsi;
        UInt16    m_new_lac;
        UInt16    m_old_lac;
        UInt16    m_new_mcc;
        UInt16    m_old_mcc;
        UInt16    m_new_mnc;
        UInt16    m_old_mnc;
        UInt8    m_new_rac;
        UInt8    m_old_rac;
        IMEI    m_imei;
        UInt16  m_cell_id; //ci for 2G, sac for 3G
        CTmsiImsiContent::CellIdType   m_cell_type;
        enum IMEI_type
            {
            IMEI_type_UNSET,
            IMEI_type_IMEI,
            IMEI_type_IMEISV,
            } m_imeiType;
        bool    m_identityaccepted;
        bool    m_checkedInsert;
        TmsiMgrComm::LocationUpdate m_locUpdateType;
        TmsiMgrComm::RoutingUpdate m_rauUpdateType;

        bool is_imsi_present_by_signaling;
        bool is_imsi_present_by_tmsimgr;
    };
    // --------------------------------------------------------------------------------------------------------------------

}
#endif
