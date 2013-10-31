#include <functional>
#include <iostream>
#include <vector>

using const_array_ref_t = std::vector< std::reference_wrapper< const int > >;
using array_ref_t = std::vector< std::reference_wrapper< int > >;

int main(int, char**) {
    int i = 1;
    int j = 2;
    const_array_ref_t rv 
        = {std::cref(i), std::cref(j)};
    std::cout << rv[1] << std::endl;
    array_ref_t rv2 = {std::ref(i), std::ref(j)};    
    rv2[1].get() = 7;
    std::cout << j << std::endl; 
    return 0;
}