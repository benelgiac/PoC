#ifndef PROBEHEADER_INCLUDE_NG
#define PROBEHEADER_INCLUDE_NG

/*
A.B. Most of the following is redefined slightly differently in 
source/pdu/stacks/Opl/test/src/ProbeHeader_Opl.h, so we have to "doubleguard" all this
stuff from that header
*/
#ifndef PROBEHEADER_OPL_INCLUDE_NG
#define PROBEHEADER_OPL_INCLUDE_NG


#include <cal/defs.h>

#define NETLEDGE_FLAG_MARKER 0x7E

#define MOS_PROTO_FLAG_MARKER 0x7C

#define MAX_PROTOCOL_TYPE  0x0F

namespace QAppNG
{
    typedef enum 
    {
        UNK_FORMAT = 0x00,
        ACS7_FORMAT = 0x01,
        DFU_FORMAT = 0x02,
        ACS7_EXT_FORMAT = 0x03,
        ETH_FORMAT = 0x04,
        ATM_FORMAT = 0x05
    } PDU_FORMAT;
    
    typedef enum 
    {
        PDU_NORMAL=0,
        PDU_RETX=1,
        PDU_RETX_NP=2,
        PDU_RETX_REQ=3
    } MCAST_PDU_TYPE;
    
    
    typedef enum 
    {
        UNK_FFORMAT = 0x00,
        NL_NO_PREFIX_FFORMAT = 0x01,
        NL_FFORMAT = 0x02, 
        NL2_FFORMAT = 0x03
    } FILE_FORMAT;
    
    typedef enum 
    {
        eS1AP = 0x00,
        eLAPD = 0x01,
        eMTP2 = 0x02,
        eFR = 0x03,
        eIUPS = 0x04,
        eIUB = 0x05,
        eMTP3 = 0x06,
        eMTP2E = 0x07,
        eSEQ16 = 0x08,
        eM3UA = 0x09,
        eH248 = 0x0A,
        eNBAP= 0x0B,
        eEXTENSION_PROTOCOL = 0x0C, //check ACS7_INFO padding as protocol extension and OLD M3UA
        eGBIP = 0x0D,
        eETH = 0x0E,
        eUNK = 0x0F,
        // eUNK should be always 0x0F
        eGTP,
        eDIAMETER,
        eSMPP,
        eSGs,
        eGXGY,
        eIP_NBAP
    } PROTOCOL_TYPE;



    typedef enum {
        eIUA = 0x00,
        eDUA = 0x01,
        eSUA = 0x02,
        eBICC= 0x03
    } PROTOCOL_TYPE_EXTENSION;

    typedef enum {
        eAAL2 =0,
        eAAL5 =1
    }AAL_TYPE;

    typedef enum {
        eOK_HDLC = 0x07,            /* HDLC Encapsulated frame (Valid PDU)  */
        eERRORFRAME = 0x04,         /* Frame has error given in statusValue*/
        eAPPGENERATED = 0x06,       /* Application generated event*/
        eSYNCH_LOSS_EVENT = 0x14,   /* Loss of synch - error in statusValue*/
        eDATA_LOSS_EVENT = 0x15,    /* Loss of Channel data - error in*/
                                    /* statusValue*/
        eCONFIG_EVENT = 0x20        /* Configuration change detected*/
                                    /* (statusValue of zero indicates*/
                                    /* sucessful config update)*/
    } EVENTID;

    typedef enum {
                                    /* values for all events:*/
        eSU_NO_ERROR = 0x00,        /* No error detected (indicates*/
                                    /* successful config update if*/
                                    /* event is CONFIG_EVENT)*/
                                    /* values for ERRORFRAMEs/SYNCH_LOSS_EVENTs:*/
        eFRAME_ABORT_ERROR = 0x02,  /* HDLC ABORT Detected*/
        eLOSS_OF_SYNC_ERROR = 0x03, /* No Flags are being received*/
        eOCTET_COUNTING_ERR = 0x03, /* We are in octet counting Mode*/
        eSHORT_FRAME_ERROR = 0x04,  /* SS7 Frame less than the legal*/
                                    /* minimum len*/
        eLONG_FRAME_ERROR = 0x05,   /* SS7 Frame is too long CELTIC*/
                                    /* only*/
        eALIGNMENT_ERROR = 0x07,    /* Number of bits in frame not*/
                                    /* multiple of 8*/
        eCRC_ERROR = 0x08,          /* HDLC CRC error detected*/
                                    /* values for DATA_LOSS_EVENTs*/
        eDATA_OVERFLOW_ERROR = 0x01,/* The Acquisition Buffer has*/
                                    /* overflowed*/
        eAPP_DISABLED = 0x02,       /* Channel has been disabled by*/
                                    /* application*/
        eCHANNEL_SELFTEST = 0x03,   /* Channel has been removed for*/
                                    /* Self Test*/

        eINTERNAL_OVERFLOW = 0x04,  /* Internal Acquisition Overflow*/
        eEXTERNAL_OVERFLOW = 0x05,  /* External Acquisition Overflow*/
        eSCSI_OVERFLOW = 0x06,      /* SCSI Transfer capacity exceeded*/
        eTIMESYNC_LOSS = 0x07,      /* Upper timesync alarm threshold*/
                                    /* has been exceeded*/
        eTLAYER_OVERFLOW = 0x08,    /* T-Layer ran out of CDU buffers*/
        eCIP_FAILURE = 0x09,        /* T-Layer reports CIP poll loss*/
        eCDU_TOO_OLD = 0x0A,        /* Data delayed beyond buffering*/
                                    /* capacity*/
        eBAD_CDU = 0x0B,            /* CDU discarded ue to bad header*/
                                    /* data*/
        eAPP_AX_DISABLED = 0x0C,    /* The channel has been*/
                                    /* deregistered at the AX-Client*/
        eAX_CLIENT_SHUTDOWN = 0x0D, /* The AX-Client is closing down.*/
        eAX_PDU_LONG_ERR = 0x0E,    /* Long PDU error*/
        eDATA_INTERRUPTED = 0x0F,   /* No data has been received for a*/
                                    /* period of time (nominally 5*/
                                    /* seconds) even though we are*/
                                    /* configured to look for it.*/
        eENDOFDATA = 0x10           /* Control frame to indicate end of*/
                                    /* transferred data to the AX-Layer*/
    } STATUS_VALUE;


    typedef union acx_timeslot
    {
        UInt8 full_ts;

#if defined(CAL_LITTLE_ENDIAN)
        struct
        {
            UInt8 pcm_ts : 5;
            UInt8 sub_ts : 3;
        } s;
#else
        struct
        {
            UInt8 sub_ts : 3;
            UInt8 pcm_ts : 5;
        } s;
#endif

    }ACX_TIMESLOT;

    typedef struct acx_recheader
    {
        unsigned char   mm;
        unsigned char   ss;
        unsigned char   hundreds;
        unsigned char   input;

        ACX_TIMESLOT    ts;
        unsigned char   tei;

#if FLAG_GB_RECORDING
        unsigned short  msg_len;
#else
        unsigned char   msg_len;
        unsigned char   cgi_idx;
#endif

    } ACX_RECHEADER;

    typedef struct acs7_tstamp {
        UInt32 tv_sec;
        UInt32 tv_usec;
    } ACS7_TSTAMP;

    typedef struct dfu_timestamp
    {
        UInt32  sec;
        UInt32  nsec;
    } DFU_TIMESTAMP;

    typedef union dfu_chid
    {
        UInt32 word;
    /*#ifdef CAL_LITTLE_ENDIAN
        struct
        {
            UInt8   rate;
            UInt8   sub;
            UInt8   ts;
            UInt8   input;

        } fields;
#endif*/

    //#ifdef CAL_BIG_ENDIAN
        struct
        {
            UInt8   input;
            UInt8   ts;
            UInt8   sub;
            UInt8   rate;

        } fields;
    //#endif

    } DFU_CHID;

#pragma pack(1)
    typedef struct acs7_info
    {
        UInt8           flag;
        UInt8           ver_prot;
        UInt8           eventId;
        UInt8           statusValue;

        ACS7_TSTAMP     timestamp;

        UInt16          site;
        UInt8           cardCage;
        UInt8           ifCard;

        UInt32          tsID;
        UInt16          len;
        UInt16          pad; //PROTOCOL_TYPE_EXTENSION for wireshark
    } ACS7_INFO;
#pragma pack()


#pragma pack(1)
    typedef struct atm_info
    {
        UInt8           flag;   // should set to NETLEDGE_FLAG_MARKER
        UInt8           ver_prot; // 4 msb are the PDU_FORMAT - 4 lsb are the PROTOCOL_TYPE
        UInt8           eventId;    // EVENTID
        UInt8           statusValue;    //STATUS_VALUE
        ACS7_TSTAMP     timestamp;
        UInt16          aaltype;
        UInt16          vpi;
        UInt16          vci;
        UInt16          len;
        UInt8           mezz_card;
        UInt8           port;
        UInt8           cid;
        UInt8           more_bit;
    } ATM_INFO;
#pragma pack()

#pragma pack(1)
    typedef struct eth_info
    {
        UInt8           flag;   // should set to NETLEDGE_FLAG_MARKER
        UInt8           ver_prot; // 4 msb are the PDU_FORMAT - 4 lsb are the PROTOCOL_TYPE
        UInt8           eventId;    // EVENTID
        UInt8           statusValue;    //STATUS_VALUE
        DFU_TIMESTAMP   timestamp;
        UInt16          len;
        UInt8           physical_id;  // 4 msb are the card id - 4 lsb are the port id
        UInt8           pad[9];
    } ETH_INFO;
#pragma pack()

#pragma pack(1)
    typedef struct dfu_info
    {
        UInt8           flag;
        UInt8           ver_prot;
        UInt16          eventId;

        UInt32          unitID;
        DFU_CHID        chID;

        DFU_TIMESTAMP   timestamp;

        UInt16          ref;
        UInt16          len;

    } DFU_INFO;
#pragma pack()

    typedef struct acx_info
    {
        unsigned char   mm;
        unsigned char   ss;
        unsigned char   hundreds;
        unsigned char   input;

        ACX_TIMESLOT    ts;
        unsigned char   tei;

#if FLAG_GB_RECORDING
        unsigned short  msg_len;
#else
        unsigned char   msg_len;
        unsigned char   cgi_idx;
#endif

    } ACX_INFO;

    typedef struct pdu_head24
    {
        union
        {
            ACS7_INFO acs7;
            DFU_INFO dfu;
            ATM_INFO atm;
            ETH_INFO eth;
        }info;

    } PDU_HEAD24;

#pragma pack(1)
    typedef struct mos_info
    {
        UInt8  flag;
        UInt8  message_type;
        UInt16 payload_len;
        UInt32 seq_number;
    } MOS_INFO;
#pragma pack()


#if defined(CAL_LITTLE_ENDIAN)
    typedef struct fields
    {
        UInt32 lli:20;        // 4 msb are the PROTOCOL_TYPE - 16 lsb are the LAC
        UInt32 pduType:4;    //MCAST_PDU_TYPE
        UInt32 seq:8;
    } FIELDS;
#else
    typedef struct fields
    {
        UInt32 seq:8;        
        UInt32 pduType:4;    //MCAST_PDU_TYPE
        UInt32 lli:20;        // 4 msb are the PROTOCOL_TYPE - 16 lsb are the LAC
    }FIELDS;
#endif

#if defined(CAL_LITTLE_ENDIAN)
    typedef struct fields_16bitseq
    {
        UInt32 seq:16;
        UInt32 protType:4;
        UInt32 pduType:4;
        UInt32 spare:8;
    } FIELDS_16BITSEQ;
#else
    typedef struct fields_16bitseq
    {
        UInt32 spare:8;        
        UInt32 pduType:4;
        UInt32 protType:4;
        UInt32 seq:16;
    }FIELDS_16BITSEQ;
#endif

    typedef struct nl_prefix
    {
        union
        {
            UInt32 word;
            FIELDS flds;
            FIELDS_16BITSEQ flds16;
        }u;
    } NL_PREFIX;

    typedef struct nl_head
    {
        NL_PREFIX prefix;
        PDU_HEAD24 pdu_head;
    } NL_HEAD;

    typedef struct newfile_nl_head
    {
        UInt32 ip;
        NL_PREFIX prefix;
    } NEWFILE_NL_HEAD;


    typedef struct complete_pdu
    {
        NL_HEAD head;
        UInt8 data[4096];
    } COMPLETE_PDU;

    typedef struct ip_and_complete_pdu
    {
        UInt32 ip;
        COMPLETE_PDU complete_pdu;
    } IP_AND_COMPLETE_PDU;

#pragma pack(1)
    typedef struct acs7_ext
     {
     ACS7_INFO hdr;
     struct cgi
    {
    unsigned short mcc;
    unsigned short mnc;
    unsigned short lac;
    unsigned short ci;
    unsigned char rac;
    unsigned char pad;
    } embedded_cgi;
    } ACS7_EXT;
#pragma pack()

    //Added for new rxpdufw for pcap file reading
    typedef struct pcap_commprove_header
    {
        UInt32 sec;
        UInt32 nsec;
        UInt16 len;
        UInt8  skip_ip;
        UInt8  skip_atm;
    } pcap_commprove_header_t;

#pragma pack(1)
    typedef struct GENERIC_PDU_HEADER
    {
        UInt8     flag;
        UInt8     ver_prot;
        char      padding[22];
    }GENERIC_PDU_HEADER;
#pragma pack()

#pragma pack(1)
    typedef struct GUINLRHEADER
    {
        UInt64             prefix;
        GENERIC_PDU_HEADER pdu_hd;
    }GUINLRHEADER;
#pragma pack()

#endif //PROBEHEADER_OPL_INCLUDE_NG

    /* 
    The following is not redefined in source/pdu/stacks/Opl/test/src/ProbeHeader_Opl.h,
    so we keep it unprotected by the inner guard
    */

    /// The information contained in this header are a mix of physical (link) information and
    /// a protocol signature which summarizes the protocols information (which protocols and message types
    /// are found in the PDU this header belongs to.
    struct pdu_idx_summary_info_t
    {    
#pragma pack(1)
        union endpoints_t
        {
            enum {POINTCODE=0, CELLID=1, RNCID=2, IP=3};
            UInt32 word;
            struct {
                UInt32 dir :2;
                UInt32 unk :28;
                UInt32 format :2;
            } fields;
            struct 
            {
                Int32 dir :2;
                UInt32 ep1 :14;
                UInt32 ep2 :14;
                UInt32 format :2;
            } info_pointcode;
            struct 
            {
                Int32 dir :2;
                UInt32 ci :16;
                UInt32 dummy :12;
                UInt32 format :2;
            } info_cellid;

            struct  
            {
                Int32 dir      :2;
                UInt32 rncid :28;
                UInt32 format :2;
            } info_rncid;

            struct  
            {
                UInt32 ip_dest_addr    :30;
                UInt32 format :2;
            } info_ip;


        };


        union phys_info_t
        {
            struct 
            {
                UInt32 dummy0;
                endpoints_t endpoints;
                UInt8 dummy1;
                UInt8 dummy2;
                UInt8 dummy3;
            } generic;

            struct 
            {
                UInt32 probe_ip; 

                endpoints_t endpoints;

                UInt8 probe_in; 
                UInt8 ts;
                UInt8 sub;
            } non_atm;

            struct 
            {
                UInt16 vpi; 
                UInt16 vci;

                endpoints_t endpoints;

                UInt8 chid;
                UInt8 spare0;
                UInt8 spare1;
            } atm;

        };

        UInt64 timestamp;
        phys_info_t phys;

        //Not so well-defined part, dependent on how we decide to store information about protocols contained in PDU
        struct proto_signature_t
        {
            proto_signature_t () 
            {
                flags=0xFF;    
                mt1=0xFF;    
                mt2=0xFF;    
                mt3=0xFF;    
                mt4=0xFF;    
            }

            bool IsFromATM (void) {return (flags & 0x80)?true:false;}

            UInt8 flags;    //Some sort of version indicating how the following part is structured (for now it will be the same as nlPdu.m_stack_type)
            UInt8 mt1;    //e.g. SCCP MT 
            UInt8 mt2;    //e.g. BSSMAP MT
            UInt8 mt3;    //e.g. L3 PD (lower 4 bits)
            UInt8 mt4;    //e.g. L3 MT
        } proto;

#pragma pack()
    }; 


}
#endif
