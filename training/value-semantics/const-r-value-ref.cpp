//Author: Ugo Varetto
//Experiments with r-value references
// -DRVALREF: adds the T&& (template) and C&& (non template) overloads
// -DCRVALREF: adds the const T&& (template) and const C&& (non template)
//  overloads
// Bottom line: in the case of templated code the T type in the T&& signature
// is substituted with 'const U'; in the case of non-templated 'const U' 
// matches 'const U&&' or 'const U&' but *not* U&& 

#include <iostream>

//------------------------------------------------------------------------------
template < typename T >
struct Const {
    enum {isconst = 0};
};

template < typename T >
struct Const < const T > {
    enum {isconst = 1};
};

//------------------------------------------------------------------------------
#ifdef CRVALREF
template < typename T >
void bar(const T&&) {
    std::cout << "const &&\n";
};
#endif
#ifdef RVALREF
template < typename T >
void bar(T&&) {
    std::cout << "&& - " << "const: " << std::boolalpha
              << bool(Const< T >::isconst) << std::endl;
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

//------------------------------------------------------------------------------
class C {
public:
  C(int i = 1) : i_(i) {}
private:
  int i_;
};

//------------------------------------------------------------------------------
void Bar(const C&) { std::cout << "const &\n"; }
#ifdef CRVALREF
void Bar(const C&&) { std::cout << "const &&\n"; }
#endif
#ifdef RVALREF
void Bar(C&&) { std::cout << "&&\n"; }
#endif

//------------------------------------------------------------------------------
const C cc() { 
    C c(2);
    return c; 
}

//------------------------------------------------------------------------------
template < typename T >
const T cfoo(T t) {
    return t;
}

//------------------------------------------------------------------------------
int main(int, char**) {
    
    //I TEMPLATED
    //-----------
    //1. verify that a bar<T>(T&&) is called with T = U when passed the 
    //   value returned from a function
    // compile without any -D*: const & is called
    // compile with -DRVALREF -DCRVALREF: && is called 
    bar(([] {
         C c;
         return c;
         })());
    //2. verify that in case a bar<const T>(const T&&) function is present such
    //   function is called instead of (T&&) in case a temporary const
    //   instance is passed to bar.
    //2.1 the deduced return value is non-const
    // compile without any -D*: const & is called
    // compile with -DCRVALREF -DRVALREF: && is called
    bar(([] {
         const C c; 
         return c;
         })());
    //2.2 the return value is const as per declaration and will result in
    //  - bar(T&&) being called with T = const C OR
    //  - bar(const T&&) being called with T = const C in case the const&&
    //    version is defined
    // compile without -D*: const & called
    // compile with -DCREFVAL -DREFVAL =: const && called
    // compile with -DCREFVAL: && called with const type
    bar(cfoo(C()));

    //II NON-TEMPLATED
    //----------------
    //1. verify that if the const && overload is not present the const &
    //   function is called; this differs from the templated case where
    //   T && with T = const U is called instead
    // compile with -DRVALREF: const & is called
    // compile with -DCRVALREF -DRVALREF: const && is called
    // && is always ignored 
    Bar(cc()); 
    //2. double-check behavior with template version:
    // compile with -DRVALREF: && is called
    // compile with -DCRVALREF -DRVALREF: const && is called 
    // compile with -DRVALREF: && called with const type
    bar(cc()); //
    return 0;
}
