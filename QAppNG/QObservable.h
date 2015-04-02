#ifndef INCLUDE_QOBSERBABLE_H_NG
#define INCLUDE_QOBSERBABLE_H_NG
/** ===================================================================================================================
  * @file    QObsrvable HEADER FILE
  *
  * @brief   Base class for observables with Timestamp and Routing Key
  *
  * @copyright
  *
  * @history
  * REF#        Who                                                              When          What
  * #5022       A. Della Villa, F. Gragnani, G. Benelli                          May-2012      Original Development
  * #8060.......A. Della Villa, D. Verna, F. Lasagni.............................Jan-2013      Removing CAL
  *
  * @endhistory
  * ===================================================================================================================
  */

#include <bitset>
#include <assert.h>

namespace QAppNG
{
    // --------------------------------------------------------------------------------------------------------------------
    // Reserved values for routing key
    static const UInt64 BROADCAST_ROUTING_KEY_VALUE = 0xFFFFFFFFFFFFFFFF;
    static const UInt64 AUTOMATIC_ROUTING_KEY_VALUE = 0xFFFFFFFFFFFFFFFE;


    // --------------------------------------------------------------------------------------------------------------------
    typedef enum
    {
        // 1) PDU
        PDU,
        REMOTE_PDU,
        TIMEOUT_PDU, //#5498 in band signaling used only for load balancing
        UNACCURATE_TS_PDU, // not all pdus received have accurate timestamping, so these must not go into virtual clock

        // 2) PROCEDUREs
        TIMESTAMP_PROCEDURE,
        IU_PROCEDURE,
        IUB_PROCEDURE,

        // 3) EVENTs (also INFO are considered events)
        IU_CHANNEL_INFO,
        CELL_CONTEXT_CONFIG,
        GB_UNUSED_CONTEXT_ID,
        LTE_UNUSED_CONTEXT_ID,
        LTE_UPDATE_USER_CONTEXT_ID,
        LTE_DELETE_USER_CONTEXT_ID,
        QVIRTUALCLOCK_TIME_PULSE,
        QVIRTUALCLOCK_FLUSH,
        QVIRTUALCLOCK_APPLICATION_START,
        QVIRTUALCLOCK_APPLICATION_SHUTDOWN,
        SLAB_WRITER_EVENT,
        TMSI_MGR_EVENT,

        // 4) TICKETs
        GENERIC_TICKET,
        IUB_TICKET,
        IU_TICKET,
        MM_TICKET,
        GMM_TICKET,
        SM_TICKET,
        GB_PAGING_TICKET,
        GB_INTERFACE_TICKET,
        BSSGP_TICKET,
        VOIP_TICKET,
        XDR_TICKET_FIELDS,

        // 5) USER ID Objects
        UID_PDU,
        UID_IUB_PROC,
        UID_IU_PROC,
        CELL_AND_USER_IDENTIFIED_OBJECT,
        IUB_USER_CONTEXT_EVENT,          // this is the type of CIubUserContextEvent
        TCP_XDR_TICKET,
        IUPS_FILTER_PROCEDURE,
        GEVENT, // GEvent
        A_EVENT,
        ABIS_EVENT,

        // 6) INTERCOMUNICATION BETWEEN APPLICATION
        IUPS_CONTROLPLANE_INFO,
        OUTPUT_TICKET,
        GB_CONTROLPLANE_INFO,
        SS7_CONTROLPLANE_INFO,
        IUB_HSDPA_CONTROLPLANE_INFO,

        // 7) CORRELATION EVENT
        SDP_EVENT_OBS,
        MOS_MESSAGE_OBS,
        MOS_REPORT_OBS,

        // 8) XDR Tickets
        XDR_TICKET,

        // 9) GENERIC STRING OBS
        STRING_OBS,

        // 10) Tmsi Comm Observable
        TMSI_COMM_OBSERVABLE,

        MAX_NUMBER_OF_OBSERVABLES
    } QObservableType;

    // --------------------------------------------------------------------------------------------------------------------

    class QObservable
    {
    public:
        // CTOR
        QObservable( QObservableType observable_type )
            : m_type(observable_type)
            , m_has_timestamp(false)
            , m_has_routing_key(false)
            , m_has_context_key(false)
            , m_timestamp_sec(0)
            , m_timestamp_nsec(0)
            , m_context_key(0)
            , m_routing_key(0)
            , m_norm_optional_sequence_number(0)
        {}

        // DTOR
        virtual ~QObservable() {}

        // Observable type
        inline QObservableType GetType()
        {
            return m_type;
        }

        // Timestamp handling methods
        bool hasQObservableTimestamp() const { return m_has_timestamp; }
        void setQObservableTimestamp( UInt32 sec, UInt32 nsec = 0 ) { m_has_timestamp = true; m_timestamp_sec = sec; m_timestamp_nsec = nsec; }
        void setQObservableTimestamp( UInt64 sec_and_nsec ) { setQObservableTimestamp( static_cast<UInt32>( sec_and_nsec >> 32 ), static_cast<UInt32>( sec_and_nsec & 0x00000000FFFFFFFF ) ); }
        void resetQObservableTimestamp() { m_has_timestamp = false; m_timestamp_sec = 0; m_timestamp_nsec = 0; }
        UInt64 getQObservableTimestamp() const { return ( static_cast<UInt64>(m_timestamp_sec) << 32 ) | ( static_cast<UInt64>(m_timestamp_nsec) ); }
        UInt32 getQObservableTimestampSec() const { return m_timestamp_sec; }
        UInt32 getQObservableTimestampNSec() const { return m_timestamp_nsec; }

        // Routing Key handling methods...
        void resetQObservableRoutingKey()
        { 
            m_has_routing_key = false;
            m_routing_key = 0; 
        }

        bool hasQObservableRoutingKey() const 
        { 
            return m_has_routing_key; 
        }

        //... as uint64
        void setQObservableRoutingKey( UInt64 routing_key ) 
        { 
            m_has_routing_key = true; 
            m_routing_key = routing_key; 
        }

        UInt64 getQObservableRoutingKey() const 
        { 
            return m_routing_key.to_ullong();
        }
        
        //... as std::bitset
        bool addRemoteProcessorId(UInt8 remote_id)
        {
            if (m_has_routing_key == false)
            {
                assert(m_routing_key.none());
            }

            return routingKeyValidator(remote_id, true);
        }

        bool delRemoteProcessorId(UInt8 remote_id)
        {
            if (m_has_routing_key == false)
            {
                assert(m_routing_key.none());
            }

            return routingKeyValidator(remote_id, false);
        }

        bool checkRemoteProcessorId(UInt8 remote_id)
        {
            if (m_has_routing_key != false && remote_id < 64)
            {
                return m_routing_key.test(remote_id);
            }
            else
            {
                return false;
            }
        }

        bool setRemoteProcessorIds(const std::bitset<64>& routing_key)
        {   
            //Convert it to integer value
            UInt64 tmp_key = routing_key.to_ulong();

            //Check validity against reserved values
            if (tmp_key != QAppNG::BROADCAST_ROUTING_KEY_VALUE && tmp_key != QAppNG::AUTOMATIC_ROUTING_KEY_VALUE)
            {
                m_has_routing_key = true; 
                m_routing_key = routing_key;
                return true;
            }
            else
            {
                return false;
            }
        }

        const std::bitset<64> getRemoteProcessorIds() const
        { 
            if (m_routing_key != QAppNG::AUTOMATIC_ROUTING_KEY_VALUE)
            {
                return m_routing_key;
            }
            else
            {
                return 0;
            }
        }

        // Ticket #8549
        // Context Key handling methods
        bool hasQObservableContextKey() const { return m_has_context_key; }
        void setQObservableContextKey( UInt64 context_key ) { m_has_context_key = true; m_context_key = context_key; }
        void resetQObservableContextKey() { m_has_context_key = false; m_context_key = 0; }
        UInt64 getQObservableContextKey() const { return m_context_key; }

        // to make QObservable sortable in Sorted LWS
        bool operator<( const QObservable & an_observable ) const
        {
            return getQObservableTimestamp() < an_observable.getQObservableTimestamp();
        }

        /**
        * getNetworkHeader:extract header from observables received or to be sent by/on the network
        * \param buf: buffer where to write extracted bytes
        * \param len: size of memory allocated for buffer
        * \return the number of bytes extracted form observable.
        */
        virtual size_t getNetworkHeader(UInt8* header_buffer_ptr, const size_t& max_header_length)
        {
            //#10873
            UNUSED(header_buffer_ptr);
            UNUSED(max_header_length);
            return 0;
        }

        virtual UInt8* getNetworkPayloadPtr()
        {
            return NULL;
        }

        virtual size_t getNetworkPayloadLength()
        {
            return 0;
        }

        void setNormAdditionalSequenceNumber( UInt8 seq ) { m_norm_optional_sequence_number = seq; };
        UInt8 getNormAdditionalSequenceNumber() { return m_norm_optional_sequence_number; };

    protected:
        QObservableType m_type;

    private:
        bool m_has_timestamp;
        bool m_has_routing_key;
        bool m_has_context_key;

        UInt32 m_timestamp_sec;
        UInt32 m_timestamp_nsec;
        // Ticket #8549: second level routing key added to handle inner routing in each thread
        UInt64 m_context_key;

        std::bitset<64> m_routing_key;

        //#11361 adding chunk sequence number. Only for IP pdus. This is needed to 
        // distinguish chunks belonging to the same pdu, thus having the same timestamp.
        // By using an incremental number and modifying observable norm to take into 
        // account this number, in addition to timestamp, correct ordering can be 
        // guaranteed even it two such chunks are sent to differente processing threads.
        UInt8 m_norm_optional_sequence_number;

        bool routingKeyValidator(UInt8 remote_id, bool value)
        {
            //Check index do not exceed available bytes
            if (remote_id >= 64)
            {
                return false;
            }

            //Store current key in a temporary copy
            m_routing_key.set(remote_id, value);

            switch (m_routing_key.to_ullong())
            {
                //Check against reserved values
                case QAppNG::AUTOMATIC_ROUTING_KEY_VALUE:
                case QAppNG::BROADCAST_ROUTING_KEY_VALUE:
                {
                    //Operation not allowed
                    m_routing_key.flip(remote_id);
                    return false;
                }

                default:
                {
                    m_has_routing_key = true;
                    return true;
                }
            }
        }
    };

}
#endif // INCLUDE_QOBSERBABLE_H_NG
// --------------------------------------------------------------------------------------------------------------------
// End of file
