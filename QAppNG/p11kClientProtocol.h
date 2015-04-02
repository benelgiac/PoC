#ifndef P11KCLIENTPROTO_INC_NG
#define P11KCLIENTPROTO_INC_NG


#define MAX_TRIES            240     /* 4 tries in a second * 60 seconds in a minute */

#define COMMAND_OFFSET            1        /*expressed in words i.e. 4 bytes*/
#define    NUM_PARAMETER_OFFSET        2
#define PARAMETER_OFFSET        3

#define TCP_SERVER_PORT            5003 
#define UDP_SERVER_PORT            5002

#define CLOSE_SERVER_MSG        "Close tcp Server task"
#define GET_VERSION_STRING        "ver"
#define ALL_MSGS_SENT_STRING        "All msgs sent"
#define CHANNEL_ON_STRING        "All channels on"

#define COMMAND_MINIMUM_LENGTH                12
#define ZERO_PARAMETER                                0

#define ALL_MSGS_LENGTH            14

#define TX_BUFFER_DIM            32*32*4    /*all expressed in bytes*/

#define GBPA_CMD_CLASS            0x100

namespace QAppNG
{
    typedef struct sockaddr_in *SOCKADDR_IN_PTR;

    /*******************************************************************************
    *
    * Client Command enumeration
    */
    typedef enum
    {
        eLogout = 0x5a,
        eReset,
        eConfigErase,
        eBiloUp,
        eRexec,
        eRequestTx,
        eVersions,
        eVoid,
        eTxHiz,
        eATMFilterLen,
        eATMFilterChan,
        eATMAck,
        eATMResetLengthFilter,
        eATMResetChannelFilter,
        eGBPA_DLCI_CFG=0x100,
        eGBPA_DLCI_ADD,
        eGBPA_DLCI_DEL
    } eClientCommand;
    
    typedef enum
    {
        emSetIP,
        emSetPort,
        emLogout,
        emReset,
        emDelFlash,
        emBiloup,
        emRemExec,
        emSetSaveFile,
        emVersions,
        emATMFilterLen,
        emATMFilterChan,
        emATMFilterLenReset,
        emATMFilterChanReset,
        emExit,
        emNumItem
    }eMenuItems;

    typedef struct p11kc_packet
    {
        UInt32 ulLength;
        UInt32 ulCommandId;
        UInt32 ulNumberOfParameters;   // value in number of chars
        char   parameters;
    } P11KCPKT;

    typedef struct gbpa_cmd
    {
        UInt16 ver;
        UInt16 refid;
        UInt32 ulCommandId;
        UInt32 ulNumberOfParameters;
    } GBPA_CMD;

}
#endif

