#if !defined ( SequencerExtractor_H_NG )
#define SequencerExtractor_H_NG
/** ===================================================================================================================
* @file    SequencerExctractor HEADER FILE
*
* @brief   GsmApp Evolution - code refactoring project
*          Simple class to handle status
*
* @copyright
*
* @history
* REF#        Who                                                              When          What
* #3525       A. Della Villa, F. Gragnani, R. Buti, S. Luceri      Feb-2009    Copied class from SequencerExctractor.h
* #5774       Stanislav Timinsky                                   Jan-2011    Added shutdown method
*
* @endhistory
* ===================================================================================================================
*/


#include "cal/Thread.h"
#include "cal/ThreadManager.h"
#include <QAppNG/nl_osal.h>
#include <cal/CommConnector.h>
#include <cal/Sequencer.h>
#include <QAppNG/QConfigManager.h>

namespace QAppNG
{

    /// @brief SequencerExtractor
    ///
    /// This class contains the definition of the SequencerExtractor class.
    /// This class is responsible for retrieving the next appropriate 
    /// Sequencable object from the Sequencer. 
    ///
    template <typename TSequenceable>
    class SequencerExtractor
    {
    public:

        typedef fastdelegate::FastDelegate1<TSequenceable, void> DelegateType; 
        typedef fastdelegate::FastDelegate0<void> InitDelegateType;

        SequencerExtractor( DelegateType a_delegate, cal::Sequencer<TSequenceable> &a_sequencer,
            const std::string & a_module_name, InitDelegateType a_init_delegate = NULL) : 
                m_module_name(a_module_name), m_delegate(a_delegate),
                m_init_delegate(a_init_delegate), m_sequencer(&a_sequencer)
        {
            cal::ThreadAttr attr;

            m_thread = cal::ThreadManager::Instance ().StartThread<void*> ( m_module_name.c_str(),
                attr,
                NULL,
                MakeDelegate(this, &SequencerExtractor::Initial),
                MakeDelegate(this, &SequencerExtractor::RuntimeFuncExtractFromSequencer));

            m_thread->SyncInit();
        };

        ~SequencerExtractor() 
        {
            if(m_thread.get())
            {
                m_thread->SetStopFlag(true);
                cal::ThreadManager::Instance().Join(m_thread);
            }
        };

        Int32 RuntimeFuncExtractFromSequencer(void *)
        {
            // Make sure the SyncInit knows we're ready
            cal::Thread::RunOnce( cal::time::Duration<MicroRes>(1));

            cal::Thread &this_thread = cal::Thread::Self();
   
            while(!(this_thread.GetStopFlag ()))
            {
                while (!m_sequencer->IsFetchSectorEmpty())
                {
                    TSequenceable sequencable = (TSequenceable) m_sequencer->FetchNext();
                    m_delegate(sequencable);
                } // end while FetchNext
            
                cal::Thread::RunOnce( cal::time::Duration<MicroRes>(1));
            }  // end permanent loop

            // cleanup sequencer (all sectors)  
            m_sequencer->FlushInput();  
          
            UInt64 total_objs =   
                m_sequencer->GetStats().GetCounter(cal::Sequencer<TSequenceable>::Stats_CurrentTotalObjects);  
          
            while ( total_objs > 0 )  
            {  
  
                while (!m_sequencer->IsFetchSectorEmpty())  
                {  
                    TSequenceable sequencable = (TSequenceable) m_sequencer->FetchNext();  
                    m_delegate(sequencable);  
                } // end while FetchNext  
   
                 total_objs =   
                     m_sequencer->GetStats().GetCounter(cal::Sequencer<TSequenceable>::Stats_CurrentTotalObjects);  
             }
          
             return 0;
        };

        Int32 Initial(void *)
        {
            if (m_init_delegate) m_init_delegate();

            UInt16 comm_queue_size = 0;
            QAppNG::QConfigManager::instance().getValue<UInt16>("main", "comm_queue_size", comm_queue_size);
            cal::CommConnector::Instance().ConnectQueues( comm_queue_size );

            return 0;
        }

        void shutdown() 
        {
            m_sequencer->FlushInput();

            UInt64 total_objs = 
                m_sequencer->GetStats().GetCounter(cal::Sequencer<TSequenceable>::Stats_CurrentTotalObjects);

            while ( total_objs > 0 )
            {
                while (!m_sequencer->IsFetchSectorEmpty())
                {
                    TSequenceable sequencable = (TSequenceable) m_sequencer->FetchNext();
                    m_delegate(sequencable);
                } // end while FetchNext

                total_objs = 
                    m_sequencer->GetStats().GetCounter(cal::Sequencer<TSequenceable>::Stats_CurrentTotalObjects);
            }
        }

        void Assign(DelegateType a_delegate) 
        {
            m_delegate = a_delegate;
        };

    private:
        std::string m_module_name;
        DelegateType m_delegate;
        InitDelegateType m_init_delegate;
        cal::Sequencer<TSequenceable> *m_sequencer;
        cal::Thread::SharedPtr m_thread;

    };

}
#endif // SequencerExtractor_H_NG

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  ...End of file */
