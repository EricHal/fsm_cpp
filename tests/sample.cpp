/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Eric Halère
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
 * Example
 * -------
 *
 * The following example implements this simple state machine.
 *
 * ~~~
 *   ---------------                    ----------                              -------------
 *   | Fsm_Initial | -- 'a'/action1 --> | stateA | -- [guard2]'b' / action2 --> | Fsm_Final |
 *   ---------------                    ----------                              -------------
 * ~~~
 *
 */
//
//  Created by Halere Eric on 06/05/2015.
//

#include <iostream>
#include "../include/fsm.h"
#include <vector>

void action1(FSM::Event * evt) { std::cout << "perform custom action 1 with event ID:" << evt->getID() << "\n"; }
bool guard2()  { return true; }
void action2(FSM::Event * evt) { std::cout << "perform custom action 2 with event ID:" << evt->getID() << "\n"; }
void enterFn() { std::cout << "entering stateA \n"; }
void exitFn() { std::cout << "exiting stateA \n"; }
void dbg_fsm(FSM::State * from_state, FSM::State * to_state, FSM::Event * trigger) {
    std::cout << "changed from " << from_state->getID() << " to " << to_state->getID() << " with trigger " << trigger->getID()<< "\n";
    }
int sample() {

    FSM::Event * EventA = new FSM::Event();
    FSM::Event * EventB = new FSM::Event();
    
    FSM::State * stateA = new FSM::State();
    stateA->setEnterFunction(enterFn);
    stateA->setExitFunction(exitFn);
    
    // from state     , to state      , trigger, guard           , action
    FSM::Trans trans1 = { FSM::Fsm::Fsm_Initial, stateA             , EventA    , nullptr, action1 };
    FSM::Trans trans2 = { stateA               , FSM::Fsm::Fsm_Final, EventB    , guard2 , action2 };
    
    
    std::vector<FSM::Trans> transitions =
    {
        trans1, trans2
    };
    
    FSM::Fsm fsm;
    fsm.add_debug_fn(dbg_fsm);
    fsm.add_transitions(transitions);
    fsm.init();
    assert(fsm.is_initial());
    fsm.execute(EventA);
    assert(stateA == fsm.state());
    fsm.execute(EventB);
    assert(FSM::Fsm::Fsm_Final->getID() == fsm.state()->getID());
    assert(fsm.is_final());
    fsm.reset();
    assert(fsm.is_initial());
    return 0;
}
