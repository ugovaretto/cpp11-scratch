//Author: Ugo Varetto
//Benchmark of various approaches to value semantics with and without virtual
//functions
//NOTE on destruction:
//for non-virtual derived class the run-time invokes the base class destructor; 
//it is therefore required that the base destructor invoke the derived 
//instance destructor or the wrapped-data destructor directly.

//Options:
//-DNOVIRTUAL: use non-virtual methods for reference test
//-DUSE_REFS:  use references instead of direct element access in tests

//SUMMARY:
//any approach other than std::function with clang's libc++ is fine;
//static vtables are much faster than anything else when lto enabled
//and outperform even regular non-virtual method invocations;
//if instead of static function pointers, non-static pointers are used the
//performance is equal to virtual methods under -O3 optimizations;
//when using xxx::function it is not worth trying with closures: do use regular
//functions which accept as the first parameter the model_t<> pointer to
//operate on, using a closure on this or model_t.d results in >40% performance
//penalty;

//to see the benefits of a static vtable you need link time optimization
//clang on Apple: do use -O3 + lto
//gcc or clang on linux: do install the gold linker (binutils-gold) then
//rebuild gcc or clang with lto enabled

//the printed time in nanoseconds is computed as
// global time /
// (num executions 
//  * num elements
//  * num function calls per elements)
// and is proportional to the time it takes to perform a single method
// invocation but it is *not* the actual method invocation time since e.g. 
// in the case of non-virtual methods or full lto optimization with static
// functions everything seems to be inlined

//checkout results at the bottom of the file

//Also: try to replace the set(get() + get()) expression with more complex e.g.
//sqrt(get() +*/- get()) you'll find that most of the time is still spent in the
//actual function/method invocation

//Note 1: on linux it seems that the std::function implementation found in
//clang libc++ (svn snapshot on 20131128) with llvm 3.4 is actually 
//slower than the gcc 4.8.1 version

//Note 2: replacing direct element access with access through references gives
//better performance under any -O level

//Note 3: try to change the number of elements in the test array from
//cache-friendly (e.g. 1024, or anything small) to bigger (100k)
//values: for big numbers lto has no effect and function pointers and
//virtual methods perform better than the actual wrapped objects

#include <cassert>
#include <functional>
#include <memory>
#include <chrono>
#include <ctime>
#include <string>
#include <vector>
#include <cstring>
#include <iostream>
#include "../etc/function.h" //Malte Skarupke's std::function
#include "../simple-fun.h" //my own quick implementation of std::function always
                           //faster than std and Malte's when using static
                           //members
#include <algorithm>
                        
using namespace std;

#ifdef NOVIRTUAL
#define VIR
#else
#define VIR virtual
#endif

//------------------------------------------------------------------------------
//std::function [WORST]
struct wrapper_t {
    float GetX() const { return model_->GetX(); }
    float GetY() const { return model_->GetY(); }
    float GetZ() const { return model_->GetZ(); }
    float GetW() const { return model_->GetW(); }
    float SetX( float x_ ) { return model_->SetX(x_); }
    float SetY( float y_ ) { return model_->SetY(y_); }
    float SetZ( float z_ ) { return model_->SetZ(z_); }
    float SetW( float w_ ) { return model_->SetW(w_); }
    wrapper_t() = default;
    wrapper_t(wrapper_t&& ) = default;
    wrapper_t(const wrapper_t& w) : model_(w.model_->Copy()) {}
    template < typename T >
    wrapper_t(const T& t) : model_(new model_t< T >(t)) {}
    template < typename T >
    T& get() {
        return static_cast< model_t< T >& >(*model_).d;
    }
    struct base_t {
        ~base_t() {Destroy();}
        static std::function< float (const void*) > GetXImpl;
        static std::function< float (const void*) > GetYImpl;
        static std::function< float (const void*) > GetZImpl;
        static std::function< float (const void*) > GetWImpl;
        static std::function< float (void*, float) > SetXImpl;
        static std::function< float (void*, float) > SetYImpl;
        static std::function< float (void*, float) > SetZImpl;
        static std::function< float (void*, float) > SetWImpl;
        std::function< base_t* () > Copy;
        std::function< void () > Destroy;
        float GetX() const { return GetXImpl(this); }
        float GetY() const { return GetYImpl(this); }
        float GetZ() const { return GetZImpl(this); }
        float GetW() const { return GetWImpl(this); }
        float SetX(float x) { return SetXImpl(this, x); }
        float SetY(float y) { return SetYImpl(this, y); }
        float SetZ(float z) { return SetZImpl(this, z); }
        float SetW(float w) { return SetWImpl(this, w); }
    };
    template < typename T >
    struct model_t : base_t {
        T d;
        model_t(const T& t) : d(t) {
            BuildVTable();
        }    
        model_t(const model_t& m) : d(m.d) {
            BuildVTable();
        }
        void BuildVTable() {
            GetXImpl = [](const void* p) {
                return static_cast< const model_t< T >* >(p)->d.GetX();            
            };
            GetYImpl = [](const void* p) {
                return static_cast< const model_t< T >* >(p)->d.GetY();            
            };
            GetZImpl = [](const void* p) {
                return static_cast< const model_t< T >* >(p)->d.GetZ();            
            };
            GetWImpl = [](const void* p) {
                return static_cast< const model_t< T >* >(p)->d.GetW();            
            };
            SetXImpl = [](void* p, float v) {
                return static_cast< model_t< T >* >(p)->d.SetX(v);            
            };
            SetYImpl = [](void* p, float v) {
                return static_cast< model_t< T >* >(p)->d.SetY(v);            
            };
            SetZImpl = [](void* p, float v) {
                return static_cast< model_t< T >* >(p)->d.SetZ(v);            
            };
            SetWImpl = [](void* p, float v) {
                return static_cast< model_t< T >* >(p)->d.SetW(v);            
            };
            Copy = [this]() {
                return new model_t< T >(static_cast< model_t& >(*this));
            };
            Destroy = [this]() {
                static_cast< model_t< T >& >(*this).d.T::~T();
            };                    
        }
    };
    unique_ptr< base_t > model_;
};


std::function< float (const void*) > wrapper_t::base_t::GetXImpl;
std::function< float (const void*) > wrapper_t::base_t::GetYImpl;
std::function< float (const void*) > wrapper_t::base_t::GetZImpl;
std::function< float (const void*) > wrapper_t::base_t::GetWImpl;
std::function< float (void*, float) > wrapper_t::base_t::SetXImpl;
std::function< float (void*, float) > wrapper_t::base_t::SetYImpl;
std::function< float (void*, float) > wrapper_t::base_t::SetZImpl;
std::function< float (void*, float) > wrapper_t::base_t::SetWImpl;

//------------------------------------------------------------------------------
//static function pointers
//fastest, with lto enabled is faster than regualar non-virtual methods!
struct wrapper2_t {
    float GetX() const { return model_->GetX(); }
    float GetY() const { return model_->GetY(); }
    float GetZ() const { return model_->GetZ(); }
    float GetW() const { return model_->GetW(); }
    float SetX( float x_ ) { return model_->SetX(x_); }
    float SetY( float y_ ) { return model_->SetY(y_); }
    float SetZ( float z_ ) { return model_->SetZ(z_); }
    float SetW( float w_ ) { return model_->SetW(w_); }
    wrapper2_t() = default;
    wrapper2_t(wrapper2_t&& ) = default;
    wrapper2_t(const wrapper2_t& w) : model_(w.model_->Copy()) {}
    template < typename T >
    wrapper2_t(const T& t) : model_(new model_t< T >(t)) {}  
    template < typename T >
    T& get() {
        return static_cast< model_t< T >& >(*model_).d;
    }
    struct base_t {
        using GF = float (*)(const void* );
        using SF = float (*)(void*, float);
        using CP = base_t* (*)(const void*);
        using D  = void (*)(void*);
        struct vtable {
            static GF GetXImpl;
            static GF GetYImpl;
            static GF GetZImpl;
            static GF GetWImpl;
            static SF SetXImpl;
            static SF SetYImpl;
            static SF SetZImpl;
            static SF SetWImpl;

        };
        template < typename T >
        base_t(const T& ) {
            BuildVTable< T >();         
        }
        ~base_t() {Destroy();}
        template < typename T >
        void BuildVTable() {
            if(!vtable::GetXImpl) {
            vtable::GetXImpl = (GF)([](const void* self) {
                return reinterpret_cast< const model_t< T >* >(self)->d.GetX();
                           
            });
            vtable::GetYImpl = (GF)([](const void* self) {
                return reinterpret_cast< const model_t< T >* >(self)->d.GetY();
                           
            });
            vtable::GetZImpl = (GF)([](const void* self) {
                return reinterpret_cast< const model_t< T >* >(self)->d.GetZ();
                           
            });
            vtable::GetWImpl = (GF)([](const void* self) {
                return reinterpret_cast< const model_t< T >* >(self)->d.GetW();
                           
            });
            vtable::SetXImpl = (SF)([](void* self, float x_) {
                return reinterpret_cast< model_t< T >* >(self)->d.SetX(x_);
            });
            vtable::SetYImpl = (SF)([](void* self, float y_) {
                return reinterpret_cast< model_t< T >* >(self)->d.SetY(y_);
            });
            vtable::SetZImpl = (SF)([](void* self, float z_) {
                return reinterpret_cast< model_t< T >* >(self)->d.SetZ(z_);
            });
            vtable::SetWImpl = (SF)([](void* self, float w_) {
                return reinterpret_cast< model_t< T >* >(self)->d.SetW(w_);
            });
            CopyImpl = (CP)([](const void* self) {
                return reinterpret_cast< const model_t< T >* >(self)->Copy();
            });
            DestroyImpl = (D)([](void* self) {
                reinterpret_cast< model_t< T >* >(self)->d.T::~T();
            });
            }
        }
        float GetX() const { return vtable::GetXImpl(this); }
        float GetY() const { return vtable::GetYImpl(this); }
        float GetZ() const { return vtable::GetZImpl(this); }
        float GetW() const { return vtable::GetWImpl(this); }
        float SetX(float x_) { return vtable::SetXImpl(this, x_); }
        float SetY(float y_) { return vtable::SetYImpl(this, y_); }
        float SetZ(float z_) { return vtable::SetZImpl(this, z_); }
        float SetW(float w_) { return vtable::SetWImpl(this, w_); }
        base_t* Copy() const { return CopyImpl(this); }
        void Destroy() { DestroyImpl(this); }       
        
        static CP CopyImpl;
        static D DestroyImpl;
    };
    template < typename T >
    struct model_t : base_t {
        T d;
        model_t(const T& t) : base_t(t), d(t) {}
        base_t* Copy() const { return new model_t(*this); }
    };
    
    unique_ptr< base_t > model_;
};
using GF = float (*)(const void* );
using SF = float (*)(void*, float);
using CP = wrapper2_t::base_t* (*)(const void*);
using D  = void (*)(void*);
GF wrapper2_t::base_t::vtable::GetXImpl = nullptr;
GF wrapper2_t::base_t::vtable::GetYImpl = nullptr;
GF wrapper2_t::base_t::vtable::GetZImpl = nullptr;
GF wrapper2_t::base_t::vtable::GetWImpl = nullptr;
SF wrapper2_t::base_t::vtable::SetXImpl = nullptr;
SF wrapper2_t::base_t::vtable::SetYImpl = nullptr;
SF wrapper2_t::base_t::vtable::SetZImpl = nullptr;
SF wrapper2_t::base_t::vtable::SetWImpl = nullptr;
CP wrapper2_t::base_t::CopyImpl         = nullptr;
D  wrapper2_t::base_t::DestroyImpl      = nullptr;

//------------------------------------------------------------------------------
//virtual
struct wrapper3_t {
    float GetX() const { return model_->GetX(); }
    float GetY() const { return model_->GetY(); }
    float GetZ() const { return model_->GetZ(); }
    float GetW() const { return model_->GetW(); }
    float SetX( float x_ ) { return model_->SetX(x_); }
    float SetY( float y_ ) { return model_->SetY(y_); }
    float SetZ( float z_ ) { return model_->SetZ(z_); }
    float SetW( float w_ ) { return model_->SetW(w_); }
    wrapper3_t() = default;
    wrapper3_t(wrapper3_t&& ) = default;
    wrapper3_t(const wrapper3_t& w) : model_(w.model_->Copy()) {}
    template < typename T >
    wrapper3_t(const T& t) : model_(new model_t< T >(t)) {}  
    template < typename T >
    T& get() {
        return static_cast< model_t< T >& >(*model_).d;
    }
    struct base_t {
        virtual float GetX() const = 0;
        virtual float GetY() const = 0;
        virtual float GetZ() const = 0;
        virtual float GetW() const = 0;
        virtual float SetX( float x_ ) = 0;
        virtual float SetY( float y_ ) = 0;
        virtual float SetZ( float z_ ) = 0;
        virtual float SetW( float w_ ) = 0;
        virtual base_t* Copy() const = 0;

        virtual ~base_t() {}
    };
    template < typename T >
    struct model_t final: base_t {
        T d;
        model_t(const T& t) : d(t) {}    
        virtual float GetX() const { return d.GetX(); }
        virtual float GetY() const { return d.GetY(); }
        virtual float GetZ() const { return d.GetZ(); }
        virtual float GetW() const { return d.GetW(); }
        virtual float SetX( float x_ ) { return d.SetX(x_); }
        virtual float SetY( float y_ ) { return d.SetY(y_); }
        virtual float SetZ( float z_ ) { return d.SetZ(z_); }
        virtual float SetW( float w_ ) { return d.SetW(w_); }
        virtual base_t* Copy() const { return new model_t< T >(*this);}

    };
    unique_ptr< base_t > model_;
};

//------------------------------------------------------------------------------
//my own std::function implementation, always sligthly faster than std
struct wrapper4_t {
    float GetX() const { return model_->GetX(); }
    float GetY() const { return model_->GetY(); }
    float GetZ() const { return model_->GetZ(); }
    float GetW() const { return model_->GetW(); }
    float SetX( float x_ ) { return model_->SetX(x_); }
    float SetY( float y_ ) { return model_->SetY(y_); }
    float SetZ( float z_ ) { return model_->SetZ(z_); }
    float SetW( float w_ ) { return model_->SetW(w_); }
    wrapper4_t() = default;
    wrapper4_t(wrapper4_t&& ) = default;
    wrapper4_t(const wrapper4_t& w) : model_(w.model_->Copy()) {}
    template < typename T >
    wrapper4_t(const T& t) : model_(new model_t< T >(t)) {}
    template < typename T >
    T& get() {
        return static_cast< model_t< T >& >(*model_).d;
    }
    struct base_t {
        ~base_t() {Destroy();}
        static Fun< float (const void*) > GetXImpl;
        static Fun< float (const void*) > GetYImpl;
        static Fun< float (const void*) > GetZImpl;
        static Fun< float (const void*) > GetWImpl;
        static Fun< float (void*, float) > SetXImpl;
        static Fun< float (void*, float) > SetYImpl;
        static Fun< float (void*, float) > SetZImpl;
        static Fun< float (void*, float) > SetWImpl;
        Fun< base_t* () > Copy;
        Fun< void () > Destroy;
        float GetX() const { return GetXImpl(this); }
        float GetY() const { return GetYImpl(this); }
        float GetZ() const { return GetZImpl(this); }
        float GetW() const { return GetWImpl(this); }
        float SetX(float x) { return SetXImpl(this, x); }
        float SetY(float y) { return SetYImpl(this, y); }
        float SetZ(float z) { return SetZImpl(this, z); }
        float SetW(float w) { return SetWImpl(this, w); }

    };
    template < typename T >
    struct model_t : base_t {
        T d;
        model_t(const T& t) : d(t) {
            BuildVTable();
        }    
        model_t(const model_t& m) : d(m.d) {
            BuildVTable();
        }
        void BuildVTable() {
            GetXImpl = [](const void* p) {
                return static_cast< const model_t< T >* >(p)->d.GetX();            
            };
            GetYImpl = [](const void* p) {
                return static_cast< const model_t< T >* >(p)->d.GetY();            
            };
            GetZImpl = [](const void* p) {
                return static_cast< const model_t< T >* >(p)->d.GetZ();            
            };
            GetWImpl = [](const void* p) {
                return static_cast< const model_t< T >* >(p)->d.GetW();            
            };
            SetXImpl = [](void* p, float v) {
                return static_cast< model_t< T >* >(p)->d.SetX(v);            
            };
            SetYImpl = [](void* p, float v) {
                return static_cast< model_t< T >* >(p)->d.SetY(v);            
            };
            SetZImpl = [](void* p, float v) {
                return static_cast< model_t< T >* >(p)->d.SetZ(v);            
            };
            SetWImpl = [](void* p, float v) {
                return static_cast< model_t< T >* >(p)->d.SetW(v);            
            };
            Copy = [this]() {
                return new model_t< T >(static_cast< model_t& >(*this));
            };
            Destroy = [this]() {
                static_cast< model_t< T >& >(*this).d.T::~T();
            };                    
        }
    };
    unique_ptr< base_t > model_;
};

Fun< float (const void*) > wrapper4_t::base_t::GetXImpl;
Fun< float (const void*) > wrapper4_t::base_t::GetYImpl;
Fun< float (const void*) > wrapper4_t::base_t::GetZImpl;
Fun< float (const void*) > wrapper4_t::base_t::GetWImpl;
Fun< float (void*, float) > wrapper4_t::base_t::SetXImpl;
Fun< float (void*, float) > wrapper4_t::base_t::SetYImpl;
Fun< float (void*, float) > wrapper4_t::base_t::SetZImpl;
Fun< float (void*, float) > wrapper4_t::base_t::SetWImpl;

//------------------------------------------------------------------------------
//pointer to members initialized in derived class
struct wrapper5_t {
    float GetX() const { return model_->GetX(); }
    float GetY() const { return model_->GetY(); }
    float GetZ() const { return model_->GetZ(); }
    float GetW() const { return model_->GetW(); }
    float SetX( float x_ ) { return model_->SetX(x_); }
    float SetY( float y_ ) { return model_->SetY(y_); }
    float SetZ( float z_ ) { return model_->SetZ(z_); }
    float SetW( float w_ ) { return model_->SetW(w_); }
    wrapper5_t() = default;
    wrapper5_t(wrapper5_t&& ) = default;
    wrapper5_t(const wrapper5_t& w) : model_(w.model_->Copy()) {}
    template < typename T >
    wrapper5_t(const T& t) : model_(new model_t< T >(t)) {}
    template < typename T >
    T& get() {
        return static_cast< model_t< T >& >(*model_).d;
    }
    struct base_t {
        float GetX() const {return (this->*GetXImpl)();}
        float GetY() const {return (this->*GetYImpl)();}
        float GetZ() const {return (this->*GetZImpl)();}
        float GetW() const {return (this->*GetWImpl)();}
        float SetX( float x_ ) {return (this->*SetXImpl)(x_);}
        float SetY( float y_ ) {return (this->*SetYImpl)(y_);}
        float SetZ( float z_ ) {return (this->*SetZImpl)(z_);}
        float SetW( float w_ ) {return (this->*SetWImpl)(w_);}
        base_t* Copy() const { return (this->*CopyImpl)();}
        using GF = float (base_t::*)() const;
        using SF = float (base_t::*)(float);
        using CP = base_t* (base_t::*)() const; 
        //using D = void (base_t::*)();
        GF GetXImpl;
        GF GetYImpl;
        GF GetZImpl;
        GF GetWImpl;
        SF SetXImpl;
        SF SetYImpl;
        SF SetZImpl;
        SF SetWImpl;
        CP CopyImpl;
        void (base_t::*DestroyImpl)();
        ~base_t() { 
            if(!destroyed) {
                (this->*DestroyImpl)(); 
                destroyed = true;
            }
        }
        bool destroyed = false;
    };
    template < typename T >
    struct model_t : base_t {
        T d;
        model_t(const T& t) : d(t) {
            GetXImpl = (GF) &model_t< T >::GetX;
            GetYImpl = (GF) &model_t< T >::GetY;
            GetZImpl = (GF) &model_t< T >::GetZ;
            GetWImpl = (GF) &model_t< T >::GetW;
            SetXImpl = (SF) &model_t< T >::SetX;
            SetYImpl = (SF) &model_t< T >::SetY;
            SetZImpl = (SF) &model_t< T >::SetZ;
            SetWImpl = (SF) &model_t< T >::SetW;
            CopyImpl = (CP) &model_t< T >::Copy;
            DestroyImpl = (void (base_t::*)()) &model_t< T >::Destroy;
        }    
        float GetX() const { return d.GetX(); }
        float GetY() const { return d.GetY(); }
        float GetZ() const { return d.GetZ(); }
        float GetW() const { return d.GetW(); }
        float SetX( float x_ ) { return d.SetX(x_); }
        float SetY( float y_ ) { return d.SetY(y_); }
        float SetZ( float z_ ) { return d.SetZ(z_); }
        float SetW( float w_ ) { return d.SetW(w_); }
        base_t* Copy() const { return new model_t< T >(*this);}
        ~model_t() = default;
        void Destroy() { d.T::~T(); }

    };
    unique_ptr< base_t > model_;
};

//------------------------------------------------------------------------------
//Callbacks, similar to 
//http://www.codeproject.com/Articles/136799/Lightweight-Generic-C-Callbacks-or-Yet-Another-Del
//not worth, approach is not faster than the other techniques not based 
template < typename R, typename P >
struct Callback {
    typedef R (*FuncType)(void*, P);
    Callback() : func(0), obj(0) {}
    Callback(const Callback&) = default;
    Callback(const FuncType f, void*  o) : func(f), obj(o) {}
    R operator()(P p) { return func(obj, p); }
    void operator=(const Callback& c) const {
        FuncType& r = const_cast< FuncType& >(func);
        r = c.func;
        obj = c.obj; 
    }
    const FuncType func;
    mutable void*  obj;
};

template < typename R, typename P >
struct CCallback {
    typedef R (*FuncType)(const void*, P);
    CCallback() : func(0), obj(0) {}
    CCallback(const CCallback&) = default;
    CCallback(const FuncType f, const void*  o) : func(f), obj(o) {}
    R operator()(P p) { return func(obj, p); }
     void operator=(const CCallback& c) const {
        FuncType& r = const_cast< FuncType& >(func);
        r = c.func;
        obj = c.obj;  
    }
    const FuncType func;
    mutable const void*  obj;
};

template < typename R >
struct Callback<R, void> {
    typedef R (*FuncType)(const void*);
    Callback(const FuncType f, const void*  o) : func(f), obj(o) {}
    Callback(const Callback&) = default;
    R operator()() { return func(obj); }
    Callback() : func(0), obj(0) {}
    const FuncType func;
    mutable const void*  obj;
     void operator=(const Callback& c) const {
        FuncType& r = const_cast< FuncType& >(func);
        r = c.func;
        obj = c.obj;  
    }
};


template < typename R, typename T, typename P, R (T::*Func)(P) >
R Wrapper(void*  obj, P p) {
    T* pp = reinterpret_cast< T* >(obj);
    return (pp->*Func)(p);
} 



template < typename R, typename T, typename P, R (T::*Func)(P) const >
static R WrapperC(const void*  obj, P p) {
    const T* pp = reinterpret_cast< const T* >(obj);
    return (pp->*Func)(p);
} 
    


template < typename R, typename T, R (T::*Func)() const >
R WrapperV(const void*  obj) {
    const T* pp = reinterpret_cast< const T* >(obj);
    return (pp->*Func)();
} 


struct wrapper6_t {
    using GF = Callback< float, void >;
    using SF = Callback< float, float >;
    mutable GF GetX;
    mutable GF GetY;
    mutable GF GetZ;
    mutable GF GetW;
    mutable SF SetX;
    mutable SF SetY;
    mutable SF SetZ;
    mutable SF SetW;
    wrapper6_t() = default;
    wrapper6_t(wrapper6_t&& ) = delete;
    wrapper6_t(const wrapper6_t& w) : model_(w.model_->Copy(this)) {
    }

    template < typename T >
    wrapper6_t(const T& t) : model_(new model_t< T >(t, this)) {}
    
    struct base_t {
        mutable func::function< base_t* (wrapper6_t*) > Copy;
        func::function< void () > Destroy;
        ~base_t() {Destroy();}
    };
    template < typename T >
    struct model_t : base_t {
        using GF = Callback< float, void >;
        using SF = Callback< float, float >;
        T d;
        model_t(const T& t, wrapper6_t* w) : d(t) {
            Copy = [this](wrapper6_t* w) {
                return new model_t< T >
                    (static_cast< const model_t< T >& >(*this), w);
            };       
            build_table(w);
        }
        model_t(const model_t& m, wrapper6_t* w) : d(m.d) {
            Copy = [this](wrapper6_t* w) {
                return new model_t< T >
                    (static_cast< const model_t< T >& >(*this), w);
            };  
            build_table(w);  
        }
        model_t(const model_t&) = delete;
        void build_table(wrapper6_t* w) {
            w->GetX = GF(&WrapperV< float, T, &T::GetX >, &d);
            w->GetY = GF(&WrapperV< float, T, &T::GetY >, &d);
            w->GetZ = GF(&WrapperV< float, T, &T::GetZ >, &d);
            w->GetW = GF(&WrapperV< float, T, &T::GetW >, &d);
            w->SetX = SF(&Wrapper< float, T, float, &T::SetX >, &d);
            w->SetY = SF(&Wrapper< float, T, float, &T::SetY >, &d);
            w->SetZ = SF(&Wrapper< float, T, float, &T::SetZ >, &d);
            w->SetW = SF(&Wrapper< float, T, float, &T::SetW >, &d);
            Destroy = [this]() {
                static_cast< model_t< T >& >(*this).d.T::~T();
            };
            
        } 
          
    };
    unique_ptr< base_t > model_;
};

//------------------------------------------------------------------------------
//alternative std::function implementation >50% faster than std::function
//Developed by Malte Skarupke
struct wrapper7_t {
    float GetX() const { return model_->GetX(); }
    float GetY() const { return model_->GetY(); }
    float GetZ() const { return model_->GetZ(); }
    float GetW() const { return model_->GetW(); }
    float SetX( float x_ ) { return model_->SetX(x_); }
    float SetY( float y_ ) { return model_->SetY(y_); }
    float SetZ( float z_ ) { return model_->SetZ(z_); }
    float SetW( float w_ ) { return model_->SetW(w_); }
    wrapper7_t() = default;
    wrapper7_t(wrapper7_t&& ) = default;
    wrapper7_t(const wrapper7_t& w) : model_(w.model_->Copy()) {}
    template < typename T >
    wrapper7_t(const T& t) : model_(new model_t< T >(t)) {}
    template < typename T >
    T& get() {
        return static_cast< model_t< T >& >(*model_).d;
    }
    struct base_t {
        ~base_t() {Destroy();}
        static func::function< float (const void*) > GetXImpl;
        static func::function< float (const void*) > GetYImpl;
        static func::function< float (const void*) > GetZImpl;
        static func::function< float (const void*) > GetWImpl;
        static func::function< float (void*, float) > SetXImpl;
        static func::function< float (void*, float) > SetYImpl;
        static func::function< float (void*, float) > SetZImpl;
        static func::function< float (void*, float) > SetWImpl;
        func::function< base_t* () > Copy;
        func::function< void () > Destroy;
        float GetX() const { return GetXImpl(this); }
        float GetY() const { return GetYImpl(this); }
        float GetZ() const { return GetZImpl(this); }
        float GetW() const { return GetWImpl(this); }
        float SetX(float x) { return SetXImpl(this, x); }
        float SetY(float y) { return SetYImpl(this, y); }
        float SetZ(float z) { return SetZImpl(this, z); }
        float SetW(float w) { return SetWImpl(this, w); }

    };
    template < typename T >
    struct model_t : base_t {
        T d;
        model_t(const T& t) : d(t) {
            BuildVTable();
        }    
        model_t(const model_t& m) : d(m.d) {
            BuildVTable();
        }
        void BuildVTable() {
            GetXImpl = [](const void* p) {
                return static_cast< const model_t< T >* >(p)->d.GetX();            
            };
            GetYImpl = [](const void* p) {
                return static_cast< const model_t< T >* >(p)->d.GetY();            
            };
            GetZImpl = [](const void* p) {
                return static_cast< const model_t< T >* >(p)->d.GetZ();            
            };
            GetWImpl = [](const void* p) {
                return static_cast< const model_t< T >* >(p)->d.GetW();            
            };
            SetXImpl = [](void* p, float v) {
                return static_cast< model_t< T >* >(p)->d.SetX(v);            
            };
            SetYImpl = [](void* p, float v) {
                return static_cast< model_t< T >* >(p)->d.SetY(v);            
            };
            SetZImpl = [](void* p, float v) {
                return static_cast< model_t< T >* >(p)->d.SetZ(v);            
            };
            SetWImpl = [](void* p, float v) {
                return static_cast< model_t< T >* >(p)->d.SetW(v);            
            };
            Copy = [this]() {
                return new model_t< T >(static_cast< model_t& >(*this));
            };
            Destroy = [this]() {
                static_cast< model_t< T >& >(*this).d.T::~T();
            };                    
        }
    };
    unique_ptr< base_t > model_;
};

func::function< float (const void*) > wrapper7_t::base_t::GetXImpl;
func::function< float (const void*) > wrapper7_t::base_t::GetYImpl;
func::function< float (const void*) > wrapper7_t::base_t::GetZImpl;
func::function< float (const void*) > wrapper7_t::base_t::GetWImpl;
func::function< float (void*, float) > wrapper7_t::base_t::SetXImpl;
func::function< float (void*, float) > wrapper7_t::base_t::SetYImpl;
func::function< float (void*, float) > wrapper7_t::base_t::SetZImpl;
func::function< float (void*, float) > wrapper7_t::base_t::SetWImpl;

//------------------------------------------------------------------------------
//same as wrapper2_t i.e. static function pointers but stored directly into
//the wrapper class instead of going through base_t->model_t member types
struct wrapper8_t {
    using GETTER  = float (*)(const void*);
    using SETTER  = float (*)(void*, float);
    using COPY    = void (*)(const void*, void*);
    using DESTROY = void (*)(const void* );
    static GETTER GetXImpl;
    static GETTER GetYImpl;
    static GETTER GetZImpl;
    static GETTER GetWImpl;
    static SETTER SetXImpl;
    static SETTER SetYImpl;
    static SETTER SetZImpl;
    static SETTER SetWImpl;
    static COPY Copy;
    static DESTROY Destroy; 
    template < typename T >
    void BuildVTable() {
        GetXImpl = [](const void* p) {
            return static_cast< const T* >(p)->GetX();            
        };
        GetYImpl = [](const void* p) {
            return static_cast< const T* >(p)->GetY();            
        };
        GetZImpl = [](const void* p) {
            return static_cast< const T* >(p)->GetZ();            
        };
        GetWImpl = [](const void* p) {
            return static_cast< const T* >(p)->GetW();            
        };
        SetXImpl = [](void* p, float v) {
            return static_cast< T* >(p)->SetX(v);            
        };
        SetYImpl = [](void* p, float v) {
            return static_cast< T* >(p)->SetY(v);                    
        };
        SetZImpl = [](void* p, float v) {
            return static_cast< T* >(p)->SetZ(v);                    
        };
        SetWImpl = [](void* p, float v) {
            return static_cast< T* >(p)->SetW(v);            
        };
        Copy = [](const void* src, void* target) {
            new (target) T(*static_cast< const T* >(src));
        };
        Destroy = [](const void* p) {
            static_cast< const T* >(p)->T::~T();
        };                    
    }
    float GetX() const { return GetXImpl(&model_[0]); }
    float GetY() const { return GetYImpl(&model_[0]); }
    float GetZ() const { return GetZImpl(&model_[0]); }
    float GetW() const { return GetWImpl(&model_[0]); }
    float SetX(float x) { return SetXImpl(&model_[0], x); }
    float SetY(float y) { return SetYImpl(&model_[0], y); }
    float SetZ(float z) { return SetZImpl(&model_[0], z); }
    float SetW(float w) { return SetWImpl(&model_[0], w); }

    ~wrapper8_t() { Destroy(&model_[0]); }
    wrapper8_t() = default;
    wrapper8_t(wrapper8_t&& ) = default;
    wrapper8_t(const wrapper8_t& w)  {
            model_.resize(sizeof(w.model_.size()));
            w.Copy(&w.model_[0], &model_[0]);   
        } 
    template < typename T >
    wrapper8_t(const T& t) {
        model_.resize(sizeof(t));
        new (&model_[0]) T(t);
        BuildVTable< T >();
    }
    template < typename T >
    T& get() {
        return *reinterpret_cast< T* >(&model_[0]);
    }
    //struct B {char c[8];};
    std::vector< char > model_; //using stack based allocation: ~20% faster
    //double model_[]
};

wrapper8_t::GETTER wrapper8_t::GetXImpl;
wrapper8_t::GETTER wrapper8_t::GetYImpl;
wrapper8_t::GETTER wrapper8_t::GetZImpl;
wrapper8_t::GETTER wrapper8_t::GetWImpl;
wrapper8_t::SETTER wrapper8_t::SetXImpl;
wrapper8_t::SETTER wrapper8_t::SetYImpl;
wrapper8_t::SETTER wrapper8_t::SetZImpl;
wrapper8_t::SETTER wrapper8_t::SetWImpl;
wrapper8_t::DESTROY wrapper8_t::Destroy;
wrapper8_t::COPY wrapper8_t::Copy;


//from
//http://assemblyrequired.crashworks.org/2009/01/19/how-slow-are-virtual-functions-really
//------------------------------------------------------------------------------
class Vector4TestV /*final*/ {
  float x = 0, y = 0, z = 0, w = 0;
public:
    VIR float GetX() const  { return x; }
    VIR float GetY() const  { return y; }
    VIR float GetZ() const  { return z; }
    VIR float GetW() const  { return w; }
    VIR float SetX( float x_ )  { return x = x_; }
    VIR float SetY( float y_ )  { return y = y_; }
    VIR float SetZ( float z_ )  { return z = z_; }
    VIR float SetW( float w_ )  { return w = w_; }
    VIR ~Vector4TestV() {}
};

class Vector4Test {
  float x = 0, y = 0, z = 0, w = 0;
public:
    float GetX() const { return x; }
    float GetY() const { return y; }
    float GetZ() const { return z; }
    float GetW() const { return w; }
    float SetX( float x_ ) { return x = x_; }
    float SetY( float y_ ) { return y = y_; }
    float SetZ( float z_ ) { return z = z_; }
    float SetW( float w_ ) { return w = w_; }
    ~Vector4Test() {}
};

//difference between cache-friendly and non-cache friendly values: static
//pointers wtih lto optimizaiton (almost) not affected!
static const int NUM_ELEMENTS = 1024;//37025; //122222;

//------------------------------------------------------------------------------
std::vector< shared_ptr< Vector4TestV > > A(NUM_ELEMENTS),
                                          B(NUM_ELEMENTS),
                                          C(NUM_ELEMENTS);
void test(int NUM_TESTS) {
    for (int n = 0 ; n != NUM_TESTS; ++n)
        for (int i=0; i != NUM_ELEMENTS ; ++i) {
#ifdef USE_REFS            
            Vector4TestV& c = *C[i];
            const Vector4TestV& a = *A[i];
            const Vector4TestV& b = *B[i];
            c.SetX(a.GetX() + b.GetX());
            c.SetY(a.GetY() + b.GetY());
            c.SetZ(a.GetZ() + b.GetZ());
            c.SetW(a.GetW() + b.GetW());
#else            
            C[i]->SetX(A[i]->GetX() + B[i]->GetX());
            C[i]->SetY(A[i]->GetY() + B[i]->GetY());
            C[i]->SetZ(A[i]->GetZ() + B[i]->GetZ());
            C[i]->SetW(A[i]->GetW() + B[i]->GetW());
#endif            
        }
}

std::vector< wrapper_t > Aw(NUM_ELEMENTS, wrapper_t(Vector4Test())),
                         Bw(NUM_ELEMENTS, wrapper_t(Vector4Test())),  
                         Cw(NUM_ELEMENTS, wrapper_t(Vector4Test()));
void testw(int NUM_TESTS) {
    for (int n = 0 ; n != NUM_TESTS; ++n)
        for (int i=0; i != NUM_ELEMENTS ; ++i) {
#ifdef USE_REFS            
            wrapper_t& C = Cw[i];
            const wrapper_t& A = Aw[i];
            const wrapper_t& B = Bw[i];
            C.SetX(A.GetX() + B.GetX());
            C.SetY(A.GetY() + B.GetY());
            C.SetZ(A.GetZ() + B.GetZ());
            C.SetW(A.GetW() + B.GetW());
#else
            Cw[i].SetX(Aw[i].GetX() + Bw[i].GetX());
            Cw[i].SetY(Aw[i].GetY() + Bw[i].GetY());
            Cw[i].SetZ(Aw[i].GetZ() + Bw[i].GetZ());
            Cw[i].SetW(Aw[i].GetW() + Bw[i].GetW());
#endif            
        }
}

std::vector< wrapper2_t > Aw2(NUM_ELEMENTS, wrapper2_t((Vector4Test()))),
                          Bw2(NUM_ELEMENTS, wrapper2_t((Vector4Test()))),  
                          Cw2(NUM_ELEMENTS, wrapper2_t((Vector4Test())));
void testw2(int NUM_TESTS) {
    for (int n = 0 ; n != NUM_TESTS; ++n)
        for (int i=0; i != NUM_ELEMENTS ; ++i) {
#ifdef USE_REFS            
            wrapper2_t& C = Cw2[i];
            const wrapper2_t& A = Aw2[i];
            const wrapper2_t& B = Bw2[i];
            C.SetX(A.GetX() + B.GetX());
            C.SetY(A.GetY() + B.GetY());
            C.SetZ(A.GetZ() + B.GetZ());
            C.SetW(A.GetW() + B.GetW());
#else
            Cw2[i].SetX(Aw2[i].GetX() + Bw2[i].GetX());
            Cw2[i].SetY(Aw2[i].GetY() + Bw2[i].GetY());
            Cw2[i].SetZ(Aw2[i].GetZ() + Bw2[i].GetZ());
            Cw2[i].SetW(Aw2[i].GetW() + Bw2[i].GetW());
#endif            
        }

}

std::vector< wrapper3_t > Aw3(NUM_ELEMENTS, wrapper3_t(Vector4Test())),
                          Bw3(NUM_ELEMENTS, wrapper3_t(Vector4Test())),  
                          Cw3(NUM_ELEMENTS, wrapper3_t(Vector4Test()));
void testw3(int NUM_TESTS) {
    for (int n = 0 ; n != NUM_TESTS; ++n)
        for (int i=0; i != NUM_ELEMENTS ; ++i) {
#ifdef USE_REFS            
            wrapper3_t& C = Cw3[i];
            const wrapper3_t& A = Aw3[i];
            const wrapper3_t& B = Bw3[i];
            C.SetX(A.GetX() + B.GetX());
            C.SetY(A.GetY() + B.GetY());
            C.SetZ(A.GetZ() + B.GetZ());
            C.SetW(A.GetW() + B.GetW());
#else
            Cw3[i].SetX(Aw3[i].GetX() + Bw3[i].GetX());
            Cw3[i].SetY(Aw3[i].GetY() + Bw3[i].GetY());
            Cw3[i].SetZ(Aw3[i].GetZ() + Bw3[i].GetZ());
            Cw3[i].SetW(Aw3[i].GetW() + Bw3[i].GetW());
#endif            
        }
}


std::vector< wrapper4_t > Aw4(NUM_ELEMENTS, wrapper4_t(Vector4Test())),
                          Bw4(NUM_ELEMENTS, wrapper4_t(Vector4Test())),  
                          Cw4(NUM_ELEMENTS, wrapper4_t(Vector4Test()));
void testw4(int NUM_TESTS) {
    for (int n = 0 ; n != NUM_TESTS; ++n)
        for (int i=0; i != NUM_ELEMENTS ; ++i) {
#ifdef USE_REFS            
            wrapper4_t& C = Cw4[i];
            const wrapper4_t& A = Aw4[i];
            const wrapper4_t& B = Bw4[i];
            C.SetX(A.GetX() + B.GetX());
            C.SetY(A.GetY() + B.GetY());
            C.SetZ(A.GetZ() + B.GetZ());
            C.SetW(A.GetW() + B.GetW());
#else
            Cw4[i].SetX(Aw4[i].GetX() + Bw4[i].GetX());
            Cw4[i].SetY(Aw4[i].GetY() + Bw4[i].GetY());
            Cw4[i].SetZ(Aw4[i].GetZ() + Bw4[i].GetZ());
            Cw4[i].SetW(Aw4[i].GetW() + Bw4[i].GetW());
#endif            
        }
}

std::vector< wrapper5_t > Aw5(NUM_ELEMENTS, wrapper5_t(Vector4Test())),
                          Bw5(NUM_ELEMENTS, wrapper5_t(Vector4Test())),  
                          Cw5(NUM_ELEMENTS, wrapper5_t(Vector4Test()));
void testw5(int NUM_TESTS) {
    for (int n = 0 ; n != NUM_TESTS; ++n)
        for (int i=0; i != NUM_ELEMENTS ; ++i) {
#ifdef USE_REFS            
            wrapper5_t& C = Cw5[i];
            const wrapper5_t& A = Aw5[i];
            const wrapper5_t& B = Bw5[i];
            C.SetX(A.GetX() + B.GetX());
            C.SetY(A.GetY() + B.GetY());
            C.SetZ(A.GetZ() + B.GetZ());
            C.SetW(A.GetW() + B.GetW());
#else
            Cw5[i].SetX(Aw5[i].GetX() + Bw5[i].GetX());
            Cw5[i].SetY(Aw5[i].GetY() + Bw5[i].GetY());
            Cw5[i].SetZ(Aw5[i].GetZ() + Bw5[i].GetZ());
            Cw5[i].SetW(Aw5[i].GetW() + Bw5[i].GetW());
#endif            
        }
}

std::vector< wrapper6_t > Aw6(NUM_ELEMENTS, wrapper6_t(Vector4Test())),
                          Bw6(NUM_ELEMENTS, wrapper6_t(Vector4Test())),  
                          Cw6(NUM_ELEMENTS, wrapper6_t(Vector4Test()));
void testw6(int NUM_TESTS) {
    for (int n = 0 ; n != NUM_TESTS; ++n)
        for (int i=0; i != NUM_ELEMENTS ; ++i) {
#ifdef USE_REFS            
            wrapper6_t& C = Cw6[i];
            const wrapper6_t& A = Aw6[i];
            const wrapper6_t& B = Bw6[i];
            C.SetX(A.GetX() + B.GetX());
            C.SetY(A.GetY() + B.GetY());
            C.SetZ(A.GetZ() + B.GetZ());
            C.SetW(A.GetW() + B.GetW());
#else
            Cw6[i].SetX(Aw6[i].GetX() + Bw6[i].GetX());
            Cw6[i].SetY(Aw6[i].GetY() + Bw6[i].GetY());
            Cw6[i].SetZ(Aw6[i].GetZ() + Bw6[i].GetZ());
            Cw6[i].SetW(Aw6[i].GetW() + Bw6[i].GetW());
#endif            
        }
}

std::vector< wrapper7_t > Aw7(NUM_ELEMENTS, wrapper7_t(Vector4Test())),
                          Bw7(NUM_ELEMENTS, wrapper7_t(Vector4Test())),  
                          Cw7(NUM_ELEMENTS, wrapper7_t(Vector4Test()));
void testw7(int NUM_TESTS) {
    for (int n = 0 ; n != NUM_TESTS; ++n)
        for (int i=0; i != NUM_ELEMENTS ; ++i) {
#ifdef USE_REFS            
            wrapper7_t& C = Cw7[i];
            const wrapper7_t& A = Aw7[i];
            const wrapper7_t& B = Bw7[i];
            C.SetX(A.GetX() + B.GetX());
            C.SetY(A.GetY() + B.GetY());
            C.SetZ(A.GetZ() + B.GetZ());
            C.SetW(A.GetW() + B.GetW());
#else
            Cw7[i].SetX(Aw7[i].GetX() + Bw7[i].GetX());
            Cw7[i].SetY(Aw7[i].GetY() + Bw7[i].GetY());
            Cw7[i].SetZ(Aw7[i].GetZ() + Bw7[i].GetZ());
            Cw7[i].SetW(Aw7[i].GetW() + Bw7[i].GetW());
#endif            
        }
}

std::vector< wrapper8_t > Aw8(NUM_ELEMENTS, wrapper8_t(Vector4Test())),
                          Bw8(NUM_ELEMENTS, wrapper8_t(Vector4Test())),  
                          Cw8(NUM_ELEMENTS, wrapper8_t(Vector4Test()));
void testw8(int NUM_TESTS) {
    for (int n = 0 ; n != NUM_TESTS; ++n)
        for (int i=0; i != NUM_ELEMENTS ; ++i) {
#ifdef USE_REFS            
            wrapper8_t& C = Cw8[i];
            const wrapper8_t& A = Aw8[i];
            const wrapper8_t& B = Bw8[i];
            C.SetX(A.GetX() + B.GetX());
            C.SetY(A.GetY() + B.GetY());
            C.SetZ(A.GetZ() + B.GetZ());
            C.SetW(A.GetW() + B.GetW());
#else
            Cw8[i].SetX(Aw8[i].GetX() + Bw8[i].GetX());
            Cw8[i].SetY(Aw8[i].GetY() + Bw8[i].GetY());
            Cw8[i].SetZ(Aw8[i].GetZ() + Bw8[i].GetZ());
            Cw8[i].SetW(Aw8[i].GetW() + Bw8[i].GetW());
#endif            
        }
}

//------------------------------------------------------------------------------
int main(int argc, char** argv) {
    
    for(int i = 0; i != NUM_ELEMENTS; ++i) {
        A[i] = shared_ptr< Vector4TestV >(new Vector4TestV);
        B[i] = shared_ptr< Vector4TestV >(new Vector4TestV);
        C[i] = shared_ptr< Vector4TestV >(new Vector4TestV);
    }
    const int numtests = argc == 1 ? 1 : stoi(argv[1]);
    const int calls_per_iteration = 12;
    const int calls = numtests * NUM_ELEMENTS * calls_per_iteration;
    using myclock_t = chrono::high_resolution_clock;
    using duration  = chrono::high_resolution_clock::duration;
    using timepoint = chrono::high_resolution_clock::time_point;

    vector< pair< string, duration > > data;

    cout << "\nSingle method execution time (ns)\n"
         << "-----------------------------------\n";
    
    timepoint t1 = myclock_t::now();
    test(numtests);
    timepoint t2 = myclock_t::now();
    duration d = t2 - t1;

#ifdef NOVIRTUAL    
    cout << "Reference (non-virtual):         "
#else
    cout << "Reference (virtual):             "
#endif     
         << float(chrono::nanoseconds(d).count()) / calls << endl;

    t1 = myclock_t::now();
    testw(numtests);
    t2 = myclock_t::now();
    d = t2 - t1;
    data.push_back(make_pair("std::function:                   ", d));     

    t1 = myclock_t::now();
    testw6(numtests);
    t2 = myclock_t::now();
    d = t2 - t1;
    data.push_back(make_pair("Alternative to std::function:    ", d));     

    t1 = myclock_t::now();
    testw4(numtests);
    t2 = myclock_t::now();
    d = t2 - t1;
    data.push_back(make_pair("My own std::function:            ", d));                       

    t1 = myclock_t::now();
    testw2(numtests);
    t2 = myclock_t::now();
    d = t2 - t1;
    data.push_back(make_pair("Function pointers:               ", d));     

    t1 = myclock_t::now();
    testw8(numtests);
    t2 = myclock_t::now();
    d = t2 - t1;
    data.push_back(make_pair("Function pointers in wrapper:    ", d));           

    t1 = myclock_t::now();
    testw3(numtests);
    t2 = myclock_t::now();
    d = t2 - t1;
    data.push_back(make_pair("Virtual:                         ", d));     

    t1 = myclock_t::now();
    testw5(numtests);
    t2 = myclock_t::now();
    d = t2 - t1;
    data.push_back(make_pair("Pointer to members:              ", d));    

    t1 = myclock_t::now();
    testw6(numtests);
    t2 = myclock_t::now();
    d = t2 - t1;
    data.push_back(make_pair("Callbacks:                       ", d));     
    
    using P = pair< string, duration >;
    sort(data.begin(), data.end(), [](const P& v1, const P& v2) {
        return v1.second < v2.second;
    });
    std::cout << std::endl;
    for(auto& i: data) {
        std::cout << i.first 
                  << float(chrono::nanoseconds(i.second).count()) / calls
                  << std::endl;
    }
    return 0;
}

// RESULTS
// -------

// Hardware:

// processor   : 0
// vendor_id   : GenuineIntel
// cpu family  : 6
// model       : 23
// model name  : Intel(R) Xeon(R) CPU           E5420  @ 2.50GHz
// stepping    : 6
// microcode   : 0x60b
// cpu MHz     : 2493.730
// cache size  : 6144 KB
// physical id : 0
// siblings    : 4
// core id     : 0
// cpu cores   : 4
// apicid      : 0
// initial apicid  : 0
// fpu     : yes
// fpu_exception   : yes
// cpuid level : 10
// wp      : yes
// flags       : fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov
//               pat pse36 clflush dts acpi mmx fxsr sse sse2 ss ht tm pbe 
//               syscall nx lm constant_tsc arch_perfmon pebs bts rep_good 
//               nopl aperfmperf pni dtes64 monitor ds_cpl vmx est tm2 ssse3 
//               cx16 xtpr pdcm dca sse4_1 lahf_lm dtherm tpr_shadow vnmi
//               flexpriority
// bogomips    : 4987.46
// clflush size    : 64
// cache_alignment : 64
// address sizes   : 38 bits physical, 48 bits virtual
// power management:

// OS: Ubuntu 13.04

//Compilers:
//CLang llvm 3.4 (2013-11-18 snapshot) with gold linker plugin 
//gcc 4.8.1 no gold linker

//Best: CLang llvm 3.4 (2013-11-18 snapshot) with gold linker and lto 
// clang++ -std=c++11 ../value-semantics-novirtual.cpp -flto -O3 -DNOVIRTUAL
// a.out 100
// Single method execution time (ns)
// -----------------------------------
// Reference (non-virtual):         0.843371
// std::function:                   2.95573
// Alternative to std::function:    2.93527
// My own std::function:            2.56618
// Function pointers:               0.694427 <<<
// Virtual:                         2.87633
// Pointer to members:              3.65078
// Callbacks:                       2.91835

// g++ -std=c++11 ../value-semantics-novirtual.cpp -O3
// a.out 100
// Single method execution time (ns)
// -----------------------------------
// Reference (virtual):             2.77228
// std::function:                   2.96681
// Alternative to std::function:    3.02521
// My own std::function:            2.72485
// Function pointers:               2.51679
// Virtual:                         2.75736
// Pointer to members:              3.43607
// Callbacks:                       3.03872

// g++ -std=c++11 ../value-semantics-novirtual.cpp -O3 -DNOVIRTUAL
// a.out 100
// Single method execution time (ns)
// -----------------------------------
// Reference (non-virtual):         0.985648
// std::function:                   2.95736
// Alternative to std::function:    3.0514
// My own std::function:            2.71151
// Function pointers:               2.50039
// Virtual:                         2.75616
// Pointer to members:              3.40965
// Callbacks:                       3.04172

// clang++ -std=c++11 ../value-semantics-novirtual.cpp -O3
// a.out 100 
// Single method execution time (ns)
// -----------------------------------
// Reference (virtual):             2.79378
// std::function:                   3.12673
// Alternative to std::function:    2.94879
// My own std::function:            2.65235
// Function pointers:               2.38956
// Virtual:                         2.75857
// Pointer to members:              3.64075
// Callbacks:                       2.93292

// clang++ -std=c++11 ../value-semantics-novirtual.cpp -O3 -flto
// a.out 100
// Single method execution time (ns)
// -----------------------------------
// Reference (virtual):             2.79266
// std::function:                   2.91576
// Alternative to std::function:    2.93675
// My own std::function:            2.59382
// Function pointers:               0.689505 
// Virtual:                         2.74981
// Pointer to members:              3.44271
// Callbacks:                       2.93133

// clang++ -std=c++11 ../value-semantics-novirtual.cpp  -O3 -DNOVIRTUAL
// a.out 100
// Single method execution time (ns)
// -----------------------------------
// Reference (non-virtual):         0.854446
// std::function:                   2.99174
// Alternative to std::function:    2.9939
// My own std::function:            2.6373
// Function pointers:               2.40005
// Virtual:                         2.76247
// Pointer to members:              3.50929
// Callbacks:                       2.96531

// clang++ -std=c++11 ../value-semantics-novirtual.cpp -flto -O3 -DNOVIRTUAL
// a.out 100
// Single method execution time (ns)
// -----------------------------------
// Reference (non-virtual):         0.843371
// std::function:                   2.95573
// Alternative to std::function:    2.93527
// My own std::function:            2.56618
// Function pointers:               0.694427 <<<
// Virtual:                         2.87633
// Pointer to members:              3.65078
// Callbacks:                       2.91835
