#if !defined (SEQUENCEABLE_MULTIQUEUE_H_NG)
#define SEQUENCEABLE_MULTIQUEUE_H_NG
// ===========================================================================
/// @file
/// @brief This file contains an implementation of a multiqueue which takes
///        Sequenceable objects, and, assuming each producer thread produces its
///        consumables in time order in relation to its own thread, will pass
///        the consumables to the consumer thread in time order.
///
///        If a queue goes quiet, processing will STOP until m_quietQueueTimeout (in nanoseconds)
///        passes, in order to verify that the queue has actually gone quiet.
///
///        IMPORTANT: If you have one or more queues which do go quiet, this can affect
///                   performance! Choose your m_quietQueueTimeout wisely!
///                   The SequenceableMultiQueue is intended to be used in situations where
///                   each input queue is reasonably evenly balanced, load-wise.
///
///        IMPORTANT: Sequenceables which arrive OUT OF TIME ORDER will be DISCARDED. An
///                   internal stats counter will be incremented, however the insertion
///                   WILL succeed! Each input thread MUST produce IN TIME ORDER with
///                   respect to itself. If a thread goes quiet and comes back later and
///                   its Sequenceables are earlier in time than the current Sequenceables,
///                   they will be DISCARDED until it has caught up. Again, you'll only
///                   know via the internal stats counter!!
///
/// @copyright
/// @history
/// Reference   Who                 When        What
/// 2032        David O'Loghlin     26-Jun-2008 Original development
/// @endhistory
///
// ===========================================================================

/*  ...Include files
*/
#include <stdio.h>
#include <string>
#include <sstream>
#include <vector>

#include <cal/defs.h>
#include <cal/MultiQueue.h>
#include <cal/DQueue.h>
#include <cal/HashMap.h>
#include <cal/StatsCounters.h>
#define DEFAULT_SEQUENCEABLE_MULTIQUEUE_QUEUE_SIZE  1000
#define DEFAULT_SEQUENCEABLE_MULTIQUEUE_FLUSH_RATE  1000
#define DEFAULT_SEQUENCEABLE_MULTIQUEUE_SKEW        125000 //Same as sequencer

namespace QAppNG
{
    ///@brief SequenceableQueue (DQueue + timestamp
    ///
    /// This serves as both the DQueue and the Traits object for the SequenceableMultiQueue.
    /// Since it includes the timestamp, it removes the need for a hashmap of pointers to timestamps
    /// the previous implementation needed.
    template<class DATA>
    class SequenceableQueue : public cal::DQueue<DATA>
        {
    public:
        SequenceableQueue( std::size_t a_numElements) : cal::DQueue<DATA>( a_numElements) { m_timestamp.sec = 0; m_timestamp.nsec = 0; m_initialized = false; };
    
        struct SeqDQueueTimestamp
        {
            UInt32 sec;
            UInt32 nsec;
        };
        
        SeqDQueueTimestamp m_timestamp;
        bool               m_initialized;
    
        typedef SequenceableQueue<DATA>      QueueType;
        typedef std::shared_ptr<QueueType> QueuePtr;
        };
    
    
    ///@brief SequenceableMultiQueue
    ///
    /// This requires that the streams be relatively constant in data production
    /// There isn't a provision to timeout a queue, despite the presence of the nsec_timeout constructor field.
    ///@note BDS-The lack of timeout seems to be an oversight, but I'm hesitant to correct it as it will add overhead, 
    ///      plus this class is already redundant with the LWS.  If the queue idle time is necessary, use the LWS.
    template<class TSequenceable>
    class CSequenceableMultiQueue :
        public cal::MultiQueue<TSequenceable, SequenceableQueue<TSequenceable> >
    {
    public:
    
        typedef cal::MultiQueue<TSequenceable, SequenceableQueue<TSequenceable> > SeqMQueue;
        typedef typename SeqMQueue::QueuePtr SeqDQueuePtr;
    
        enum InternalStats
        {
            InternalStats_SequenceablesInsertionFailure = 0,
            InternalStats_SequenceablesInserted,
            InternalStats_SequenceablesDropped,
            InternalStats_SequenceablesProcessed,
            InternalStats_QueueCount,
            InternalStats_EnumSize
        };
    
        // internal stats accessor
        inline cal::StatsCounters<UInt64, enum InternalStats, cal::FastLock>& GetInternalStatsCounters()
        {
            return m_internal_stats_counters;
        };
    
        /// @brief Constructor
        CSequenceableMultiQueue(
            UInt64 a_nsec_timeout,
            typename SeqMQueue::size_type a_queue_size = DEFAULT_SEQUENCEABLE_MULTIQUEUE_QUEUE_SIZE,
            Int64 a_flush_rate = DEFAULT_SEQUENCEABLE_MULTIQUEUE_FLUSH_RATE,
            UInt32 a_skew = DEFAULT_SEQUENCEABLE_MULTIQUEUE_SKEW);
        
        /// @brief Destructor
        ~CSequenceableMultiQueue();
    
        /// @brief Creates a new queue. Should be called for each producer thread.
        ///        Each producer thread will be required to pass the returned queue
        ///        pointer to the AddToQueue call.
        SeqDQueuePtr CreateQueue(void);
    
        /// @brief Add a TSequenceable to a queue
        cal::Status AddToQueue(TSequenceable &a_toEnqueue, SeqDQueuePtr &a_queueHandle);
    
    protected:
        /// @brief Process all queues. Overrides the MultiQueue virtual.
        virtual void ProcessQueues(void);
    
    private:
        UInt64               m_quietQueueTimeout;
        
        typename SeqMQueue::size_type m_QueueSize;
        Int64                m_QueueFlushRate;
    
    
        typedef typename SeqMQueue::QueueNode QueueNode;
        typedef typename SequenceableQueue<TSequenceable>::SeqDQueueTimestamp SeqDQueueTimestamp;
        
        UInt32 m_currentSec, m_currentNSec;
        UInt32 m_skew;
    
        // Internal stats counters
        cal::StatsCounters<UInt64, enum InternalStats, cal::FastLock> m_internal_stats_counters;
    };
}
#include "detail/SequenceableMultiQueueImpl.h"

#endif // end SEQUENCEABLE_MULTIQUEUE_H_NG


