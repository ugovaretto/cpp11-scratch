//Author: Ugo Varetto
//Shows how to apply a templated functor to all the elements
//in a variadic argument list
#include <iostream>
#include <typeinfo>
template < typename T > T Print(T v) { 
    std::cout << v << std::endl;
    return v;
}

template < typename... Args >
void foo(Args... args) {  
}

template < typename...Args >
void runfoo(Args...args) {
    foo(Print<Args>(args)...);
}

int main(int, char**) {
    runfoo(1,'2',"3",4.0);
    return 0;
}
