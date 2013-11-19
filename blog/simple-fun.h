#pragma once
//Author; Ugo Varetto
//simple implementation of std::function-like class using lambdas and unions

//generic, need this to be able to declare Fun< int(int) > instead of
//Fun< int, int >
template < typename > struct Fun {};

template < typename > struct FunBase {};

template < typename R, typename...ArgTypes >
class FunBase< R (ArgTypes...) > {
//make everything only accessible to derived type  
protected:    
    struct Object {}; //generic object type used to cast member methods
    //constructors
    FunBase() {}
    FunBase(const FunBase& fun) 
        : f_(fun.f_),
          m_(fun.m_),
          CopyObj(fun.CopyObj),
          Destruct(fun.Destruct) {
        CopyObj(fun, *this);
    }
    FunBase(R (*f)(ArgTypes...));
    template < typename T > FunBase(R (T::*f)(ArgTypes...) );
    template < typename T > FunBase(R (T::*f)(ArgTypes...) const);
    template < typename T > FunBase(const T& f);
    FunBase(FunBase&&) = default;
    //problem: cannot use the standard C++11 idiom
    //operator=(FunBase f) then move because operator= is invoked
    //from derived class passing an instance of Fun which is then
    //used to construct an instance of FunBase through the 
    //template < typename T > FunBase(const T& f) constructor
    FunBase& operator=(const FunBase& f) {
        if(&f == this) return *this;
        if(Destruct) Destruct(this);
        f_ = f.f_;
        m_ = f.m_;
        CopyObj = f.CopyObj;
        Destruct = f.Destruct;
        buf_ = f.buf_;
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
        if(Destruct) Destruct(this);
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
        new (&target.buf_[0]) F(*reinterpret_cast< const F* >(src.buf_[0]));
    };
}

//callable entities with a return type
template < typename R, typename...ArgTypes >
class Fun< R (ArgTypes...) > : FunBase< R (ArgTypes...) > {
public:    
    using Base = FunBase< R(ArgTypes...) >;
    Fun() : Base([](ArgTypes...) { 
        throw std::logic_error("EMPTY FUNCTION OBJECT");
        return R();
    }){}
    //without the cast the wrong (templated) constructor is invoked
    //and f is interpreted as a generic function object
    Fun(const Fun& f) : Base(static_cast< const Base& >(f)) {}
    Fun(Fun&& f) :  Base(std::forward< Base >(f)) {} 
    Fun(R (*f)(ArgTypes...)) :  Base(f) {}
    template < typename T > Fun(R (T::*f)(ArgTypes...) ) : Base(f) {}
    template < typename T > Fun(R (T::*f)(ArgTypes...) const) : Base(f) {}
    template < typename T > Fun(const T& f) : Base(f) {}
    Fun& operator=(const Fun& f) {
        //need to explicitly invoke the base operator with proper type
        //if not the base class interprets Fun as a generic functor type
        //and tries to construct a temporary FunBase object with it
        Base::operator=(static_cast< const Base& >(f));
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
class Fun< void (ArgTypes...) > : FunBase< void (ArgTypes...) > {
public:    
    using Base = FunBase< void (ArgTypes...) >;
    Fun() : Base([](ArgTypes...) { 
        throw std::logic_error("EMPTY FUNCTION OBJECT");
    }){}
    //without the cast the wrong (templated) constructor is invoked
    //and f is interpreted as a generic function object
    Fun(const Fun& f) : Base(static_cast< const Base& >(f)) {}
    Fun(Fun&& f) :  Base(std::forward< Base >(f)) {}
    Fun(void (*f)(ArgTypes...)) :  Base(f) {}
    template < typename T > Fun(void (T::*f)(ArgTypes...) ) : Base(f) {}
    template < typename T > Fun(void (T::*f)(ArgTypes...) const) : Base(f) {}
    template < typename T > Fun(const T& f) : Base(f) {}
    Fun& operator=(const Fun& f) {
        //need to explicitly invoke the base operator with proper type
        //if not the base class interprets Fun as a generic functor type
        //and tries to construct a temporary FunBase object with it
        Base::operator=(static_cast< const Base& >(f));
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
