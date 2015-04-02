#ifndef STRUCT_DEFS_H_NG
#define STRUCT_DEFS_H_NG

#ifdef WIN32
using boost::uint8_t;
using boost::uint16_t;
using boost::uint32_t;
using boost::uint64_t;
#else
#include <cstdint>
#endif

#pragma pack(1)
namespace QAppNG
{
    namespace IuPSFilter
    {

    enum {TCP_TAG=0xAABB, UDP_TAG=0xCCDD};

    struct RAI
    {
        uint16_t mcc;
        uint16_t mnc;
        uint16_t lac;
        uint8_t  rac;
    };

    struct SAI
    {
        uint16_t mcc;
        uint16_t mnc;
        uint16_t lac;
        uint16_t sac;
    };

    struct IMEI
    {
        uint32_t tac;
        uint32_t snr;
        uint8_t svn;
    };

    struct TCP
    {
        uint16_t tag;

        uint64_t startTime;
        uint64_t endTime;
        uint64_t lastPduTime;

        uint8_t  termType;
        uint8_t  TCP_State;

        uint32_t TCP_SetupDurationMsec;
        uint32_t TCP_NW_LatencyMsec;                    //???
        uint32_t TCP_MS_LatencyMsec;                    //???

        char     TCP_ServerAddress[15+1];
        uint16_t TCP_ServerPort;

        uint64_t bytesUplink;
        uint64_t bytesDownlink;
        uint64_t bytesRtxUplink;
        uint64_t bytesRtxDownlink;

        uint32_t PDP_CXT_ACT_DurationMsec;
        uint8_t  ticketType;
        uint8_t  subTicketType;
        char     longestPath[64+1];

        RAI      rai;
        SAI      sai;
        uint16_t ci;
        uint64_t imsi;
        IMEI     imei;

        //uint8_t  RAB_Status;                            //???
        char     APN[100+1];
        //uint8_t  RAB_Cause;                             //???
        uint64_t callID;
        uint64_t SYNACK_Time;
        uint64_t SYN_Time;
        char     UE_IP[15+1];
        uint32_t UE_TEID;
        char     SGSN_IP[15+1];
        uint32_t SGSN_TEID;

        /*
         *uint8_t  trafficClass;                          //???
         *uint32_t maximumBitrateUplink;                  //???
         *uint32_t maximumBitrateDownlink;                //???
         *uint32_t guaranteedBitrateDownlink;             //???
         *uint32_t guaranteedBitrateUplink;               //???
         */

        char     MSISDN[40+1];
        char     userAgent[128+1];

        uint32_t TCP_SessionDurationMsec;
        uint16_t RNC_ID;
        char     RNC_Name[64+1];                        //???
    };
    }
}
#pragma pack()

#endif // STRUCT_DEFS_H_NG

