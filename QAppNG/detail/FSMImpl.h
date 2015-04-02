#ifndef FSM_IMPL_H_NG 
#define FSM_IMPL_H_NG

namespace QAppNG
{
    template<
        class Transition
            , class Next
    >
    struct event_dispatcher
    {
        typedef typename Transition::fsm_t fsm_t;
        typedef typename Transition::event event;

        static int dispatch(
            fsm_t& fsm, int state, event const& e)
        {
            if (state == Transition::current_state)
            {
                Transition::execute(fsm, e);
                return Transition::next_state;
            }
            else // move on to the next node in the chain.
            {
                return Next::dispatch(fsm, state, e);
            }
        }
    };





    // get the Event associated with a transition.
    template <class Transition>
    struct transition_event
    {
        typedef typename Transition::event type;
    };

    template<class Table, class Event>
    struct generate_dispatcher
        : mpl::fold<
        mpl::filter_view<   // select rows triggered by Event
        Table
        , boost::is_same<Event, transition_event<mpl::_1> >
        >
        , default_event_dispatcher
        , event_dispatcher<mpl::_2,mpl::_1> 
        >
    {};
}
#endif //FSM_IMPL_H_NG