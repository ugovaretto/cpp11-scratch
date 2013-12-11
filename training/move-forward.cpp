#include <iostream>
#include <utility> //move and forward

//------------------------------------------------------------------------------
//overloaded functions called by Forward function
void Overfoo(int& arg) { std::cout << "L-value\n"; }
void Overfoo(int const &arg) { std::cout << "const L-value\n"; }
void Overfoo(int&& arg) { std::cout << "R-value\n"; }
//note: returning a const r-value reference is useful to forbid code to use
//temporary values returned by functions:
//foo(const T& ): accepts temporary instances of T returned by functions
//as xvalues
//foo(const T&&): returns an error whe passing a temporary object returned
//by another function
void Overfoo(const int&& arg) { std::cout << "const R-value\n"; }
void Overfoo(volatile const int &arg ) { 
    std::cout << "volatile const L-value\n";
}
void Overfoo(volatile int &arg) { std::cout << "volatile L-value\n"; }
void Overfoo(volatile int && arg) { std::cout << "volatile R-value\n"; }
void Overfoo(volatile const int && arg) { 
    std::cout << "const volatile R-value\n";
}

// '&&' keeps information about the original type qualifier allowing
// to forward parameters to another function through perfect forwarding;
// if T = Type& -> T&& = Type&; if T = Type&& -> T&& = Type&&
// std::forward does not allow R value to L value reference conversion,
// preventing it through a static_assert in the forward function implementation 
template < typename T > 
void Forward(T&& arg) {
    std::cout << "  std::forward -> ";
    Overfoo(std::forward< T >(arg)); 
    std::cout << "  std::move    -> ";
    Overfoo(std::move(arg));
    std::cout << "               -> ";
    Overfoo(arg);
}



//------------------------------------------------------------------------------
int main() {
    std::cout << "R-value ->\n";
    Forward(0);
    std::cout << "L-value ->\n";
    int i = 1;
    Forward(i);
    std::cout << "const L-value ->\n";
    const int ci = 2;
    Forward(ci);
    std::cout << "volatile L-value ->\n";
    volatile int vi = 3;
    Forward(vi);
    std::cout << "volatile const L-value ->\n";
    volatile const int cvj = 4;
    Forward(cvj);
    return 0;
}
