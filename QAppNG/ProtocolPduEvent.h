#ifndef __PROTOCOL_PDU_EVENT_H___NG
#define __PROTOCOL_PDU_EVENT_H___NG

#include "CNlPdu.h"

#include <QAppNG/ProtocolEvent.h>

namespace QAppNG
{

    class ProtocolPduEvent : public ProtocolEvent
    {
    public:
        ProtocolPduEvent( std::shared_ptr< CNlPdu > a_nlPdu )
            : ProtocolEvent( e_protocolPduEvent )
        {
            m_nlPdu = a_nlPdu;
        }
        virtual ~ProtocolPduEvent(){};

        std::shared_ptr< CNlPdu > GetNlPdu()
        {
            return m_nlPdu;
        }

    private:

        std::shared_ptr< CNlPdu > m_nlPdu;
    };

}
#endif


