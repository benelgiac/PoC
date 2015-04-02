#ifndef FSM_INCLUDE_NG
#define FSM_INCLUDE_NG

#include <boost/mpl/fold.hpp>
#include <boost/mpl/filter_view.hpp>
#include <boost/type_traits/is_same.hpp>
#include <vector>
#include <ctime>
#include <boost/mpl/vector.hpp>

//#include <boost/mpl/placeholders.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/static_assert.hpp>
#include <cassert>

namespace mpl = boost::mpl;

namespace QAppNG
{
  typedef enum {FORWARD=0, BACKWARD, UNKNOWN} pdu_dir_t; //FORWARD is the direction in which the context was born (the direction of the IAM)
  
  template <class Derived> class state_machine;
  
  class default_event_dispatcher
  {
      template<class FSM, class Event>
      static int dispatch(
          state_machine<FSM>& m, int state, Event const& e)
      {
          return m.call_no_transition(state, e);
      }
  };
  
  
  template<class Table, class Event>
  struct generate_dispatcher;
  
  template<class Derived>
  class state_machine
  {
      // ...
  protected:
      template<
          int CurrentState
          , class Event
          , int NextState
          , void (Derived::*action)(Event const&)
      >
      struct row
      {
          // for later use by our metaprogram
          static int const current_state = CurrentState;
          static int const next_state = NextState;
          typedef Event event;
          typedef Derived fsm_t;
  
          // do the transition action.
          static void execute(Derived& fsm, Event const& e)
          {
              (fsm.*action)(e);
          }
      };
  
  
      friend class default_event_dispatcher;
  
      template <class Event>
      int call_no_transition(int state, Event const& e)
      {
          return static_cast<Derived*>(this)  // CRTP downcast
              ->no_transition(state, e);
      }
      // 
  public:
  
      template<class Event>
      int process_event(Event const& evt)
      {
          // generate the dispatcher type.
          typedef typename generate_dispatcher<
              typename Derived::transition_table, Event
          >::type dispatcher;
  
          // dispatch the event.
          this->state = dispatcher::dispatch(
              *static_cast<Derived*>(this)        // CRTP downcast
              , this->state
              , evt
              );
  
          // return the new state
          return this->state;
      }
  
      // ...
  protected:
      state_machine()
          : state(Derived::initial_state)
      {
      }
  
  private:
      int state;
      // ...
  
      // ...
  public:
      template <class Event>
      int no_transition(int state, Event const& e)
      {
          assert(false);
          return state;
      }
      // ...
      ////
  };
}
#include "detail/FSMImpl.h"

#endif