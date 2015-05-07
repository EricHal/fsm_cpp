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
 * \file fsm_test.cpp
 * Test definitions for the state machine implementation.
 */

/****************
 * The following source is orginally Copyright (c) 2014 wisol technologie GmbH
 * Thanks, Great implementation.
 *
 *
 * Eric Halère modify them to support new method signature and class Event and State
 *
 * Added a test cases for enter and exit functions
 * Added a test cases to just run the sample
 */

#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include <array>
#include <vector>
#include "../include/fsm.h"
#include "sample.h"

TEST_CASE("Test Initialization")
{
	FSM::Fsm fsm;
    FSM::Event * a = new FSM::Event();
	REQUIRE(fsm.execute(a) == FSM::Fsm_NotInitialized);
    
    delete a;
}

TEST_CASE("Test initial and final pseudo states")
{
	FSM::Fsm fsm;
    FSM::Event * a = new FSM::Event();
	std::array<FSM::Trans,1> transitions = {{
			{FSM::Fsm::Fsm_Initial, FSM::Fsm::Fsm_Final, a, nullptr, nullptr},
	}};
	fsm.add_transitions(transitions.begin(), transitions.end());
	fsm.init();

	SECTION("Test initial pseudo state") {
		REQUIRE(fsm.state()->getID() == FSM::Fsm::Fsm_Initial->getID());
		REQUIRE(fsm.is_initial() == true);
		REQUIRE(fsm.is_final() == false);
	}

	SECTION("Test initial pseudo state") {
		fsm.execute(a);
		REQUIRE(fsm.state()->getID() == FSM::Fsm::Fsm_Final->getID());
		REQUIRE(fsm.is_initial() == false);
		REQUIRE(fsm.is_final() == true);
	}
    
    delete a;
}

TEST_CASE("Test missing trigger")
{
	FSM::Fsm fsm;
    FSM::Event * a = new FSM::Event();
    FSM::Event * b = new FSM::Event();
	std::array<FSM::Trans,1> transitions = {{
			{FSM::Fsm::Fsm_Initial, FSM::Fsm::Fsm_Final, b, nullptr, nullptr},
	}};
	fsm.add_transitions(transitions.begin(), transitions.end());
	fsm.init();
	REQUIRE(fsm.execute(a) == FSM::Fsm_NoMatchingTrigger);
    
    delete a;
    delete b;
}

TEST_CASE("Test guards")
{
	SECTION("Test false guard") {
		FSM::Fsm fsm;
        FSM::Event * a = new FSM::Event();
		std::array<FSM::Trans,1> transitions = {{
				{FSM::Fsm::Fsm_Initial, FSM::Fsm::Fsm_Final, a, []{return false;}, nullptr},
		}};
		fsm.add_transitions(transitions.begin(), transitions.end());
		fsm.init();
		REQUIRE(fsm.execute(a) == FSM::Fsm_Success);
		// ensure that the transition to final is not taken (because of the guard).
		REQUIRE(fsm.state()->getID() == (int)FSM::Fsm::Fsm_Initial->getID());
        delete a;
    }

	SECTION("Test true guard") {
		FSM::Fsm fsm;
        FSM::Event * a = new FSM::Event();
		std::array<FSM::Trans,1> transitions = {{
				{FSM::Fsm::Fsm_Initial, FSM::Fsm::Fsm_Final, a, []{return true;}, nullptr},
		}};
		fsm.add_transitions(transitions.begin(), transitions.end());
		fsm.init();
		REQUIRE(fsm.execute(a) == FSM::Fsm_Success);
		// ensure that the transition to final is taken (because of the guard).
		REQUIRE(fsm.state()->getID() == FSM::Fsm::Fsm_Final->getID());
        
        delete a;
    }

	SECTION("Test same action with different guards") {
		int count = 0;
		FSM::Fsm fsm;
        FSM::Event * a = new FSM::Event();
		std::array<FSM::Trans,2> transitions = {{
            {FSM::Fsm::Fsm_Initial, FSM::Fsm::Fsm_Final, a, []{return false;}, [&count](FSM::Event * evt){count++;}},
				{FSM::Fsm::Fsm_Initial, FSM::Fsm::Fsm_Final, a, []{return true; }, [&count](FSM::Event * evt){count = 10;}},
		}};
		fsm.add_transitions(transitions.begin(), transitions.end());
		fsm.init();
		REQUIRE(fsm.execute(a) == FSM::Fsm_Success);
		// ensure that action2 was taken (because of the guard).
		REQUIRE(count == 10);
        
        delete a;
	}
}

TEST_CASE("Test Transitions")
{
	SECTION("Test multiple matching transitions") {
		int count = 0;
		FSM::Fsm fsm;
        FSM::Event * a = new FSM::Event();
        FSM::State * stateA = new FSM::State();
		std::array<FSM::Trans,3> transitions = {{
				{FSM::Fsm::Fsm_Initial, stateA        , a, nullptr, [&count](FSM::Event * evt){count++;}},
				{stateA          , stateA        , a, nullptr, [&count](FSM::Event * evt){count++;}},
				{stateA          , FSM::Fsm::Fsm_Final, a, nullptr, [&count](FSM::Event * evt){count++;}},
		}};
		fsm.add_transitions(transitions.begin(), transitions.end());
		fsm.init();
		REQUIRE(fsm.execute(a) == FSM::Fsm_Success);
		// Ensure that only one action has executed.
		REQUIRE(count == 1);
        
        delete a;
        delete stateA;
	}
}

TEST_CASE("Test state machine reset")
{
	int action_count = 0;
	FSM::Fsm fsm;
    FSM::Event * a = new FSM::Event();
    FSM::Event * b = new FSM::Event();
    FSM::State * stateA = new FSM::State();
	std::array<FSM::Trans,2> transitions = {{
			{FSM::Fsm::Fsm_Initial, stateA        , a, nullptr, nullptr},
			{stateA          , FSM::Fsm::Fsm_Final, b, nullptr, nullptr},
	}};
	fsm.add_transitions(transitions.begin(), transitions.end());
	fsm.init();
	REQUIRE(fsm.execute(a) == FSM::Fsm_Success);
	REQUIRE(fsm.state() == stateA);
	fsm.reset();
	REQUIRE(fsm.state()->getID() == FSM::Fsm::Fsm_Initial->getID());
	REQUIRE(fsm.execute(a) == FSM::Fsm_NotInitialized);
	fsm.init();
	REQUIRE(fsm.execute(a) == FSM::Fsm_Success);
	REQUIRE(fsm.execute(b) == FSM::Fsm_Success);
	REQUIRE(fsm.is_final() == true);
    
    delete a;
    delete b;
    delete stateA;
}

TEST_CASE("Test debug function")
{
	int action_count = 0;
	FSM::Fsm fsm;
    FSM::Event * a = new FSM::Event();
    FSM::Event * b = new FSM::Event();
    FSM::State * stateA = new FSM::State();
	std::array<FSM::Trans,2> transitions = {{
			{FSM::Fsm::Fsm_Initial, stateA        , a, nullptr, nullptr},
			{stateA          , FSM::Fsm::Fsm_Final, b, nullptr, nullptr},
	}};
	fsm.add_transitions(transitions.begin(), transitions.end());
	fsm.init();

	SECTION("Test enable debugging function.") {
		FSM::State * dbg_from = 0;
		FSM::State * dbg_to = 0;
		FSM::Event * dbg_tr = 0;
		fsm.add_debug_fn([&dbg_from, &dbg_to, &dbg_tr](FSM::State *  from, FSM::State *  to, FSM::Event * tr) { dbg_from = from; dbg_to = to; dbg_tr = tr; });
		fsm.execute(a);
		REQUIRE(dbg_from == FSM::Fsm::Fsm_Initial);
		REQUIRE(dbg_to == stateA);
		REQUIRE(dbg_tr == a);
	}

	SECTION("Test disable debugging function.") {
        FSM::State * dbg_from = 0;
        FSM::State * dbg_to = 0;
        FSM::Event * dbg_tr = 0;
		fsm.reset();
		fsm.add_debug_fn(nullptr);
		REQUIRE(dbg_from == 0);
		REQUIRE(dbg_to == 0);
		REQUIRE(dbg_tr == 0);
	}
    
    delete a;
    delete b;
    delete stateA;
}

TEST_CASE("Test single argument add_transitions function")
{
	FSM::Fsm fsm;
    FSM::Event * a = new FSM::Event();
    FSM::Event * b = new FSM::Event();
    FSM::State * stateA = new FSM::State();

	SECTION("Test vector") {
		std::vector<FSM::Trans> v = {
			{FSM::Fsm::Fsm_Initial, stateA        , a, nullptr, nullptr},
			{stateA          , FSM::Fsm::Fsm_Final, b, nullptr, nullptr},
		};
		fsm.add_transitions(v);
		fsm.init();
		fsm.execute(a);
		fsm.execute(b);
		REQUIRE(fsm.state() == FSM::Fsm::Fsm_Final);
	}

	SECTION("Test initializer list") {
		fsm.add_transitions({
			{FSM::Fsm::Fsm_Initial, stateA        , a, nullptr, nullptr},
			{stateA          , FSM::Fsm::Fsm_Final, b, nullptr, nullptr},
			});
		fsm.init();
		fsm.execute(a);
		fsm.execute(b);
		REQUIRE(fsm.state() == FSM::Fsm::Fsm_Final);
	}
    
    delete a;
    delete b;
    delete stateA;
}

TEST_CASE("Test enter and exit states functions")
{
    SECTION("Test enter state") {
        int count = 0;
        FSM::Fsm fsm;
        FSM::Event * a = new FSM::Event();
        FSM::State * stateA = new FSM::State();
        stateA->setEnterFunction([&count](){count++;});
        
        std::array<FSM::Trans,1> transitions = {{
            {FSM::Fsm::Fsm_Initial, stateA, a, nullptr, nullptr},
        }};
        fsm.add_transitions(transitions.begin(), transitions.end());
        fsm.init();
        REQUIRE(fsm.execute(a) == FSM::Fsm_Success);
        // ensure that the enter state function is called once.
        REQUIRE(count == 1);
        
        delete a;
        delete stateA;
    }
    
    SECTION("Test exit state") {
        int count = 0;
        FSM::Fsm fsm;
        FSM::Event * a = new FSM::Event();
        FSM::State * stateA = new FSM::State();
        stateA->setExitFunction([&count](){count++;});
        
        std::array<FSM::Trans,2> transitions = {{
            {FSM::Fsm::Fsm_Initial, stateA, a, nullptr, nullptr},
            {stateA, FSM::Fsm::Fsm_Final, a, nullptr, nullptr},
        }};
        fsm.add_transitions(transitions.begin(), transitions.end());
        fsm.init();
        REQUIRE(fsm.execute(a) == FSM::Fsm_Success);
        REQUIRE(fsm.execute(a) == FSM::Fsm_Success);
        // ensure that the enter state function is called once.
        REQUIRE(count == 1);

        delete a;
        delete stateA;
    }
}


TEST_CASE("SAMPLE")
{
    REQUIRE(sample() == 0);
}