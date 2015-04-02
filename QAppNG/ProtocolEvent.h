#ifndef __PROTOCOL_EVENT_H___NG
#define __PROTOCOL_EVENT_H___NG

#include <QAppNG/nl_consumable.h>

namespace QAppNG
{

    class ProtocolEvent:
        public CNlConsumable
    {
    public:
        typedef enum {
            e_unknownEvent,
            e_protocolPduEvent,
            e_mmEvent,
            e_gmmEvent,
            e_userPlaneEvent,
            e_userPlaneIuPS_RABEvent,
        } EventType;

        ProtocolEvent( EventType a_eventType ):
            cal::threadpool::Consumable<EConsumableType>(ePROTOCOL_EVENT_CONSUMABLE)
        {
            m_eventType = a_eventType;
        };
        virtual ~ProtocolEvent(){};

        inline EventType GetEventType()
        {
            return m_eventType;
        }

        inline UInt64 GetCallId ()
        {
            return m_callId;
        }

        void SetCallId(UInt64 call_id)
        {
            m_callId = call_id;
        }

    private:
        ProtocolEvent()
        {
            m_eventType = e_unknownEvent;
        };
        EventType m_eventType;

        UInt64 m_callId;
    };

}
#endif

