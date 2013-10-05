#include <iostream>
#include <future>
#include <exception>
#include <stdexcept>
int main(int, char**) {
    try {
    std::promise< int > p;
    std::future< int > f = p.get_future();
    p.set_value(2);
    std::cout << f.get() << std::endl;
    //std::future< int > f2 = p.get_future();
    p.set_value(3);
    std::cout << f.get() << std::endl; 
    return 0;
    } catch(const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }
}