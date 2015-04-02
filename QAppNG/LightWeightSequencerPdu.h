#ifndef INCLUDE_LIGHTWEIGHTSEQUENCERPDU_NG
#define INCLUDE_LIGHTWEIGHTSEQUENCERPDU_NG
/** ===================================================================================================================
  * @file    LightWeightSequencerPdu HEADER FILE
  *
  * @brief   GsmApp Evolution - code refactoring project
  *          This class implement a light weight sequencer (LWS)
  *
  * @copyright
  *
  * @history
  * REF#        Who                                                              When          What
  * #3062       A. Della Villa, A. Manetti, F. Gragnani, R. Buti, S. Luceri      Jen-2009      Code Refactoring
  * #3775       Bennett Schneider                                                Nov-2009      Removed dispatcher thread
  * #????       A. Della Villa                                                   Mar-2010      Splitting in a new file
  *
  * @endhistory
  * ===================================================================================================================
  */

#include "LightWeightSequencerObservable.h"
#include "CNlPdu.h"

namespace QAppNG
{
    typedef LightWeightSequencerEvo< std::shared_ptr< CNlPdu >, std::function< void( std::shared_ptr< CNlPdu >& ) >, LightWeightSequencerQObservableNormFunctorEvo< std::tuple< UInt64, UInt8, std::shared_ptr< CNlPdu > > > > LightWeightSequencerPdu;
}

#endif // INCLUDE_LIGHTWEIGHTSEQUENCERPDU_NG
// --------------------------------------------------------------------------------------------------------------------
// End of file

