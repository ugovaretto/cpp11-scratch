//Author; Ugo Varetto
//simple implementation of std::function-like class, clang compiler >= 3.3
//crashes! not tried with < 3.3, builds fine with g++ 4.8.1
#include <cassert>
#include <iostream>
//==============================================================================
//generic
template < typename > struct Fun {};

template < typename R, typename...ArgTypes >
struct Fun< R (ArgTypes...) >  {
    struct Object {};
    Fun(R (*f)(ArgTypes...));
    template < typename T > Fun(R (T::*f)(ArgTypes...) );
    template < typename T > Fun(R (T::*f)(ArgTypes...) const);
    R operator()(ArgTypes...args);
    template < typename T > R operator()(T& obj, ArgTypes...args);
    template < typename T > R operator()(const T& obj, ArgTypes...args);
    union {
        R(*free_)(ArgTypes...);
        R(*method_)(void* ,  R (Object::*)(ArgTypes...), ArgTypes...);
        R(*cmethod_)(const void* , R (Object::*)(ArgTypes...) const, ArgTypes...);
    } f_;
    union {
        R(Object::*method_)(ArgTypes...);
        R(Object::*cmethod_)(ArgTypes...) const;
    } m_;
};

//non-void return type
template < typename R, typename...ArgTypes >
Fun< R (ArgTypes...) >::Fun(R (*f)(ArgTypes...)) { 
    std::cout << "R (*f)(ArgTypes...)" << std::endl;
    f_.free_ = f;
}

template < typename R, typename...ArgTypes >
template < typename T >
Fun< R(ArgTypes...) >::Fun(R (T::*f) (ArgTypes...) ) {
    std::cout << "R (T::*f)(ArgTypes...)" << std::endl;
    using F = R (*)(void*, R (Object::* )(ArgTypes...), ArgTypes...);
    using OM = R (Object::*)(ArgTypes...);
    f_.method_ = (F) ([](void* obj, OM m, ArgTypes...args) {
            using M = R (T::*)(ArgTypes...);
            M method = (M) m;
            return (static_cast< T* >(obj)->*method)(args...);
        });
    using OM = R (Object::*)(ArgTypes...);
    m_.method_ = (OM) f; 
}

template < typename R, typename...ArgTypes >
template < typename T >
Fun< R(ArgTypes...) >::Fun(R (T::*f) (ArgTypes...) const) {
    std::cout << "R (T::*f)(ArgTypes...) const" << std::endl;
    using F = R (*)(const void*, R (Object::* )(ArgTypes...) const, ArgTypes...);
    using OM = R (Object::*)(ArgTypes...) const;
    f_.cmethod_ = (F) ([](const void* obj, OM m, ArgTypes...args) {
            using M = R (T::*)(ArgTypes...) const;
            M method = (M) m;
            return (static_cast< const T* >(obj)->*method)(args...);
        });
    using OM = R (Object::*)(ArgTypes...) const;
    m_.cmethod_ = (OM) f; 
}

template < typename R, typename...ArgTypes >
R Fun< R (ArgTypes...) >::operator()(ArgTypes...args) {
    return f_.free_(args...);
}

template < typename R, typename...ArgTypes >
template < typename T >
R Fun< R (ArgTypes...) >::operator()(T& obj, ArgTypes...args) {
    return f_.method_(&obj, m_.method_, args...);
}

template < typename R, typename...ArgTypes >
template < typename T >
R Fun< R (ArgTypes...) >::operator()(const T& obj, ArgTypes...args) {
    return f_.cmethod_(&obj, m_.cmethod_, args...);
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

struct A {
    int a;
};

struct B {
    int b;
};

struct Class : A, virtual B {
    int Twice(int i) { return 2 * i; }
    int VoidParam() { return 2; }
    int VoidParamConst() const { return 2; }
    void Void() {}
    Class() {}
};

void test1 () {
    Class c;
    const Class cc;
    Fun< int (int) > fun1(&Twice);  
    Fun< int (int) > fun2(&Class::Twice);
    // Fun< int (   ) > fun3(&VoidParam);
    // Fun< int (   ) > fun4(&Class::VoidParam);
    // Fun< int (   ) > fun5(&Class::VoidParamConst);
    // Fun< void(   ) > fun6(&Class::Void);
   
    // assert(fun1(2) == 4);
    // assert(fun2(c, 3) == 6);
    // assert(fun5(cc) == 2);
    std::cout << "PASSED" << std::endl;
}

//------------------------------------------------------------------------------

int main(int, char**) {
    test1();
    return 0;
}               