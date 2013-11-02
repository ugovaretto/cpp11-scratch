#include <iostream>
#include <functional>

struct S {
    S(void (*f)(S&), S& s) : f_(f), s_(std::ref(s)) {}
    int data = 4;
    std::reference_wrapper< S > s_;
    void (*f_)(S&);
    void exec() { f_(s_); }
};

void f(S& s) {
    std::cout << s.data << std::endl;
}

int main(int, char**) {
    S s(f, s); 
    s.exec();
    return 0;
}