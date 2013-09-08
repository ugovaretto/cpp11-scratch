#include <iostream>

//------------------------------------------------------------------------------
template < int N > 
struct ReversePrinter {
    static void Print() {
        //no idea at this point in the code about the actual number of
        //iterations, would need to have a separate int template parameter
        //holding the number of iterations
        for(int t = 0; t != N; ++t) std::cout << "  ";
        std::cout << "Iteration " << N << '\n';
        ReversePrinter< N - 1 >::Print();
    }    
};

template <>
struct ReversePrinter< 0 > {
    static void Print() {
        std::cout << "Iteration 0" << std::endl;
    }
};

//------------------------------------------------------------------------------
template < int N, int Count = 0 > // add a counter as the last parameter
struct Printer {                  // default initialized to zero, client code
    static void Print() {         // is free to change this to select a
        for(int t = 0; t != Count; ++t) std::cout << "  ";// different offset
        std::cout << "Iteration " << Count << '\n'; 
        Printer< N, Count + 1 >::Print();
    }
};

template < int N >
struct Printer< N, N > {
    static void Print() {
        for(int t = 0; t != N; ++t) std::cout << "  ";
        std::cout << "Iteration " << N << std::endl;
    }
};


//------------------------------------------------------------------------------
int main(int, char**) {
    std::cout << "Standard reverse iterator:\n";
    ReversePrinter< 3 >::Print();
    std::cout << '\n';
    std::cout << "In order iterator:\n";
    Printer< 3 >::Print();
    return 0;
}