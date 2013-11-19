//Author; Ugo Varetto
//simple implementation of std::function-like class using lambdas and unions
//TODO: void return type

#include <cassert>
#include <iostream>
#include <vector>
//==============================================================================
//generic, need this to be able to declare Fun< int(int) > instead of
//Fun< int, int >
template < typename > struct Fun {};

//callable entities with a return type
template < typename R, typename...ArgTypes >
struct Fun< R (ArgTypes...) >  {
    struct Object {}; //generic object type used to cast member methods
    //constructors
    Fun(const Fun& fun) : f_(fun.f_), m_(fun.m_) {
        CopyObj(fun, *this);
    }
    Fun(R (*f)(ArgTypes...));
    template < typename T > Fun(R (T::*f)(ArgTypes...) );
    template < typename T > Fun(R (T::*f)(ArgTypes...) const);
    template < typename T > Fun(const T& f);
    Fun(Fun&&) = default;
    Fun& operator=(Fun f) {
        Destruct(this);
        f_ = f.f_;
        m_ = f.m_;
        buf_ = std::move(f.buf_);
        return *this;
    }
    //operator()
    R operator()(ArgTypes...args) const;
    template < typename T > R operator()(T& obj, ArgTypes...args) const;
    template < typename T > R operator()(const T& obj, ArgTypes...args) const;

    //delegate functions
    union {
        R(*free_)(ArgTypes...);
        R(*method_)(void* ,  R (Object::*)(ArgTypes...), ArgTypes...);
        R(*cmethod_)(const void* , R (Object::*)(ArgTypes...) const,
                     ArgTypes...);
    } f_;
    //delegate method invocation, either direct or through free functions
    //receiving an object instance pointer
    union {
        R(Object::*method_)(ArgTypes...);
        R(Object::*cmethod_)(ArgTypes...) const;
        R(*call_)(const Fun*, ArgTypes...);
        R(*obj_)(Fun*, ArgTypes...);
    } m_;
    void (*Destruct)(Fun*) = nullptr;
    void (*CopyObj)(const Fun&, Fun&) = nullptr;
    //storage for functor objects, in a real-world scenario we would need to
    //either pass a custom allocator using e.g. stack memory for small objects
    //or use a custom data type
    std::vector< char > buf_;
    ~Fun() {
        Destruct(this);
     }
};

//constructors
template < typename R, typename...ArgTypes >
Fun< R (ArgTypes...) >::Fun(R (*f)(ArgTypes...)) { 
    f_.free_ = f; 
    using T = R (*)(const Fun*, ArgTypes...);
    m_.call_ = (T) ([](const Fun* f, ArgTypes...args) {
         return f->f_.free_(std::forward< ArgTypes >(args)...);
    });
    Destruct = [](Fun*) {};
    CopyObj  = [](const Fun&, Fun&) {};
}

template < typename R, typename...ArgTypes >
template < typename T >
Fun< R(ArgTypes...) >::Fun(R (T::*f) (ArgTypes...) ) {
    using F = R (*)(void*, R (Object::* )(ArgTypes...), ArgTypes...);
    using OM = R (Object::*)(ArgTypes...);
    f_.method_ = (F) ([](void* obj, OM m, ArgTypes...args) {
             using M = R (T::*)(ArgTypes...);
             M method = reinterpret_cast< M >(m);
             return (static_cast< T* >(obj)->*method)
                        (std::forward< ArgTypes >(args)...);
             return R();
         });
    using OM = R (Object::*)(ArgTypes...);
    m_.method_ = reinterpret_cast< OM >(f); 
    Destruct = [](Fun*) {};
    CopyObj  = [](const Fun&, Fun&) {};
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
            return (static_cast< const T* >(obj)->*method)
                        (std::forward< ArgTypes >(args)...);
        });
    using OM = R (Object::*)(ArgTypes...) const;
    m_.cmethod_ = reinterpret_cast< OM >(f);
    Destruct = [](Fun*) {}; 
    CopyObj  = [](const Fun&, Fun&) {};
}

template < typename R, typename...ArgTypes >
template < typename F >
Fun< R (ArgTypes...) >::Fun(const F& f) {
    buf_.resize(sizeof(f));
    new (&buf_[0]) F(f);
    using FF = R (*)(Fun*, ArgTypes...);
    m_.obj_ = (FF) ([](Fun* f, ArgTypes...args) {
        return reinterpret_cast< F* >(&f->buf_[0])
                                ->operator()(std::forward< ArgTypes >(args)...);
    });
    Destruct = [](Fun* f) {
        F* fun = reinterpret_cast< F* >(&f->buf_[0]);
        fun->F::~F();
    };
    CopyObj  = [](const Fun& src, Fun& target) {
        target.buf_.resize(src.buf_.size());
        new (&target.buf_[0]) Fun(src);
    };
}

//operator()
template < typename R, typename...ArgTypes >
R Fun< R (ArgTypes...) >::operator()(ArgTypes...args) const {
   return m_.call_(this, args...);
}

template < typename R, typename...ArgTypes >
template < typename T >
R Fun< R (ArgTypes...) >::operator()(T& obj, ArgTypes...args) const {
    return f_.method_(&obj, m_.method_, args...);
}

template < typename R, typename...ArgTypes >
template < typename T >
R Fun< R (ArgTypes...) >::operator()(const T& obj, ArgTypes...args) const {
    return f_.cmethod_(&obj, m_.cmethod_, args...);
}


//TODO: callable entities without a return value (a.k.a void return type)
//instead of rewriting a complete specialzition it is actually possible
//to move the constructors and unions into a base class or use the
//Barton - Nackman trick:
//Fun< R(ArgTypes...) > : DelegateImpl< Fun< R, ArgTypes...> > {...}
//DelegateImpl adds the required operator()(...) implementations and
//has two specizations for void and non-void return type
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
    ~Class() {std::cout << "Class::~Class\n";}
};

struct Functor {
    int operator()(int i) const { return 3 * i; }
};

Fun< int(int) > RetFun() {
    return Functor();
}

void test1 () {
    Class c;
    const Class cc;
    Fun< int (int) >   fun1(&Twice);  
    Fun< int (int) >   fun2(&Class::Twice);
    Fun< int () >      fun3(&VoidParam);
    Fun< int () >      fun4(&Class::VoidParam);
    Fun< int () >      fun5(&Class::VoidParamConst);
    Fun< void(int&) >  fun6(&Class::Void);
    Fun< int (int) >   fun7((Functor()));
    auto               fun8(RetFun());
   
    assert(fun1(2)    ==  4);
    assert(fun2(c, 3) ==  6);
    assert(fun5(cc)   ==  2);
    assert(fun7(6)    == 18);
    assert(fun8(7)    == 21);
    // int i = 0;
    // fun6(c, i);
    // assert(i == 1234);
}

//------------------------------------------------------------------------------

int main(int, char**) {
    test1();
    return 0;
}               