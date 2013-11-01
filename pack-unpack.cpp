//Author: Ugo Varetto
//Experiments with converting tuples to variadic type lists and deferred
//function invocation

#include <iostream>
#include <tuple>
#include <functional>

#include <typeinfo>

using std::tuple;
using std::cout;
using std::endl;
using std::make_tuple;

//------------------------------------------------------------------------------
template<int ...> struct integer_sequence {};
//first int parameter is a counter, second onwards are used to create the
//specialized type integer_sequence<0, 1, 2...> 
//at each inheritance step the counter is decremented and the new counter
//value is added to the integer sequence
template<int N, int ...Is> 
struct make_integer_sequence : make_integer_sequence< N - 1, N - 1, Is... > {};
template< int ...Is > 
struct make_integer_sequence< 0, Is... > { 
    typedef integer_sequence< Is... > type; 
};



template < typename F, typename...Args > 
auto call(F f, tuple< Args... > params )
    -> typename std::result_of< F (Args...) >::type {    
    return call_impl(typename make_integer_sequene<sizeof...(Args)>::type(), 
                     f, params);
}

template < int...Is, typename F, typename... Args > 
auto call_impl(integer_sequence< Is... >, F f, tuple< Args... > params )
    -> typename std::result_of< F (Args...) >::type {    
    return f(std::get< Is >(params)...);
}

//------------------------------------------------------------------------------
template < typename F, typename...Args >
std::function< typename std::result_of< F (Args...) >::type () >
make_deferred_call(F f, Args...args) {
    return [f, args...] {
        return f(std::move(args)...);
    };
}

//------------------------------------------------------------------------------
int f(int i1, int i2) { return i1 * i2; }

void f2(int i1, int i2) { cout << i1 + i2 << endl; }

int main(int, char**) {
    cout << "direct call 1 x 10\n";
    cout << call(f, make_tuple(1, 10)) << endl;
    auto deferred_f = make_deferred_call(f,2,3);
    cout << "deferred call 2 x 3\n";
    cout << deferred_f() << endl;
    auto deferred_void_f = make_deferred_call(f2, 2, 3);
    cout << "deferred call 2 + 3 'void' return type\n";
    deferred_void_f();
    return 0;
}