#ifndef  __CPINFOSTRUCT_H___NG
#define  __CPINFOSTRUCT_H___NG

#include <QAppNG/QObservable.h>

///
#define MAXIMUM_ACCESS_NAME_LENGHT 100

namespace QAppNG
{
    ///
    typedef enum {
        CP_BSSGP_UL_UNITDATA  = 0x40,
        CP_BSSGP_RADIO_STATUS = 0x41,

        CP_RAB_ESTABLISHED    = 0xAA,
        CP_RAB_RELEASED       = 0xAB,
        CP_IU_RELEASED        = 0xAC,

        CP_SS7_LOCATION_SKEY                   = 0xC0,
        CP_SS7_LOCATION_SKEY_DB_INSERT_LOCALS  = 0xC1,
        CP_SS7_LOCATION_SKEY_DB_INSERT_ROAMERS = 0xC2,
        CP_SS7_LOCATION_SKEY_DB_UPDATE_LOCALS  = 0xC3,
        CP_SS7_LOCATION_SKEY_DB_UPDATE_ROAMERS = 0xC4,
        CP_SS7_LOCATION_SKEY_DB_DELETE_LOCALS  = 0xC5,
        CP_SS7_LOCATION_SKEY_DB_DELETE_ROAMERS = 0xC6,

        CP_IUB_HSDPA = 0xE0
    }CONTROL_PLANE_EVENT;

    typedef struct _control_plane_entry_ 
    {
        /// Control plane key
        UInt32      rnc_ip;
        UInt32      rnc_teid;
        UInt32      sgsn_ip;
        UInt32      sgsn_teid;
        /// CP event type
        UInt8       type;
        /// timestamp when the event was generated
        UInt32      sec;
        UInt32      nsec;
        /// User Identity parameters
        UInt64      imsi;
        UInt16      imsi_mcc;
        UInt16      imsi_mnc;
        UInt64      imsi_res;
        UInt64      imei;
        UInt64      rai;
        UInt64      sai;
        // RAB parameters
        UInt8       traffic_class;
        UInt32      max_bit_rate[2];
        UInt32      guaranteed_bit_rate[2];
        UInt8       cause;
        char        apn[MAXIMUM_ACCESS_NAME_LENGHT];
        UInt64      callid;
        char        msisdn[20];
    }cp_entry_t;


#pragma pack(1)
    typedef struct rab_info
    {
        /// timestamp when the event was generated
        UInt32      sec;                    //0
        UInt32      nsec;                   //4
        /// User Identity parameters
        UInt64      imsi;                   //8
        /// Cell
        UInt64      sai;                    //16
        /// RAB parameters
        UInt32      max_bit_rate[2];        //24
        UInt32      guaranteed_bit_rate[2]; //32
        UInt8       type;                   //40
        UInt8       cause;                  //41
        UInt8       traffic_class;          //42
        UInt8       pad;                    //43
                                            //size 44 Bytes
    }rab_info_t;

    typedef struct bssgp_info
    {
        /// timestamp when the event was generated
        UInt32      sec;
        UInt32      nsec;
        /// User Identity parameters
        UInt64      imsi;
        /// Cell
        UInt64      rai;
    
        UInt8       qos_profile[3];         //24 // filled for UL-UNITDATA only (coded as per 3GPP specs 08.18)
        UInt8       pad[13];                //27
        UInt8       type;                   //40    // values lower than 0x80 are reserved to Gb
        UInt8       cause;                  //41    // filled for Radio-Status only
        UInt16      ci;                     //42    // filled for UL-UNITDATA only
                                            //size 44 Bytes
    }bssgp_info_t;


    typedef struct ss7_info
    {
        /// timestamp when the event was generated
        UInt32      sec;                    //0
        UInt32      nsec;                   //4
        /// User Identity parameters
        UInt64      msisdn;                 //8
        UInt64      imsi;                   //16
        UInt64      imei;                   //24
        UInt32      servicekey;             //32
        UInt32      ndc;                    //36
        UInt8       type;                   //40   // values greater then 0xC0 reserver for SS7
        UInt8       pad;                    //41
        UInt16      cc;                     //42
                                            //size 44 Bytes
    }ss7_info_t;

    //TICKET #7744
    typedef struct iub_hsdpa_info
    {
        /// timestamp when the event was generated
        UInt32      sec;                    //0
        UInt32      nsec;                   //4
        /// User Identity parameters
        UInt64      imsi;                   //8
        UInt32      access_stratum_rel_ind; //16
        UInt8       hsdpa_activity_type;    //20
        UInt16      cellid;                 //21
        UInt16      rncid;                  //23
        UInt16      scrambling_code;        //25
        UInt16      mcc;                    //27
        UInt16      mnc;                    //29
        UInt16      lac;                    //31
        UInt8       ecno;                   //33
        Int8        rscp;                   //34
        UInt8       pad0[5];                //35
        UInt8       type;                   //40 
        UInt8       pad1[3];                //41
        //size 44 Bytes
    }iub_hsdpa_info_t;

    typedef union 
    {
        rab_info_t       rab_info;
        bssgp_info_t     bssgp_info;
        ss7_info_t       ss7_info;
        iub_hsdpa_info_t iub_hsdpa_info;
    } cp_info_t;
#pragma pack()


    typedef struct _cp_static_info_
    {
        UInt32      rnc_ip;
        UInt32      rnc_teid;
        UInt32      sgsn_ip;
        UInt32      sgsn_teid;
        UInt64      imsi;
        UInt16      imsi_mcc;
        UInt16      imsi_mnc;
        UInt64      imsi_res;
        UInt64      imei;
        UInt64      rai;
        UInt64      sai;
        char        apn[MAXIMUM_ACCESS_NAME_LENGHT];
        UInt64      callid;
        char        msisdn[20];
        UInt64      bytes_up;
        UInt64      bytes_down;
        UInt32      rab_start_sec;
        UInt32      rab_start_nsec;
        UInt32      rab_end_sec;
        UInt32      rab_end_nsec;
        UInt32      user_start_sec;
        UInt32      user_start_nsec;
        UInt32      user_end_sec;
        UInt32      user_end_nsec;
    } cp_static_info_t;


    typedef struct _cp_dynamic_info_
    {
        UInt8       type;
        UInt32      sec;
        UInt32      nsec;
        UInt8       traffic_class;
        UInt32      max_bit_rate[2];
        UInt32      guaranteed_bit_rate[2];
        UInt8       cause;
    } cp_dynamic_info_t;

    typedef std::list< cp_dynamic_info_t > cp_dynamic_info_list_t ;

    typedef struct _session_control_plane_info_
    {
        cp_dynamic_info_list_t   cp_dynamic_info_list;
        cp_static_info_t         cp_static_info;
    }session_control_plane_info_t;


    class IupsControlPlaneInfoObservable : public QObservable
    {
    public:
        IupsControlPlaneInfoObservable(UInt32 sec, UInt32 nsec) : QObservable(IUPS_CONTROLPLANE_INFO) 
        { 
            setQObservableTimestamp(sec, nsec); 
        }; 
        cp_entry_t m_data;
    };

    class GbControlPlaneInfoObservable : public QObservable
    {
    public:
        GbControlPlaneInfoObservable(UInt32 sec, UInt32 nsec) : QObservable(GB_CONTROLPLANE_INFO) 
        { 
            setQObservableTimestamp(sec, nsec); 
        }; 
        bssgp_info_t m_data;
    };

    class Ss7ControlPlaneInfoObservable : public QObservable
    {
    public:
        Ss7ControlPlaneInfoObservable(UInt32 sec, UInt32 nsec) : QObservable(SS7_CONTROLPLANE_INFO) 
        { 
            setQObservableTimestamp(sec, nsec); 
        }; 
        ss7_info_t m_data;
    };

    class IubHsdpaControlPlaneInfoObservable : public QObservable
    {
    public:
        IubHsdpaControlPlaneInfoObservable(UInt32 sec, UInt32 nsec) : QObservable(IUB_HSDPA_CONTROLPLANE_INFO) 
        { 
            setQObservableTimestamp(sec, nsec); 
        }; 
        iub_hsdpa_info_t m_data;
    };

}
#endif // __CPINFOSTRUCT_H___NG