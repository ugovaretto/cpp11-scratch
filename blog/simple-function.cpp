#include <cassert>
#include <iostream>
//==============================================================================
//generic
template < typename > struct Fun {};

template < typename R, typename...ArgTypes >
struct Fun< R (ArgTypes...) > {
    Fun(R (*f)(ArgTypes...));
    template < typename T > Fun(R (T::*f)(ArgTypes...) );
    template < typename T > Fun(R (T::*f)(ArgTypes...) const);

    R (*f_)(ArgTypes...);  
};

//non-void return type
template < typename R, typename...ArgTypes >
Fun< R (ArgTypes...) >::Fun(R (*f)(ArgTypes...)) { 
    std::cout << "R (*f)(ArgTypes...)" << std::endl;
}

template < typename R, typename...ArgTypes >
template < typename T >
Fun< R(ArgTypes...) >::Fun(R (T::*f) (ArgTypes...) ) {
    std::cout << "R (T::*f)(ArgTypes...)" << std::endl;
}

template < typename R, typename...ArgTypes >
template < typename T >
Fun< R(ArgTypes...) >::Fun(R (T::*f) (ArgTypes...) const) {
    std::cout << "R (T::*f)(ArgTypes...) const" << std::endl;
}

//void return type
template < typename...ArgTypes >
struct Fun< void (ArgTypes...) > {
    Fun(void (*f)(ArgTypes...));
    template < typename T > Fun(void (T::*f)(ArgTypes...) );
    template < typename T > Fun(void (T::*f)(ArgTypes...) const);

    void (*f_)(ArgTypes...);  
};

template < typename...ArgTypes >
Fun< void (ArgTypes...) >::Fun(void (*f)(ArgTypes...)) { 
    std::cout << "void (*f)(ArgTypes...)" << std::endl;
}

template < typename...ArgTypes >
template < typename T >
Fun< void(ArgTypes...) >::Fun(void (T::*f) (ArgTypes...) ) {
    std::cout << "void (T::*f)(ArgTypes...)" << std::endl;
}

template < typename...ArgTypes >
template < typename T >
Fun< void (ArgTypes...) >::Fun(void (T::*f) (ArgTypes...) const) {
    std::cout << "void (T::*f)(ArgTypes...) const" << std::endl;
}


//------------------------------------------------------------------------------
int Twice(int i) { return 2 * i; }

int VoidParam() { return 2; }

struct Class {
    int Twice(int i) { return 2 * i; }
    int VoidParam() { return 2; }
    int VoidParamConst() const { return 2; }
    void Void() {}
};

void test1 () {
    Fun< int (int) > fun1(&Twice);  
    Fun< int (int) > fun2(&Class::Twice);
    Fun< int (   ) > fun3(&VoidParam);
    Fun< int (   ) > fun4(&Class::VoidParam);
    Fun< int (   ) > fun5(&Class::VoidParamConst);
    Fun< void(   ) > fun6(&Class::Void);
}

//------------------------------------------------------------------------------

int main(int, char**) {
    test1();
    return 0;
}               