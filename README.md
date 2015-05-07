Finite State Machine for C++
============================

A simple, generic, header-only state machine implementation for C++.

Documentation
-------------

Please see the documentation in `fsm.h` for more detailed documentation about
the implemented features and semantics.

Usage
-----

copy fsm.h and fsm.cpp in your project. Check that the path to include fsm.h in fsm.cpp is correct


Stability
---------

The implementation is already in use in different commercial applications. We
have not found any issues so far. Please report any problems you find.

Tests
-----

Tests can be run with

~~~
cd tests
g++ -std=c++11 -Wall -o tests fsm_test.cpp
./tests
~~~

Or open the xcode workspace and run

Contributions
-------------

Contributions are welcome. Please use the Github issue tracker.
