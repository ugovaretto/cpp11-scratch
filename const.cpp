#include <iostream>
template <typename T>
bool isconst(T*) { return false;}

template <typename T>
bool isconst(const T*) { return true;}

struct C {
    C() {
        int* i;
        std::cout << isconst(i) << std::endl;
    }
};



int main(int, char**) {
    const C c;
    C c2;

    return 0;
}