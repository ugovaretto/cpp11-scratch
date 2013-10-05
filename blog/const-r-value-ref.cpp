#include <functional>
#include <iostream>
//const T&& - a use case: when keeping references we do not want to accept
//references to temporary objects
//#ifdef CRVALREF
template < typename T >
void bar(const T&&) {
    std::cout << "const && - !!!\n";
};
//#endif
//#ifdef RVALREF
template < typename T >
void bar(T&&) {
    std::cout << "&& - !!!\n";
};
//#endif
template < typename T >
void bar(T&) {
    std::cout << "&\n";
};
template < typename T >
void bar(const T&) {
    std::cout << "const &\n";
}

template < typename T >
void f(const T&& i) {
  bar(i);
}

template < typename T >
const T foo(const T& v) {return v; }

class C {
public:
 C() : i(1) {}
private:
int i;
};

int main(int, char**) {
    int i = int();
    ++i = 2;
    bar(i++); // calls &; removing the int& overload results in a compilation
            // error: use std::move to call the && overload
    bar(foo(C())); // calls && (would call const & if && overload not present)
    bar(([]{return 1;})()); // calls && (would call const & if &&
                            //   overload not present)
    f(2);
    bar(2);
    const int&& r = 2;
    bar(r);
    return 0;
}
