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
 */
//
//  Created by Halere Eric on 06/05/2015.
//  Copyright (c) 2015 Halere Eric. All rights reserved.
//

#include "../include/fsm.h"


// static assignement

unsigned int FSM::Event::__current_id = 0;
unsigned int FSM::State::__current_id = 0;

FSM::State * FSM::Fsm::Fsm_Initial = new FSM::State;
FSM::State * FSM::Fsm::Fsm_Final = new FSM::State;

// Event class methods implementation

FSM::Event::Event() : m_id(__current_id) {
    __current_id++;
    assert(std::numeric_limits<unsigned int>::max() != __current_id);
};

unsigned int FSM::Event::getID() {
    return m_id;
};

// State Class implementation

FSM::State::State() : m_id(__current_id), m_enterFn(nullptr), m_exitFn(nullptr) {
    __current_id++;
    assert((std::numeric_limits<unsigned int>::max() -2 ) != __current_id);
};

unsigned int FSM::State::getID() {
    return m_id;
};

void FSM::State::setEnterFunction(stateFn iFn) {
    m_enterFn = iFn;
};

void FSM::State::setExitFunction(stateFn iFn) {
    m_exitFn = iFn;
};

void FSM::State::invokeEnterFunction() {
    if (m_enterFn) m_enterFn();
};

void FSM::State::invokeExitFunction() {
    if (m_exitFn) m_exitFn();
};

