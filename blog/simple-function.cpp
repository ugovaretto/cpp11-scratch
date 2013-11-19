//Author; Ugo Varetto
//simple implementation of std::function-like class using lambdas and unions

#include <cassert>
#include <iostream>
#include <vector>
#include <stdexcept>
#include "simple-fun.h"

//------------------------------------------------------------------------------
int Twice(int i) { return 2 * i; }

int VoidParam() { return 2; }

struct Class {
    int Twice(int i) { return 2 * i; }
    int VoidParam() { return 2; }
    int VoidParamConst() const { return 2; }
    void Void(int& i) const {i = 1234;}
    Class() {}
};

struct Functor {
    int operator()(int i) const { return 3 * i; }
};

Fun< int(int) > RetFun() {
    return Functor();
}

void test1 () {
    struct V {
        float x = 1.0f;
    } v;
    Class c;
    const Class cc;
    Fun< int (int) >   fun0;
    Fun< int (int) >   fun1(&Twice);  
    Fun< int (int) >   fun2(&Class::Twice);
    Fun< int () >      fun3(&VoidParam);
    Fun< int () >      fun4(&Class::VoidParam);
    Fun< int () >      fun5(&Class::VoidParamConst);
    Fun< void(int&) >  fun6(&Class::Void);
    Fun< int (int) >   fun7((Functor()));
    auto               fun8(RetFun());
    Fun< float (int, int) > fun9([=](int i, int j) {return float(i/j);});
    Fun< float (int, int) > fun10(fun9);
    Fun< float () > fun11([v](){return v.x;});
    try { 
         fun0(9); //must throw
         assert(false);
    } catch(...) {}
    fun0 = fun1;
    assert(fun0(2)    == fun1(2));
    assert(fun1(2)    ==  4);
    assert(fun2(c, 3) ==  6);
    assert(fun5(cc)   ==  2);
    assert(fun7(6)    == 18);
    assert(fun8(7)    == 21);
    assert(fun9(10,4) == float(10/4));
    assert(fun10(20, 8) == fun9(20, 8));
    assert(fun11() == 1.0f);
    //void return type
    int i = 0;
    fun6(c, i);
    assert(i == 1234);
}

//------------------------------------------------------------------------------

int main(int, char**) {
    test1();
    return 0;
}               