#include <functional>
#include <iostream>


std::function< int() > make_countdown(int c) {
    int m = 0;
    return [=]() mutable {
        if(m < c) return m++;
        return c;
    };
}

int main(int, char**) {
    auto countdown = make_countdown(10);
    std::cout << countdown() << ' '
              << countdown() << ' '
              << countdown()
              << std::endl;
    return 0;
}