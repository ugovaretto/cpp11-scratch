//Author: Ugo Varetto
//Benchmark of various approaches to value semantics with and without virtual
//functions
//NOTE on destruction:
//for non-virtual derived class the run-time invokes the base class destructor; 
//it is therefore required that the base destructor invoke the derived 
//instance destructor or the wrapped-data destructor directly.

//SUMMARY:
//any approach other than std::function or func::function including a base_t 
//with virtual methods is fine
//static vtables are much faster than anything else when lto enabled
//and outperform even regular non-virtual method invocations
//if instead of static function pointers, non-static pointers are used the
//performance is equal to virtual methods under -O3 optimizations
//when using xxx::function it is not worth trying with closures: do use regular
//functions which accept as the first parameter the model_t<> pointer to
//operate on, using a closure on this or model_t.d results in >40% performance
//penalty

//to see the benefits of a static vtable you need link time optimization
//clang on Apple: do use -O3 + lto
//gcc or clang on linux: do install the gold linker (binutils-gold) then
//rebuild gcc or clang with lto enabled

//the printed time in nanoseconds is computed as
// global time /
// (num executions 
//  * num elements
//  * num function calls per elements)
// and is proportional to the time it takes to perform a single method
// invocation but it is *not* the actual method invocation time since e.g. 
// in the case of non-virtual methods or full lto optimization with static
// functions everything seems to be inlined

//checkout results at the bottom of the file

#include <cassert>
#include <functional>
#include <memory>
#include <chrono>
#include <ctime>
#include <string>
#include <vector>
#include <cstring>
#include <iostream>
#include "function.h"   //Malte Skarupke's std::function
#include "simple-fun.h" //my own quick implementation of std::function always
                        //faster than std and Malte's when using static
                        //members, when not using static members Malte's version
                        //is always the fastest
                        

using namespace std;

#ifdef NOVIRTUAL
#define VIR
#else
#define VIR virtual
#endif


//------------------------------------------------------------------------------

template < int I, typename...Args >
struct Sizeof {
    enum { size = SizeofHelper< I, 0, Args... > };
};

template < int I, typename...Args >
struct SizeofHelper< I, I, Args...> {
    enum { size = 0 };    
};

template < int I, int Cnt, typename T, typename... Args >
struct SizeofHelper {
    enum { size = sizeof(T) + Size<I, Cnt +1, Args...>::size };
};

template < int I, typename...Args >
struct GetType {
    using type = typename GetTypeHelper< I, 0, Args...>::type;
};

template < int I, int Cnt, typename T, typename... Args >
struct GetTypeHelper {
    using type = typename GetTypeHelper< I, Cnt + 1, Args... >
};

template < int I, typename T, typename... Args >
struct GetTypeHelper< I, I, Args...> {
    using type = T;   
};


// template < typename...Args >
// BuildVTable(wrapper* w) {
//     Bind< 2, Args... >(w, w.fooimpl);
// }

// template < int I, typename T, typename...Args >
// void Bind(T& w, wrapper) { //also implement for const
//     w.m = [](void* p) {
//         static_cast< GetType< I, Args... > >(p + Sizeof< I, Args...>::size)->*method)
//     }
// }

struct B1 {
    void Foo() const { cout << "C1::Foo" << endl; }
};

struct B2 {
    void Foo() const { cout << "C2::Foo" << endl; }
};

struct Object {
    template < typename F&& f, typename...Args >
    Object(const Args&... args);
   ]
    vector< char > buf_;
};

template < typename F, typename...Args >
Object::Object(F&& f, const Args&...args) 
    : buf_(vector(Sizeof< sizeof...(Args), Args...>::size)) {
    //f(this, args...); //builds VTable
    
}


//------------------------------------------------------------------------------
int main(int argc, char** argv) {
  
    return 0;
}
