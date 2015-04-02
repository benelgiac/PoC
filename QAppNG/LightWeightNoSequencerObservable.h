#ifndef INCLUDE_LIGHTWEIGHTNOSEQUENCEROBSERVABLE_H_NG
#define INCLUDE_LIGHTWEIGHTNOSEQUENCEROBSERVABLE_H_NG
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

// For EventHandler Functor
#include "LightWeightSequencerObservable.h"

// Observables
#include "QObservable.h"

namespace QAppNG
{
    struct RoundRobinFakeNormFunctor
    {
        bool operator() (UInt64& norm_value, UInt8& norm_properties, std::shared_ptr< QObservable >& observale)
        {
            static UInt64 trick_for_round_robin_queue_reading(0);
            norm_value = ++trick_for_round_robin_queue_reading;
            norm_properties = static_cast<UInt8>( SequenceableNormProperties::ELEMENT_NORM_VALUE );

            return true;
        }
    };

    // --------------------------------------------------------------------------------------------------------------------

    typedef LightWeightSequencerEvo< std::shared_ptr< QObservable >, std::function< void( std::shared_ptr< QObservable >& ) >, RoundRobinFakeNormFunctor, LightWeightSequencerQObservableEventHandler > LightWeightNoSequencerObservable;

    // --------------------------------------------------------------------------------------------------------------------
}

#endif // INCLUDE_LIGHTWEIGHTNOSEQUENCEROBSERVABLE_H_NG
// --------------------------------------------------------------------------------------------------------------------
// End of file

