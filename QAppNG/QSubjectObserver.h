#ifndef INCLUDE_QSUBJECTOBSERVER_H
#define INCLUDE_QSUBJECTOBSERVER_H
/** ===================================================================================================================
* @file    QSubject Observer HEADER FILE
*
* @brief   Simple Subject/Observer Pattern Implementation
*
* @copyright
*
* @history
* REF#        Who                                                              When          What
* #8064       A. Della Villa, D. Verna, F. Lasagni                             Jen-2013      original development
*
* @endhistory
* ===================================================================================================================
*/

// STL/Boost includes
#include <vector>

namespace QAppNG
{
    // forward declarations
    template<typename OBSERVABLE_CLASS, size_t NUMBER_OF_SLOTS = 1> class QSubject;
    template<typename OBSERVABLE_CLASS, size_t NUMBER_OF_SLOTS = 1> class QObserver;

    // --------------------------------------------------------------------------------------------------------------------

    // QSubject PARTIAL SPECIALIZATION for SHARED POINTERS
    template<typename OBSERVABLE_CLASS, size_t NUMBER_OF_SLOTS>
    class QSubject< std::shared_ptr<OBSERVABLE_CLASS>, NUMBER_OF_SLOTS >
    {
    public:
        // CTOR
        QSubject()
        {
            for ( size_t slot_number = 0; slot_number < NUMBER_OF_SLOTS; slot_number++ )
            {
                //m_per_slot_observers_size[slot_number] = 0;
                m_per_slot_observers_size.push_back(0);
            }

            m_per_slot_observers.resize( NUMBER_OF_SLOTS );
        }

        // DTOR
        virtual ~QSubject() {}

        void registerToObserver( QObserver< std::shared_ptr<OBSERVABLE_CLASS> > * observer, size_t slot_number = 0 )
        {
            // take pointer to observer and store it
            m_per_slot_observers[slot_number].push_back( observer );

            // increment number of observers
            m_per_slot_observers_size[slot_number] = m_per_slot_observers[slot_number].size();
        }

        void unregister( size_t slot_number = 0 )
        {
            m_per_slot_observers_size[slot_number] = 0;
            m_per_slot_observers[slot_number].clear();
        }

        void unregisterAllSlots()
        {
            // TO BE IMPLEMENTED
        }

        void notifyObservable( std::shared_ptr<OBSERVABLE_CLASS> & observable, size_t slot_number = 0 )
        {
            for ( size_t i = 0; i < m_per_slot_observers_size[slot_number]; i++ )
            {
                (*m_per_slot_observers[slot_number][i]).receiveObservable(observable);
            }
        }

        bool hasObservers( size_t slot_number = 0 )
        {
            return !m_per_slot_observers[slot_number].empty();
        }

    private:
        std::vector< std::vector<QObserver< std::shared_ptr<OBSERVABLE_CLASS> > *> > m_per_slot_observers;
        std::vector<size_t> m_per_slot_observers_size;
    };

    // --------------------------------------------------------------------------------------------------------------------

    // QObserver PARTIAL SPECIALIZATION for SHARED POINTERS whit just ONE SLOT
    template<typename OBSERVABLE_CLASS, size_t NUMBER_OF_SLOTS>
    class QObserver< std::shared_ptr<OBSERVABLE_CLASS>, NUMBER_OF_SLOTS >
    {
    public:
        virtual void receiveObservable( std::shared_ptr<OBSERVABLE_CLASS> & observable ) = 0;
    };

    // --------------------------------------------------------------------------------------------------------------------

    // QSubject PARTIAL SPECIALIZATION for POINTERS
    template<typename OBSERVABLE_CLASS>
    class QSubject< OBSERVABLE_CLASS* >
    {
        // TODO if needed...
    };

    // --------------------------------------------------------------------------------------------------------------------

    // QObserver PARTIAL SPECIALIZATION for POINTERS
    template<typename OBSERVABLE_CLASS>
    class QObserver< OBSERVABLE_CLASS* >
    {
    public:
        virtual void receiveObservable( OBSERVABLE_CLASS & observable ) = 0;
    };

    // --------------------------------------------------------------------------------------------------------------------
}

// --------------------------------------------------------------------------------------------------------------------
#endif // QSUBJECTOBSERVER
// --------------------------------------------------------------------------------------------------------------------
// End of file

