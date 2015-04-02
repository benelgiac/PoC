#ifndef __TICKET_LABEL___NG
#define __TICKET_LABEL___NG

#include <cal/defs.h>
#include "Ticket.h"
#include "TicketIndex.h"

namespace QAppNG
{

    class TicketLabel
    {
    public:
        TicketLabel() {};
        virtual ~TicketLabel() {};
        virtual void LabelTicket(std::shared_ptr< Ticket > a_ticket) { CAL_UNUSED_ARG( a_ticket ); };
        virtual UInt8 GetNumActivePdpCtx(){return 0;};
        virtual void SetNumActivePdpCtx(const UInt8 a_num_pdp_ctx){ CAL_UNUSED_ARG(a_num_pdp_ctx); };
        virtual void UpdateLabel(std::shared_ptr<TicketLabel> newLabel) { CAL_UNUSED_ARG( newLabel ); };
    };

}
#endif //__TICKET_LABEL___NG

