#include <iostream>
#include <functional>

#if __cplusplus != 201103
#error "Not a C++11 compliant compiler\n";
#endif

std::function< int () > MakeFunction() {
    return [] {
	    return __cplusplus;
	};
}

std::function< int () > MakeCounter1(int c) {
    return [=]() mutable { 
        std::cout << (void*) &MakeCounter1 << std::endl;
        return c++; };
}

std::function< int () > MakeCounter2() {
    int i = 0;
    return [=]() mutable { 
        std::cout << &MakeCounter2 << std::endl;
        return i++; };
}


int main(int, char**) {
    std::cout << MakeFunction()() << std::endl;	
    auto counter = MakeCounter1(2);
    std::cout << counter() << ' ' << counter() << std::endl;
    int v[] = {1, 2, 3, 4, 5};
    for(auto i: v) { std::cout << i;}
    std::cout << std::endl;
    return 0;
}

