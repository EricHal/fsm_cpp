#ifndef FSM_H
#define FSM_H

/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 wisol technologie GmbH
 * With Portions Copyright (C) 2015 Eric Halère
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * \author    Michael Egli
 * \copyright wisol technologie GmbH
 * \date      18-Dec-2014
 *
 * \file fsm.h
 *
 * Finite State Machine
 * ====================
 *
 * Generic implementation of a finite state machine (FSM).
 *
 * Overview
 * --------
 *
 * A finite state machine is a machine that can be in any of a finite number of
 * states. At a time, it can only be in one state. In order to change the
 * machine to another state, a transition can be executed.
 *
 * A transition is initiated by a trigger. A trigger to the machine is only
 * evaluated for outgoing transitions of the current state. A transition can
 * have a guard and a transition action associated with it. The guard is a
 * function that checks if the transition should be executed. The transition
 * action is a function that is called when the transition is effectively
 * executed.
 *
 * If a trigger is sent to the machine, and more than one guard evaluate to
 * `true`, then one of the transitions is randomly chosen.
 *
 * A state can have any number of incoming and / or outgoing transitions
 * associated with it, but at least one transition must be present. There are
 * two special states.
 *
 * - Initial pseudo state (no incoming transition, one or more outgoing
 *   transitions).
 * - Final pseudo state (one or more incoming transitions, no outgoing
 *   transition).
 *
 * The initial pseudo state must be present in each FSM.
 *
 * Important concepts
 * ------------------
 *
 * Some concepts are important to remember when defining a state machine.
 *
 * - Each FSM must define a transition from the initial pseudo state to
 *   another state.
 * - If a guard is executed multiple times successively, it must return the
 *   same value every time (no side-effects in guards).
 * - A FSM is a purely reactive, and therefore passive component. In order to
 *   execute some actions, it must receive a trigger.
 *
 * Semantics
 * ---------
 *
 * The following semantic is implemented when a machine receives a trigger.
 *
 * - Consume a trigger.
 * - Check if the trigger potentially initiates some transitions from the
 *   current state. Otherwise return.
 * - For each transition found, check if its guard evaluates to `true`. Return
 *   if none is found.
 * - Execute the transition action of one (and only one) of the selected
 *   transition.
 * - Change current state to the state where the transition points to.
 *
 * Limitations
 * -----------
 *
 * This FSM implementation does not implement all concepts found in all
 * definitions of state machines (notably the UML definition). Some parts that
 * are omitted are.
 *
 * - do actions
 * - hierarchical nested states
 * - orthogonal regions
 *
 * Implementation
 * --------------
 *
 * This implementation can be used to implement a FSM. It enforces the
 * semantics described above, while being non-intrusive. All guards and actions
 * are defined in the client.
 *
 * The state machine and transitions can be conveniently defined with an array
 * of FSM::Trans structs. This makes the structure of the FSM very clear.
 *
 * C++11
 * -----
 *
 * The implementation uses some C++11 features. Therefore, in order to use the
 * code, the compiler must support theses features and C++11 must be enabled.
 *
 * Debug
 * -----
 *
 * It is possible to add a debug function in order to track state changes. The
 * debug function is either a `nullptr`, or of type `debugFn`. When the
 * function is defined, it is invoked with the `from_state`, `to_state`, and
 * `trigger` arguments whenever a state change happens.
 *
 * ~~~
 * // Define debug function
 * void dbg_fsm(State * from_state, State * to_state, Event * trigger) {
 *   std::cout << "changed from " << from_state << " to " << to_state << " with trigger " << trigger;
 * }
 * // Enable debug
 * fsm.add_debug_fn(dbg_fsm);
 * // Disable debug
 * fsm.add_debug_fn(nullptr);
 * ~~~
 *
 */

// Includes
#include <limits>
#include <map>
#include <vector>
#include <functional>
#include <assert.h>

// Forward declarations

/****************
 * The following namespace is orginally Copyright (c) 2014 wisol technologie GmbH
 * Thanks, Great implementation.
 *
 *
 * Eric Halère Added support to:
 * Enter and Exit function in for states.
 *
 * Capture the states as class (class State) as well as trigers (class Event).
 *
 * this will allow to pass information via the Event class to the transition action
 * each Event and State has an Identifier (unsigned int) set automatically during construction
 * special states (Fsm_Initial and Fsm_Final) are now static class instance inside fsm class
 * their Id should be 0 and 1 respectively as instance are created at launch of process.
 * to check equality of State instances or Event instances, it is recommended to test equality of their Id
 *
 * States Id and events Id are global, even if used in different fsm.
 *
 * Finally, I've separated declaration from implementation in different files for added classes.
 */


namespace FSM {
    
    void dealocateFSMStatic();
    
    enum Fsm_Errors {
        // Success
        Fsm_Success = 0,
        
        // Warnings
        // The current state has not such trigger associated.
        Fsm_NoMatchingTrigger,
        
        // Errors
        // The state machine has not been initialized. Call init().
        Fsm_NotInitialized,
    };
    
    // EricHal added: an event class instead of a char
    // this class should be derived to include information about trigger (renamed event)
    class Event {
        public :
        // 'tor
        Event();
        // get the Event ID
        unsigned int getID();
    private:
        static unsigned int __current_id;
        unsigned int m_id;
    };
    
    // EricHal added: an state class instead of an int
    // this class should be derived to include information about state
    
    // Defines the function prototype for enter and exit function.
    using stateFn = std::function<void()>;
    
    class State {
        public :
        // 'tor
        State();
        // get the state ID
        unsigned int getID();
        
        // set the enter and exit functions (set to nullptr to unset it)
        void setEnterFunction(stateFn iFn);
        void setExitFunction(stateFn iFn);
        
        // invoke the enter or exit function.
        void invokeEnterFunction();
        void invokeExitFunction();
        
    private:
        static unsigned int __current_id;
        unsigned int m_id;
        stateFn m_enterFn;
        stateFn m_exitFn;
    };
    
    // Defines the function prototype for a guard function.
    using guardFn = std::function<bool()>;
    // Defines the function prototype for an action function.
    // EricHal added: a event pointer to pass information to action
    using actionFn = std::function<void(Event *)>;
    // Defines the function prototype for a debug function.
    // Parameters are: from_state, to_state, trigger
    using debugFn = std::function<void(State *,State *,Event *)>;
    
    /**
     * Defines a transition between two states.
     */
    struct Trans {
        State * from_state;
        State * to_state;
        Event * trigger;
        guardFn guard;
        actionFn action;
    };
    
    /**
     * An generic finite state machine (FSM) implementation.
     */
    class Fsm {
        
        // Definitions for the structure that holds the transitions.
        // For good performance on state machines with many transitions, transitions
        // are stored for each `from_state`:
        //   map<from_state, vector<Trans> >
        using transition_elem_t = std::vector<Trans>;
        using transitions_t = std::map<int, transition_elem_t>;
        transitions_t m_transitions;
        // Current state.
        State * m_cs;
        bool m_initialized;
        debugFn m_debug_fn;
        
    public:
        
        /**
         * A list of predefined pseudo states.
         */
        static State * Fsm_Initial;
        static State * Fsm_Final;
        
        // Constructor.
        Fsm() : m_transitions(), m_cs(0), m_initialized(false), m_debug_fn(nullptr) {atexit(dealocateFSMStatic);}
        Fsm(const Fsm & orig);
        /**
         * Initializes the FSM.
         *
         * This sets the current state to Fsm_Initial.
         * Once the fsm has been initialized, calling this function has no effect.
         */
        void init()
        {
            if(!m_initialized) {
                m_cs = Fsm_Initial;
                m_initialized = true;
            }
        }
        
        /**
         * Set the machine to uninitialized and the state to Fsm_Initial.
         *
         * This method can be called at any time. After a reset, init() must be
         * called in order to use the machine.
         */
        void reset()
        {
            m_cs = Fsm_Initial;
            m_initialized = false;
        }
        
        /**
         * Add a set of transition definitions to the state machine.
         *
         * This function can be called multiple times at any time. Added
         * transitions cannot be removed from the machine.
         */
        template<typename InputIt>
        void add_transitions(InputIt start, InputIt end)
        {
            InputIt it = start;
            for(; it != end; ++it) {
                // Add element in the transition table
                m_transitions[(*it).from_state->getID()].push_back(*it);
            }
        }
        
        /**
         * Overloaded method to add transitions to the state machine.
         *
         * This method takes a collection and adds all its elements to the list of
         * transitions.
         */
        template<typename Coll>
        void add_transitions(Coll&& c)
        {
            add_transitions(std::begin(c), std::end(c));
        }
        
        /**
         * Overloaded method to add transitions to the state machine.
         *
         * This method takes a initializer list and adds all its elements to the list
         * of transitions.
         *
         * This is very convenient, because it avoids the creation of an unnecessary
         * temporary object. Usage is like the following.
         *
         * ~~~
         * FSM::Fsm fsm;
         * fsm.add_transitions({
         *   { stateA, stateB, 'a', []{...}, nullptr },
         *   { stateB, stateC, 'b', nullptr, []{...} },
         * });
         * ~~~
         */
        void add_transitions(std::initializer_list<Trans>&& i)
        {
            
            add_transitions(std::begin(i), std::end(i));
        }
        
        /**
         * Adds a function that is called on every state change. The type of the
         * function is `debugFn`. It has the following parameters.
         *
         * - from_state (int)
         * - to_state (int)
         * - trigger (char)
         *
         * It can be used for debugging purposes. It can be enabled and disabled at
         * runtime. In order to enable it, pass a valid function pointer. In order
         * to disable it, pass `nullptr` to this function.
         */
        void add_debug_fn(debugFn fn)
        {
            m_debug_fn = fn;
        }
        
        /**
         * Execute the given trigger according to the semantics defined for this
         * state machine.
         *
         * Returns the status of the execute operation. Fsm_Success is 0.
         */
        Fsm_Errors execute(Event * trigger)
        {
            if(not m_initialized) {
                return Fsm_NotInitialized;
            }
            
            Fsm_Errors err_code = Fsm_NoMatchingTrigger;
            
            const auto state_transitions = m_transitions.find(m_cs->getID());
            if(state_transitions == m_transitions.end()) {
                return err_code; // No transition from current state found.
            }
            
            // iterate the transitions
            const transition_elem_t& active_transitions = state_transitions->second;
            for(auto& transition : active_transitions) {
                
                // Check if trigger matches.
                if(trigger->getID() != (transition.trigger)->getID()) continue;
                err_code = Fsm_Success;
                
                // Check if guard exists and returns true.
                if(transition.guard && (not transition.guard())) continue;
                
                // Now we have to take the action and set the new state.
                // Then we are done.
                
                // Check if action exists and execute it.
                if(transition.action != 0) {
                    transition.action(trigger); //execute action
                }
                
                transition.from_state->invokeExitFunction();
                m_cs = transition.to_state;
                transition.to_state->invokeEnterFunction();
                
                if(m_debug_fn) {
                    m_debug_fn(transition.from_state, transition.to_state, trigger);
                }
                break;
            }
            
            return err_code;
        }
        
        /**
         * Returns the current state;
         */
        State * state() const { return m_cs; }
        /**
         * Returns whether the current state is the initial state.
         */
        bool is_initial() const { return (m_cs->getID() == Fsm_Initial->getID()); }
        /**
         * Returns whether the current state is the final state.
         */
        bool is_final() const { return (m_cs->getID() == Fsm_Final->getID()); }
    };
    
} // end namespace FSM

#endif // FSM_H
