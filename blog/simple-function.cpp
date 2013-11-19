//Author; Ugo Varetto
//simple implementation of std::function-like class using lambdas and unions
//TODO: assignment operator

#include <cassert>
#include <iostream>
#include <vector>
#include <stdexcept>
//==============================================================================
//generic, need this to be able to declare Fun< int(int) > instead of
//Fun< int, int >
template < typename > struct Fun {};

template < typename > struct FunBase {};




template < typename R, typename...ArgTypes >
struct FunBase< R (ArgTypes...) > {
    struct Object {}; //generic object type used to cast member methods
    //constructors
    FunBase() {}
    FunBase(const FunBase& fun) : f_(fun.f_), m_(fun.m_) {
        CopyObj(fun, *this);
    }
    FunBase(R (*f)(ArgTypes...));
    template < typename T > FunBase(R (T::*f)(ArgTypes...) );
    template < typename T > FunBase(R (T::*f)(ArgTypes...) const);
    template < typename T > FunBase(const T& f);
    FunBase(FunBase&&) = default;
    FunBase& operator=(FunBase f) {
        Destruct(this);
        f_ = f.f_;
        m_ = f.m_;
        buf_ = std::move(f.buf_);
        return *this;
    }
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
        R(*call_)(const FunBase*, ArgTypes...);
        R(*obj_)(FunBase*, ArgTypes...);
    } m_;
    void (*Destruct)(FunBase*) = nullptr;
    void (*CopyObj)(const FunBase&, FunBase&) = nullptr;
    //storage for functor objects, in a real-world scenario we would need to
    //either pass a custom allocator using e.g. stack memory for small objects
    //or use a custom data type
    std::vector< char > buf_;
    ~FunBase() {
        Destruct(this);
     }
};

//constructors
template < typename R, typename...ArgTypes >
FunBase< R (ArgTypes...) >::FunBase(R (*f)(ArgTypes...)) { 
    f_.free_ = f; 
    using T = R (*)(const FunBase*, ArgTypes...);
    m_.call_ = (T) ([](const FunBase* f, ArgTypes...args) {
         return f->f_.free_(std::forward< ArgTypes >(args)...);
    });
    Destruct = [](FunBase*) {};
    CopyObj  = [](const FunBase&, FunBase&) {};
}

template < typename R, typename...ArgTypes >
template < typename T >
FunBase< R(ArgTypes...) >::FunBase(R (T::*f) (ArgTypes...) ) {
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
    Destruct = [](FunBase*) {};
    CopyObj  = [](const FunBase&, FunBase&) {};
}

template < typename R, typename...ArgTypes >
template < typename T >
FunBase< R(ArgTypes...) >::FunBase(R (T::*f) (ArgTypes...) const) {
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
    Destruct = [](FunBase*) {}; 
    CopyObj  = [](const FunBase&, FunBase&) {};
}

template < typename R, typename...ArgTypes >
template < typename F >
FunBase< R (ArgTypes...) >::FunBase(const F& f) {
    buf_.resize(sizeof(f));
    new (&buf_[0]) F(f);
    using FF = R (*)(FunBase*, ArgTypes...);
    m_.obj_ = (FF) ([](FunBase* f, ArgTypes...args) {
        return reinterpret_cast< F* >(&f->buf_[0])
                                ->operator()(std::forward< ArgTypes >(args)...);
    });
    Destruct = [](FunBase* f) {
        F* fun = reinterpret_cast< F* >(&f->buf_[0]);
        fun->F::~F();
    };
    CopyObj  = [](const FunBase& src, FunBase& target) {
        target.buf_.resize(src.buf_.size());
        new (&target.buf_[0]) FunBase(src);
    };
}

//callable entities with a return type
template < typename R, typename...ArgTypes >
struct Fun< R (ArgTypes...) > : FunBase< R (ArgTypes...) > {
    using Base = FunBase< R(ArgTypes...) >;
    Fun() : Base([](ArgTypes...) { 
        throw std::logic_error("EMPTY FUNCTION OBJECT");
        return R();
    }){}
    Fun(const Fun& f) : Base(f) {}
    Fun(Fun&& f) :  Base(f) {} 
    Fun(R (*f)(ArgTypes...)) :  Base(f) {}
    template < typename T > Fun(R (T::*f)(ArgTypes...) ) : Base(f) {}
    template < typename T > Fun(R (T::*f)(ArgTypes...) const) : Base(f) {}
    template < typename T > Fun(const T& f) : Base(f) {}
    Fun& operator=(Fun f) {
        Base::operator=(f);
        return *this;
    }    
    //operator()
    R operator()(ArgTypes...args) const;
    template < typename T > R operator()(T& obj, ArgTypes...args) const;
    template < typename T > R operator()(const T& obj, ArgTypes...args) const; 
};

template < typename R, typename...ArgTypes >
R Fun< R (ArgTypes...) >::operator()(ArgTypes...args) const {
   return Base::m_.call_(this, args...);
}

template < typename R, typename...ArgTypes >
template < typename T >
R Fun< R (ArgTypes...) >::operator()(T& obj, ArgTypes...args) const {
    return Base::f_.method_(&obj, Base::m_.method_, args...);
}

template < typename R, typename...ArgTypes >
template < typename T >
R Fun< R (ArgTypes...) >::operator()(const T& obj, ArgTypes...args) const {
    return Base::f_.cmethod_(&obj, Base::m_.cmethod_, args...);
}

//callable entities with void return type
template < typename...ArgTypes >
struct Fun< void (ArgTypes...) > : FunBase< void (ArgTypes...) > {
    using Base = FunBase< void (ArgTypes...) >;
    Fun() : Base([](ArgTypes...) { 
        throw std::logic_error("EMPTY FUNCTION OBJECT");
    }){}
    Fun(const Fun& f) : Base(f) {}
    Fun(Fun&& f) :  Base(f) {} 
    Fun(void (*f)(ArgTypes...)) :  Base(f) {}
    template < typename T > Fun(void (T::*f)(ArgTypes...) ) : Base(f) {}
    template < typename T > Fun(void (T::*f)(ArgTypes...) const) : Base(f) {}
    template < typename T > Fun(const T& f) : Base(f) {}
    Fun& operator=(Fun f) {
        Base::operator=(f);
        return *this;
    }    
    //operator()
    void operator()(ArgTypes...args) const;
    template < typename T > void operator()(T& obj, ArgTypes...args) const;
    template < typename T > 
                      void operator()(const T& obj, ArgTypes...args) const; 
};

template < typename...ArgTypes >
void Fun< void (ArgTypes...) >::operator()(ArgTypes...args) const {
    Base::m_.call_(this, args...);
}

template < typename...ArgTypes >
template < typename T >
void Fun< void (ArgTypes...) >::operator()(T& obj, ArgTypes...args) const {
    Base::f_.method_(&obj, Base::m_.method_, args...);
}

template < typename...ArgTypes >
template < typename T >
void Fun< void (ArgTypes...) >::operator()(const T& obj, ArgTypes...args) 
const {
    Base::f_.cmethod_(&obj, Base::m_.cmethod_, args...);
}

//------------------------------------------------------------------------------
int Twice(int i) { return 2 * i; }

int VoidParam() { return 2; }

struct Class {
    int Twice(int i) { return 2 * i; }
    int VoidParam() { return 2; }
    int VoidParamConst() const { return 2; }
    void Void(int& i) const {i = 1234;}
    Class() {}
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
    Fun< int (int) >   fun0;
    Fun< int (int) >   fun1(&Twice);  
    Fun< int (int) >   fun2(&Class::Twice);
    Fun< int () >      fun3(&VoidParam);
    Fun< int () >      fun4(&Class::VoidParam);
    Fun< int () >      fun5(&Class::VoidParamConst);
    Fun< void(int&) >  fun6(&Class::Void);
    Fun< int (int) >   fun7((Functor()));
    auto               fun8(RetFun());

    try { 
        fun0(9); //must throw
        assert(false); 
    } catch(...) {}
    //fun0 = fun2;
    //assert(fun0(2)    == fun1(2));
    assert(fun1(2)    ==  4);
    assert(fun2(c, 3) ==  6);
    assert(fun5(cc)   ==  2);
    assert(fun7(6)    == 18);
    assert(fun8(7)    == 21);
    //void return type
    int i = 0;
    fun6(c, i);
    assert(i == 1234);
}

//------------------------------------------------------------------------------

int main(int, char**) {
    test1();
    return 0;
}               