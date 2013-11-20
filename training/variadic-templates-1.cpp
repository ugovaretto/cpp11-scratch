#if __cplusplus < 201103L  // C++ 11
#error "C++ 11 support required"
#endif 

#include <iostream>
#include <sstream>
#include <tuple>
#include <cctype>
#include <string>
#include <stdexcept>
#include <functional>


//------------------------------------------------------------------------------
//cover 1 parameter case: the ternination condition
template< typename A >
void PrintHelper(const A& x) {
    std::cout << x;
}

//>1 parameter: takes first element from list: (xs) -> (x:xs)
template < typename A, typename... Args >
void PrintHelper(const A& x, const Args&... xs) {
    std::cout << x << ' ';
    PrintHelper(xs...);
}

//>1 parameters
template < typename... Args >
void Print(const Args&... xs) {
    PrintHelper(xs...);
}


//------------------------------------------------------------------------------
//apply a functor at run-time to a specific argument in a variadic
//argument list
namespace detail { //hide private implementation
template < typename F, typename A >
void ApplyHelper(int id, const F& f, int level, const A& arg) {
    if(level == id) f(arg);
    else throw std::range_error("index out of bound");
}

template < typename F, typename T, typename... Args >
void ApplyHelper(int id, const F& f, int level,
                  const T& x, const Args&... xs) {
    if(level == id) f(x);
    else ApplyHelper(id, f, level + 1, xs...);
}
}

template < typename F, typename... Args >
void Apply(int id, const F& f, const Args&... args) {
    detail::ApplyHelper(id, f, 0, args...);
}

//------------------------------------------------------------------------------
//apply a functor at run-time to a specific argument in a tuple

namespace detail { //hide private implementation
template < typename TupleT, size_t N > 
struct ApplyToTuple {
    template < typename F >
    static void apply(int id, const F& f, int level, const TupleT& t) {
        if(level == id) {
            f(std::get< N - 1 >(t));
            return;
        }
        ApplyToTuple< TupleT, N - 1 >::apply(id, f, level - 1, t);
    }
};

template < typename TupleT > 
struct ApplyToTuple< TupleT, 1 > {
    template < typename F >
    static void apply(int id, const F& f, int level, const TupleT& t) {
        if(level == id) {
            f(std::get< 0 >(t));
            return;
        }
        throw std::range_error("index out of bound");
    }
};
}

template < typename F, typename... Args >
void Apply(int id, const F& f, const std::tuple< Args... >& args) {
    detail::ApplyToTuple< decltype(args), sizeof...(Args) >::apply(id, f, 
                                                 sizeof...(Args) - 1, args);
}

//------------------------------------------------------------------------------
class PrintToStream {
public:
    PrintToStream(std::ostream& os) : os_(std::ref(os)) {}
    template < typename T >
    void operator()(const T& v) const {
        os_.get() << v;
    }
private:
    mutable std::reference_wrapper<std::ostream> os_;    
};

template < typename... Args >
std::string Format(const char* f, const Args&... params) {
    std::ostringstream oss;
    while(*f) {
        if(*f == '$') {
            ++f;
            if(std::isdigit(*f)) {
                //positional values go from $1 to $9
                const int pos = *f - 48 - 1; //0 in standard ASCII  
                Apply(pos, PrintToStream(oss), params...);
            } else oss << "$" << *f;
            ++f; 
        } else oss << *f++;
    }
    
    return oss.str();
}

//------------------------------------------------------------------------------
int main(int, char**) {
    Print("one", 2, 3.0);
    std::cout << std::endl;
    const std::string s = 
         Format("this is the $2nd paramenter and this is the $1.\n",
                "first", 2);
    std::cout << s;
    auto t = std::make_tuple(1., 2., 3, "4");
    Apply(2, PrintToStream(std::cout), t);
    std::cout << std::endl;
    return 0;
}
