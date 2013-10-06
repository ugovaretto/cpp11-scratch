#include <functional>
#include <utility>
#include <iostream>
//const T&& - a use case: when keeping references we do not want to accept
//references to temporary objects

template < typename T >
struct Const {
    enum {isconst = 0};
};

template < typename T >
struct Const < const T > {
    enum {isconst = 1};
};


#ifdef CRVALREF
template < typename T >
void bar(const T&&) {
    std::cout << "const && - !!!\n";
};
#endif
#ifdef RVALREF
template < typename T >
void bar(T&&) {
    std::cout << "const: " << std::boolalpha
              << bool(Const< T >::isconst) << " && - !!!\n";
};
#endif
template < typename T >
void bar(T&) {
    std::cout << "&\n";
};
template < typename T >
void bar(const T&) {
    std::cout << "const &\n";
}





class C {
public:
  C(int i = 1) : i_(i) {}
private:
  int i_;
};

void Bar(const C&) { std::cout << "const &\n"; }
//void Bar(const C&&) { std::cout << "const &&\n"; }
void Bar(C&&) { std::cout << "&&\n"; }

int f() { 
    return 3;
}

const C cc() { 
    C c(f());
    return c; 
}

template < typename T >
const T cfoo(T t) {
    return t;
}

//template < typename T > void bar(const T&&) = delete;

int main(int, char**) {
    //bar(cfoo(C()));
    // bar(([] {
    //     C c;
    //     return c;
    //     })());
    // auto ref = std::cref(([] {
    //     const C c; //using a POD type like int doesn't work in this case
    //                //because the compiler subsitute the entire function
    //                //call with the value itself which is then simply a temporary
    //                //object which causes the && overload to be selected
    //     return c;
    //     })());     
    // bar(([] {
    //     const C c; //using a POD type like int doesn't work in this case
    //                //because the compiler subsitute the entire function
    //                //call with the value itself which is then simply a temporary
    //                //object which causes the && overload to be selected
    //     return c;
    //     })());
    Bar(cc()); 
    bar(cc());
    return 0;
}
