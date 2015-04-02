#ifndef SEQUENCEABLE_MULTI_QUEUE_H_NG
#define SEQUENCEABLE_MULTI_QUEUE_H_NG

#include <QAppNG/SequenceableMultiQueue.h>

namespace QAppNG
{

    /// @brief Constructor
    template<class TSequenceable>
    CSequenceableMultiQueue<TSequenceable>::CSequenceableMultiQueue(
        UInt64 a_quiet_timeout, 
        typename SeqMQueue::size_type a_queue_size, 
        Int64 a_flush_rate,
        UInt32 a_skew) :
            m_quietQueueTimeout(a_quiet_timeout),
            m_QueueSize(a_queue_size),
            m_QueueFlushRate(a_flush_rate),
            m_skew(a_skew),
            m_internal_stats_counters(InternalStats_EnumSize)
    {
        m_internal_stats_counters.AddStatsCounterTitle(std::string("SeqableInsertionFailures"), InternalStats_SequenceablesInsertionFailure);
        m_internal_stats_counters.AddStatsCounterTitle(std::string("SeqablesInserted"), InternalStats_SequenceablesInserted);
        m_internal_stats_counters.AddStatsCounterTitle(std::string("SeqablesDropped"), InternalStats_SequenceablesDropped);
        m_internal_stats_counters.AddStatsCounterTitle(std::string("SeqablesProcessed"), InternalStats_SequenceablesProcessed);
        m_internal_stats_counters.AddStatsCounterTitle(std::string("QueueCount"), InternalStats_QueueCount);

        m_currentSec = 0;
        m_currentNSec = 0;
        if (m_skew >= 1000000000)
            {
            m_skew = 999999999;
            }
    }

    /// @brief Destructor
    template<class TSequenceable>
    CSequenceableMultiQueue<TSequenceable>::~CSequenceableMultiQueue()
    {
    }

    /// @brief Creates a new queue. Should be called for each producer thread.
    ///        Each producer thread will be required to pass the returned queue
    ///        pointer to the AddToQueue call.
    ///        Will return an empty shared pointer if the queue could not be created 
    ///        for any reason.
    template<class TSequenceable>
    typename CSequenceableMultiQueue<TSequenceable>::SeqDQueuePtr CSequenceableMultiQueue<TSequenceable>::CreateQueue(void)
    {
        SeqDQueuePtr queue = SeqMQueue::CreateQueue(m_QueueSize, cal::time::Duration<MilliRes>(m_QueueFlushRate));
    
        m_internal_stats_counters.Increment(InternalStats_QueueCount, 1);

        return queue;
    }

    /// @brief Add a TSequenceable to a queue
    template<class TSequenceable>
    cal::Status CSequenceableMultiQueue<TSequenceable>::AddToQueue(TSequenceable &a_toEnqueue, SeqDQueuePtr &a_queueHandle)
    {
        if (a_queueHandle->IsValid() &&
            a_queueHandle->push_back(a_toEnqueue))
        {
            m_internal_stats_counters.Increment(InternalStats_SequenceablesInserted, 1);
            return cal::Status_Ok;
        }

        m_internal_stats_counters.Increment(InternalStats_SequenceablesInsertionFailure, 1);
        return cal::Status_Error;
    }

    /// @brief Process all queues. Overrides the MultiQueue virtual.
    template<class TSequenceable>
    void CSequenceableMultiQueue<TSequenceable>::ProcessQueues(void)
    {
        //  ...This should only be called from the consumer thread's context, via
        //  ...the m_rxEvent processing.
    
        //  ...Handle any control events first.
    
        SeqMQueue::ProcessControl();

        if ( SeqMQueue::m_read )
        {
            QueueNode *pNode = SeqMQueue::m_queues.GetFirst ( );

            QueueNode *earliestQueue = NULL;
            bool waitBecausePossiblyQuietQueue = false;

            // Purpose explained below at "if (unpoppedQueuesToInitialise)"
            bool unpoppedQueuesToInitialise = false;

            UInt32 earliestSec = std::numeric_limits<UInt32>::max();
            UInt32 earliestNSec = 0;
            UInt32 mostRecentEmptyQueueSec = 0;
            UInt32 mostRecentEmptyQueueNSec = 0;

            UInt32 numOfEmptyQueues = 0;
            UInt32 numOfReadyQueues = 0;
#ifdef BACK_METHOD_SUPPORTED
            UInt32 latestSec = 0;
            UInt32 latestNSec = 0;
#endif   
            // We will iterate through all the queues to find the one with the earliest
            //  sequenceable object.
            // This requires all queues to have at least one object on them so we can compare
            //  timestamps. If a queue has had no TSequenceables for a certain time, we assume
            //  that that queue has gone quiet and it is ok to continue processing from
            //  other queues.
            while ( pNode != NULL )
            {
                //  ...Get the next one, in case we delete this one.
                QueueNode *pNext = SeqMQueue::m_queues.GetNext ( pNode );
        
                //  ...Get a reference to the shared_ptr, so that the operator-> will work below.
                SeqDQueuePtr & qptr = *pNode;

                // Delete objects from queue that are "out of date"
                bool dataIsOutOfTime = true;
                while (!qptr->empty() && dataIsOutOfTime)
                {
                    TSequenceable& data = qptr->front();
                    // # 9559: changing sequenced data to QObservable
                    //if ((data->GetSec() < m_currentSec) ||
                    //    ((data->GetSec() == m_currentSec) && (data->GetNSec() < m_currentNSec)))
                    if ((data->getQObservableTimestampSec() < m_currentSec) ||
                        ((data->getQObservableTimestampSec() == m_currentSec) && (data->getQObservableTimestampNSec() < m_currentNSec)))
                    {
                        dataIsOutOfTime = true;
#ifdef DEBUG
                        // # 9559: changing sequenced data to QObservable
                        /*std::cerr<<"SQM: "<<data->GetSec()<<"."<<data->GetNSec()<<" "<<m_currentSec<<"."<<m_currentNSec<<std::endl;*/
                        std::cerr<<"SQM: "<<data->getQObservableTimestampSec()<<"."<<data->getQObservableTimestampNSec()<<" "<<m_currentSec<<"."<<m_currentNSec<<std::endl;
#endif
                        qptr->pop_front();
                        m_internal_stats_counters.Increment(InternalStats_SequenceablesDropped, 1);
                    }
                    else
                    {
                        dataIsOutOfTime = false;
                    }
                }


                // Check times of the data on the queue, as we need to find the "earliest" queue
                if (!qptr->empty())
                {
                    numOfReadyQueues++;

                    // look for earliest queue
                    TSequenceable& data = qptr->front();
                    // # 9559: changing sequenced data to QObservable
                    //if ((data->GetSec() < earliestSec) ||
                    //    ((data->GetSec() == earliestSec) && (data->GetNSec() < earliestNSec)))
                    //{
                    //    earliestSec = data->GetSec();
                    //    earliestNSec = data->GetNSec();
                    //    earliestQueue = pNode;
                    //}
                    if ((data->getQObservableTimestampSec() < earliestSec) ||
                        ((data->getQObservableTimestampSec() == earliestSec) && (data->getQObservableTimestampNSec() < earliestNSec)))
                    {
                        earliestSec = data->getQObservableTimestampSec();
                        earliestNSec = data->getQObservableTimestampNSec();
                        earliestQueue = pNode;
                    }

#ifdef BACK_METHOD_SUPPORTED
                    // TODO Here must have been a look for latest timestamp within 
                    // queues (for stop waiting on empty queue(s)),
                    // but the cal::NBQueue does not support back() operation
                    // So, the block is compiled out
                    data = qptr->back();
                    // # 9559: changing sequenced data to QObservable
                    //if ((data->GetSec() > latestSec) ||
                    //    ((data->GetSec() == latestSec) && (data->GetNSec() > latestNSec)))
                    //{
                    //    latestSec = data->GetSec();
                    //    latestNSec = data->GetNSec();
                    //}
                    if ((data->getQObservableTimestampSec() > latestSec) ||
                        ((data->getQObservableTimestampSec() == latestSec) && (data->getQObservableTimestampNSec() > latestNSec)))
                    {
                        latestSec = data->getQObservableTimestampSec();
                        latestNSec = data->getQObservableTimestampNSec();
                    }
#endif                
                }
                else // Queue is empty
                {
                    numOfEmptyQueues++;

                    // We must find the empty queue which had the most recent activity
                    if (qptr->m_initialized)
                    {
                        if ((qptr->m_timestamp.sec > mostRecentEmptyQueueSec) ||
                            ((qptr->m_timestamp.sec == mostRecentEmptyQueueSec) && (qptr->m_timestamp.nsec > mostRecentEmptyQueueNSec)))
                        {
                            mostRecentEmptyQueueSec = qptr->m_timestamp.sec;
                            mostRecentEmptyQueueNSec = qptr->m_timestamp.nsec;
                        }
                    }
                    else
                    {
                        // We've never popped anything from this queue
                        unpoppedQueuesToInitialise = true;
                    }
                }

                //  ...Next one.
                pNode = pNext;
            } // while(pNode != NULL)

            if (unpoppedQueuesToInitialise && (earliestQueue != NULL)) // (also test earliestSec to make sure we have a time to set it to!)
            {
                // During startup, when we see the very first object out of the multiqueue, we will (mistakenly) think
                // all other queues are quiet. Therefore, we mark these other queues as "quiet, with the last object received
                // at time X" (where X is the time of the object we have just pulled out of this queue. This will result in us
                // waiting until X + m_quietQueueTimeout before treating other queues as empty.

                // This is executed AT MOST ONCE each time a producer queue is created

                // Iterate through all the queues, if one has never been popped, initialise its "last traffic" timestamp

                SeqDQueueTimestamp ts;
                ts.sec = earliestSec;
                ts.nsec = earliestNSec;
                if (ts.nsec == 0)
                    {
                    ts.nsec = 999999999;
                    --ts.sec;
                    }
                else
                    {
                    --ts.nsec;
                    }

                if ((ts.sec > mostRecentEmptyQueueSec) ||
                    ((ts.sec == mostRecentEmptyQueueSec) && (ts.nsec > mostRecentEmptyQueueNSec)))
                {
                    mostRecentEmptyQueueSec = ts.sec;
                    mostRecentEmptyQueueNSec = ts.nsec;
                }
                pNode = SeqMQueue::m_queues.GetFirst ( );

                while ( pNode != NULL )
                {
                    QueueNode *pNext = SeqMQueue::m_queues.GetNext ( pNode );
                    if ( ! (*pNode)->m_initialized )
                    {
                        (*pNode)->m_timestamp = ts;
                        (*pNode)->m_initialized = true;
                    }
                    pNode = pNext;
                }
            }

            // figure out if must postpone reading from the earliest queue
            if (numOfEmptyQueues >0)
            {
                // If the the most recent empty queue's last activity was within X of the current object on the 
                // earliest queue, then we cannot assume (yet) that the empty queue is quiet. It could just be busy and
                // about to produce more for us. Therefore we must not continue. Yet.

#ifdef BACK_METHOD_SUPPORTED
                // how it must have been
                Int64 secNsecDiff = 
                    (((Int64)(latestSec - mostRecentEmptyQueueSec) * 1000000000) + ((Int64) latestNSec - mostRecentEmptyQueueNSec));
#else
                // this does not make much sense since new elements in non-empty queue(s) DO NOT change the diff
                // therefore, there is no good trigger for stop waiting still
                Int64 secNsecDiff = 
                    (((Int64)(earliestSec - mostRecentEmptyQueueSec) * 1000000000) + ((Int64) earliestNSec - mostRecentEmptyQueueNSec));
#endif
                if ( (mostRecentEmptyQueueSec > 0) &&
                    (secNsecDiff < static_cast<Int64>(m_quietQueueTimeout)) &&
                    (secNsecDiff > 0) ) // we test the diff is >0 in case we have processed something when we shouldn't have
                                       // perhaps some thread got blocked for longer than our m_quietQueueTimeout
                {
                    // We must wait to see if a queue has truly gone quiet and process another time
                    waitBecausePossiblyQuietQueue = true;
                }
                else
                {
                    // We may assume this queue has gone quiet and can continue processing other queues
                    waitBecausePossiblyQuietQueue = false;
                }
            }
        
            // process events if they are and no reason to wait
            if ((earliestQueue != NULL) &&
                (waitBecausePossiblyQuietQueue == false))
            {
                TSequenceable& data = (*earliestQueue)->front();

                SeqDQueueTimestamp ts;
                // # 9559: changing sequenced data to QObservable
                //ts.sec = data->GetSec();
                //ts.nsec = data->GetNSec();
                ts.sec = data->getQObservableTimestampSec();
                ts.nsec = data->getQObservableTimestampSec();

                (*earliestQueue)->m_timestamp = ts;

                // # 9559: changing sequenced data to QObservable
                m_currentSec = data->getQObservableTimestampSec();
                m_currentNSec = data->getQObservableTimestampNSec();
                if (m_currentSec > 0)
                    {
                    if (m_currentNSec < m_skew )
                        {
                        m_currentNSec += 1000000000 - m_skew;
                        --m_currentSec;
                        }
                    else
                        {
                        m_currentNSec -= m_skew;
                        }
                    }

                m_internal_stats_counters.Increment(InternalStats_SequenceablesProcessed, 1);

                //  ...Invoke callback for the queue.
                SeqMQueue::ProcessQueue(*earliestQueue);

                if (!(*earliestQueue)->empty() || (numOfReadyQueues > 1))
                {
                    // there are more events in the queue(s) to process
                    SeqMQueue::m_rxEvent.Raise();
                }
            }
        
        } // if (m_read)
    }


}
#endif //SEQUENCEABLE_MULTI_QUEUE_H_NG
