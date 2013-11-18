//Author; Ugo Varetto
//simple implementation of std::function-like class using lambdas and unions
//TODO:
// - void,
// - forward
// - const for functor objects
// - copy, assignment and move
#include <cassert>
#include <iostream>
#include <vector>
//==============================================================================
//generic
template < typename > struct Fun {};

template < typename R, typename...ArgTypes >
struct Fun< R (ArgTypes...) >  {
    struct Object {};
    Fun(R (*f)(ArgTypes...));
    template < typename T > Fun(R (T::*f)(ArgTypes...) );
    template < typename T > Fun(R (T::*f)(ArgTypes...) const);
    template < typename T > Fun(const T& f);
    R operator()(ArgTypes...args);
    template < typename T > R operator()(T& obj, ArgTypes...args);
    template < typename T > R operator()(const T& obj, ArgTypes...args);
    union {
        R(*free_)(ArgTypes...);
        R(*method_)(void* ,  R (Object::*)(ArgTypes...), ArgTypes...);
        R(*cmethod_)(const void* , R (Object::*)(ArgTypes...) const,
                     ArgTypes...);
    } f_;
    union {
        R(Object::*method_)(ArgTypes...);
        R(Object::*cmethod_)(ArgTypes...) const;
        R(*call_)(Fun*, ArgTypes...);
        R(*obj_)(Fun*, ArgTypes...);
    } m_;
    std::vector< char > buf_;
    ~Fun() { //todo add lambda to invoke destructor on functor stored into buf_
    }
};

//non-void return type
template < typename R, typename...ArgTypes >
Fun< R (ArgTypes...) >::Fun(R (*f)(ArgTypes...)) { 
    f_.free_ = f; 
    using T = R (*)(Fun*, ArgTypes...);
    m_.call_ = (T) ([](Fun* f, ArgTypes...args) {
         return f->f_.free_(args...);
    });
}

template < typename R, typename...ArgTypes >
template < typename T >
Fun< R(ArgTypes...) >::Fun(R (T::*f) (ArgTypes...) ) {
    using F = R (*)(void*, R (Object::* )(ArgTypes...), ArgTypes...);
    using OM = R (Object::*)(ArgTypes...);
    f_.method_ = (F) ([](void* obj, OM m, ArgTypes...args) {
             using M = R (T::*)(ArgTypes...);
             M method = reinterpret_cast< M >(m);
             return (static_cast< T* >(obj)->*method)(args...);
             return R();
         });
    using OM = R (Object::*)(ArgTypes...);
    m_.method_ = reinterpret_cast< OM >(f); 
}

template < typename R, typename...ArgTypes >
template < typename T >
Fun< R(ArgTypes...) >::Fun(R (T::*f) (ArgTypes...) const) {
    using F = R (*)(const void*, R (Object::* )(ArgTypes...) const,
              ArgTypes...);
    using OM = R (Object::*)(ArgTypes...) const;
    f_.cmethod_ = (F) ([](const void* obj, OM m, ArgTypes...args) {
            using M = R (T::*)(ArgTypes...) const;
            M method = reinterpret_cast< M >(m);
            return (static_cast< const T* >(obj)->*method)(args...);
        });
    using OM = R (Object::*)(ArgTypes...) const;
    m_.cmethod_ = reinterpret_cast< OM >(f); 
}

template < typename R, typename...ArgTypes >
template < typename F >
Fun< R (ArgTypes...) >::Fun(const F& f) {
    buf_.resize(sizeof(f));
    new (&buf_[0]) F(f);
    using FF = R (*)(Fun*, ArgTypes...);
    m_.obj_ = (FF) ([](Fun* f, ArgTypes...args) {
        return reinterpret_cast< F* >(&f->buf_[0])->operator()(args...);
    });
}

template < typename R, typename...ArgTypes >
R Fun< R (ArgTypes...) >::operator()(ArgTypes...args) {
   return m_.call_(this, args...);
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

struct Class {
    int Twice(int i) { return 2 * i; }
    int VoidParam() { return 2; }
    int VoidParamConst() const { return 2; }
    void Void(int& i) {i = 1234;}
    Class() {}
};

struct Functor {
    int operator()(int i) { return 3 * i; }
};

void test1 () {
    Class c;
    const Class cc;
    Fun< int (int) > fun1(&Twice);  
    Fun< int (int) > fun2(&Class::Twice);
    Fun< int (   ) > fun3(&VoidParam);
    Fun< int (   ) > fun4(&Class::VoidParam);
    Fun< int (   ) > fun5(&Class::VoidParamConst);
    Fun< void(int&) > fun6(&Class::Void);
    Fun< int(int) > fun7((Functor()));
   
    assert(fun1(2)    ==  4);
    assert(fun2(c, 3) ==  6);
    assert(fun5(cc)   ==  2);
    assert(fun7(6)    == 18);
    // int i = 0;
    // fun6(c, i);
    // assert(i == 1234);
    std::cout << "PASSED" << std::endl;
}

//------------------------------------------------------------------------------

int main(int, char**) {
    test1();
    return 0;
}               