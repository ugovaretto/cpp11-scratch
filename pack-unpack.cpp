#include <tuple>
#include <iostream>
using std::cout;
using std::endl;
using std::tuple;

int f(int i1, int i2) { return i1 * i2; }

template<int ...> struct seq {};
template<int N, int ...S> struct gens : gens<N-1, N-1, S...> {};
template<int ...S> struct gens<0, S...>{ typedef seq<S...> type; };

template < int...S, typename F, typename... Args > 
auto callfun(seq<S...>, F f, tuple< Args... > params )
    -> typename std::result_of< F (Args...) >::type {    
    return f(std::get<S>(params)...);
}

template < typename F, typename...Args > 
auto call(F f, tuple< Args... > params )
    -> typename std::result_of< F (Args...) >::type {    
    return callfun(typename gens<sizeof...(Args)>::type(), f, params);
}

int main(int, char**) {
    cout << call(f, std::make_tuple(1, 2)) << endl;
    return 0;
}