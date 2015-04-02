#ifndef INCLUDE_LIGHTWEIGHTSEQUENCEROBSERVABLE_H_NG
#define INCLUDE_LIGHTWEIGHTSEQUENCEROBSERVABLE_H_NG
/** ===================================================================================================================
  * @file    TicketProcessor HEADER FILE
  *
  * @brief   Iub ReArch & porting to evolution
  *
  * @copyright
  *
  * @history
  * REF#        Who                                                              When          What
  * #5022       A. Della Villa, F. Gragnani, S. Luceri                           Jun-2010      Original Development
  *
  * @endhistory
  * ===================================================================================================================
  */

// base Sequencer Class
#include "LightWeightSequencerEvo.h"


// Observables
#include "QObservable.h"

// QVirtualClock Events
#include "QVirtualClock.h"

namespace QAppNG
{
    template< typename SEQUENCEABLE_CLASS >
    struct LightWeightSequencerQObservableNormFunctorEvo;

    // --------------------------------------------------------------------------------------------------------------------

    template< typename SEQUENCEABLE_POINTER_TYPE >
    struct LightWeightSequencerQObservableNormFunctorEvo< std::tuple< UInt64, UInt8, SEQUENCEABLE_POINTER_TYPE > >
    {
        bool operator() (UInt64& norm_value, UInt8& norm_properties, SEQUENCEABLE_POINTER_TYPE& element_to_evalutate)
        {
            switch ( element_to_evalutate->GetType() )
            {
                case PDU:
                case TIMESTAMP_PROCEDURE:
                case IU_PROCEDURE:
                case IUB_PROCEDURE:
                case GENERIC_TICKET:
                case IUB_TICKET:
                case IU_TICKET:
                case MM_TICKET:
                case GMM_TICKET:
                case SM_TICKET:
                case VOIP_TICKET:
                case UID_PDU:
                case TCP_XDR_TICKET:
                case GB_CONTROLPLANE_INFO:
                case GB_PAGING_TICKET:
                case GB_INTERFACE_TICKET:
                case BSSGP_TICKET:
                case IUPS_CONTROLPLANE_INFO:
                case SS7_CONTROLPLANE_INFO:
                case IUB_HSDPA_CONTROLPLANE_INFO:
                case XDR_TICKET_FIELDS:
                case TMSI_COMM_OBSERVABLE:
                case ABIS_EVENT:
                case A_EVENT:
                {
                    assert(element_to_evalutate->hasQObservableTimestamp());
                    norm_value = element_to_evalutate->getQObservableTimestamp();
                    norm_properties = static_cast<UInt8>( SequenceableNormProperties::ELEMENT_NORM_VALUE );
                    break;
                }
                case REMOTE_PDU:
                case UNACCURATE_TS_PDU:
                case OUTPUT_TICKET:
                case SDP_EVENT_OBS:
                case MOS_MESSAGE_OBS:
                case MOS_REPORT_OBS:
                case STRING_OBS:
                {
                    norm_value = 0;
                    norm_properties = static_cast<UInt8>( SequenceableNormProperties::ELEMENT_TO_INSTANT_PASS );
                    break;
                }
                // #8060
                // All EVENTs have maximum priority
                case IUB_USER_CONTEXT_EVENT:
                case IU_CHANNEL_INFO:
                case CELL_CONTEXT_CONFIG:
                case GB_UNUSED_CONTEXT_ID:
                case LTE_UNUSED_CONTEXT_ID:
                case LTE_UPDATE_USER_CONTEXT_ID:
                case LTE_DELETE_USER_CONTEXT_ID:
                case SLAB_WRITER_EVENT:
                case TMSI_MGR_EVENT:
                {
                    norm_value = 0;
                    norm_properties = static_cast<UInt8>( SequenceableNormProperties::EVENT_TO_INSTANT_PASS );
                    break;
                }
                case QVIRTUALCLOCK_TIME_PULSE:
                case QVIRTUALCLOCK_APPLICATION_START:
                case QVIRTUALCLOCK_APPLICATION_SHUTDOWN:
                {
                    //return INSTANT_ELEMENT_PASS_NORM_VALUE;
                    assert(element_to_evalutate->hasQObservableTimestamp());
                    norm_value = element_to_evalutate->getQObservableTimestamp();
                    norm_properties = static_cast<UInt8>( SequenceableNormProperties::EVENT_NORM_VALUE );
                    break;
                }
                default:
                {
                    // unsupported observable type
                    assert(0);
                    norm_value = 0;
                    norm_properties = static_cast<UInt8>( SequenceableNormProperties::UNDEFINED_NORM );
                    return false;
                }
            } // end switch

            return true;
        }
    };

    // --------------------------------------------------------------------------------------------------------------------

    struct LightWeightSequencerQObservableEventHandler
    {
        template< typename SEQUENCER_CLASS >
        bool operator() ( SEQUENCER_CLASS& sequencer, size_t queue_index, std::unique_ptr< QObservable >& event_to_evaluate )
        {
            bool result(false);

            switch ( event_to_evaluate->GetType() )
            {
                case QVIRTUALCLOCK_TIME_PULSE:
                {
                    QVirtualClockTimePulseEvent* time_pulse_event( static_cast< QVirtualClockTimePulseEvent* >( event_to_evaluate.get() ) );

                    // This particular event is issued just before QVIRTUALCLOCK_APPLICATION_SHUTDOWN
                    if ( time_pulse_event->hasEndOfTimeTrigger() )
                    {
                        // Enable Sequencer Flushing Mode
                        //sequencer.m_lws_state = typename SEQUENCER_CLASS::eLwsFlushingState;
                        sequencer.setFlushingMode();
                    }

                    result = sequencer.push( queue_index, event_to_evaluate );

                    break;
                }
                case QVIRTUALCLOCK_APPLICATION_START:
                {
                    result = sequencer.push( queue_index, event_to_evaluate );

                    break;
                }
                case QVIRTUALCLOCK_APPLICATION_SHUTDOWN:
                {
                    result = sequencer.push(queue_index, event_to_evaluate);

                    if ( result )
                    {
                        sequencer.shutdown();
                    }

                    break;
                }
                default:
                {
                    assert(0);

                    break;
                }
            }

            return result;
        }

        //// Legacy method to be removed when boost pointers are replaced with standard ones
        template< typename SEQUENCER_CLASS >
        bool operator() ( SEQUENCER_CLASS& sequencer, size_t queue_index, std::shared_ptr< QObservable >& event_to_evaluate )
        {
            bool result(false);

            switch (event_to_evaluate->GetType())
            {
                case QVIRTUALCLOCK_TIME_PULSE:
                {
                    std::shared_ptr< QVirtualClockTimePulseEvent > time_pulse_event( std::static_pointer_cast< QVirtualClockTimePulseEvent >( event_to_evaluate ) );

                    // This particular event is issued just before QVIRTUALCLOCK_APPLICATION_SHUTDOWN
                    if ( time_pulse_event->hasEndOfTimeTrigger() )
                    {
                        // Enable Sequencer Flushing Mode
                        //sequencer.m_lws_state = typename SEQUENCER_CLASS::eLwsFlushingState;
                        sequencer.setFlushingMode();
                    }

                    result = sequencer.push( queue_index, event_to_evaluate );

                    break;
                }
                case QVIRTUALCLOCK_APPLICATION_START:
                {
                    result = sequencer.push(queue_index, event_to_evaluate);

                    break;
                }
                case QVIRTUALCLOCK_APPLICATION_SHUTDOWN:
                {
                    result = sequencer.push(queue_index, event_to_evaluate);

                    if ( result )
                    {
                        sequencer.shutdown();
                    }

                    break;
                }
                default:
                {
                    assert(0);

                    break;
                }
            }

            return result;
        }
    };

    // --------------------------------------------------------------------------------------------------------------------

    typedef LightWeightSequencerEvo< std::unique_ptr< QObservable > , std::function< void( std::unique_ptr< QObservable >& ) >, LightWeightSequencerQObservableNormFunctorEvo< std::tuple< UInt64, UInt8, std::unique_ptr< QObservable > > >, LightWeightSequencerQObservableEventHandler > LightWeightSequencerUniqueObservableEvo;
    typedef LightWeightSequencerEvo< std::unique_ptr< QObservable > , fastdelegate::FastDelegate1< std::unique_ptr< QObservable >& , void >, LightWeightSequencerQObservableNormFunctorEvo< std::tuple< UInt64, UInt8, std::unique_ptr< QObservable > > >, LightWeightSequencerQObservableEventHandler > LightWeightSequencerUniqueObservableEvoDelegate;

    typedef LightWeightSequencerEvo< std::shared_ptr< QObservable > , std::function< void( std::shared_ptr< QObservable >& ) >, LightWeightSequencerQObservableNormFunctorEvo< std::tuple< UInt64, UInt8, std::shared_ptr< QObservable > > >, LightWeightSequencerQObservableEventHandler > LightWeightSequencerObservableEvo;
}
// --------------------------------------------------------------------------------------------------------------------
#endif // INCLUDE_LIGHTWEIGHTSEQUENCEROBSERVABLE_H_NG
// --------------------------------------------------------------------------------------------------------------------
// End of file
// --------------------------------------------------------------------------------------------------------------------

