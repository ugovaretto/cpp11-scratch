#include <iostream>
#include <tuple>

using std::tuple;
using std::cout;
using std::endl;


int f(int i1, int i2) { return i1 * i2; }

template<int ...> struct integer_sequence {};
//first int parameter is counter, second onwards are used to create the
//specialized type
template<int N, int ...Is> 
struct make_integer_sequene : make_integer_sequene< N - 1, N - 1, Is... > {};
template< int ...Is > 
struct make_integer_sequene< 0, Is... > { 
    typedef integer_sequence< Is... > type; 
};

template < int...Is, typename F, typename... Args > 
auto call_impl(integer_sequence< Is... >, F f, tuple< Args... > params )
    -> typename std::result_of< F (Args...) >::type {    
    return f(std::get< Is >(params)...);
}

template < typename F, typename...Args > 
auto call(F f, tuple< Args... > params )
    -> typename std::result_of< F (Args...) >::type {    
    return call_impl(typename make_integer_sequene<sizeof...(Args)>::type(), 
                     f, params);
}

int main(int, char**) {
    cout << call(f, std::make_tuple(1, 10)) << endl;
    return 0;
}