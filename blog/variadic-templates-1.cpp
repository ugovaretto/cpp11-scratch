#if __cplusplus < 201103L  // C++ 11
#error "C++ 11 support required"
#endif 

#include <iostream>

//------------------------------------------------------------------------------
//cover 1 parameter case
template< typename A >
void Print(const A& arg) {
    std::cout << arg;
}

//>1 parameters
template < typename A, typename... Args >
void Print(const A& x, const Args&... xs) {
    std::cout << x << ' ';
    Print(xs...);
    std::cout << std::endl;
}

//------------------------------------------------------------------------------
int main(int, char**) {
    Print("one", 2, 3);
    return 0;
}