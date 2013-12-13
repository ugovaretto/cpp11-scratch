#include <iostream>
#include <utility> //move and forward
#include <type_traits>
//------------------------------------------------------------------------------
//overloaded functions called by Forward function
void Overfoo(int& arg) { std::cout << "L-value\n"; }
void Overfoo(int const &arg) { std::cout << "const L-value\n"; }
void Overfoo(int&& arg) { std::cout << "R-value\n"; }
//note: returning a const r-value reference is useful to forbid code to use
//temporary values returned by functions:
//foo(const int& ): accepts temporary instances of T returned by functions
//as xvalues
//foo(const int&&): returns an error whe passing a temporary object returned
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
    //note: arg is a local NAMED parameter so it is NEVER of type &&, in order
    //to properly forward it as an r-value reference you need to use the
    //forward function
    std::cout << "  std::forward -> ";
    Overfoo(std::forward< T >(arg)); 
    //note: move transforms anything to an R-value
    std::cout << "  std::move    -> ";
    Overfoo(std::move(arg));
    std::cout << "               -> ";
    Overfoo(arg);
}

template < typename T >
T&& MyForward(typename std::remove_reference<T>::type& v) {
    return static_cast<T&&>(v);
}

template < typename T >
T&& MyForward(typename std::remove_reference<T>::type&& v) {
    return static_cast<T&&>(v);
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
    std::cout << "++i ->\n";
    Forward(++i);
    std::cout << "i++ ->\n";
    Forward(i++);
    
    //the following line fails with a static assert
    //int& ri = std::forward<int&>(2);
    
    //the following line *does not fail* because there is no static assert
    //in R's implementation
    //int& ri = MyForward<int&>(2);
    
    return 0;
}
