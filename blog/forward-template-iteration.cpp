#include <iostream>

//------------------------------------------------------------------------------
template < int N > 
struct ReversePrinter {
    static void Print() {
        //no idea at this point in the code about the actual number of
        //iterations, would need to have a separate int template parameter
        //holding the number of iterations
        for(int t = 0; t != N - 1; ++t) std::cout << "  ";
        std::cout << "Iteration " << N << '\n';
        ReversePrinter< N - 1 >::Print();
    }    
};

template <>
struct ReversePrinter< 1 > {
    static void Print() {
        std::cout << "Iteration 1" << std::endl;
    }
};

//------------------------------------------------------------------------------
template < int N, int Count = 1 > // add a counter as the last parameter
struct Printer {                  // default initialized to one, client code
    static void Print() {         // is free to change this to select a
        for(int t = 0; t != Count - 1; ++t) std::cout << "  ";// different
        std::cout << "Iteration " << Count << '\n';           // offset 
        Printer< N, Count + 1 >::Print();
    }
};

template < int N >
struct Printer< N, N > {
    static void Print() {
        for(int t = 0; t != N - 1; ++t) std::cout << "  ";
        std::cout << "Iteration " << N << std::endl;
    }
};

//------------------------------------------------------------------------------
int main(int, char**) {
    std::cout << "Standard reverse iterator:\n";
    ReversePrinter< 3 >::Print();
    std::cout << '\n';
    std::cout << "Forward iterator:\n";
    Printer< 3 >::Print();
    return 0;
}