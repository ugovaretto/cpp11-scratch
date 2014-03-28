//author: Ugo Varetto
//test driver for C++11 timer classes

#include <iostream>
#include "Timer.h"

using namespace std;

//------------------------------------------------------------------------------
void Busy(double s) {
    Sleep(s);
}

//------------------------------------------------------------------------------
int main(int, char**) {
    Timer timer;
    timer.Start();
    Busy(2.2);
    const Timer::Tick s = timer.s();
    const Timer::Tick us = timer.us();
    const Timer::Tick ns = timer.ns();
    using Seconds = ratio< 1, 1 >;
    const double ds = timer.Elapsed< double, Seconds >(); 
    cout << s << "s " << us << "us " << ns << "ns" << endl;
    cout << ds << " float seconds" << endl;
    int count = 3;
    TimerCallback tc;
    tc.Start(2, [](const int& c){cout << c << endl;},
            [&count](){return count--;},
            std::cref(count));
    tc.WaitForCompletion();
    return 0;
}



