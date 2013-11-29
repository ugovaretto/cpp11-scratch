//Author: Ugo Varetto
//Benchmark of various approaches to value semantics with and without virtual
//functions
//NOTE on destruction:
//for non-virtual derived class the run-time invokes the base class destructor; 
//it is therefore required that the base destructor invoke the derived 
//instance destructor or the wrapped-data destructor directly.

//SUMMARY:
//any approach other than std::function or func::function including a base_t 
//with virtual methods is fine
//static vtables are much faster than anything else when lto enabled
//and outperform even regular non-virtual method invocations
//if instead of static function pointers, non-static pointers are used the
//performance is equal to virtual methods under -O3 optimizations
//when using xxx::function it is not worth trying with closures: do use regular
//functions which accept as the first parameter the model_t<> pointer to
//operate on, using a closure on this or model_t.d results in >40% performance
//penalty

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

#include <cassert>
#include <functional>
#include <memory>
#include <chrono>
#include <ctime>
#include <string>
#include <vector>
#include <cstring>
#include <iostream>
#include "function.h"   //Malte Skarupke's std::function
#include "simple-fun.h" //my own quick implementation of std::function always
                        //faster than std and Malte's when using static
                        //members, when not using static members Malte's version
                        //is always the fastest
                        

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
    GF GetX;
    GF GetY;
    GF GetZ;
    GF GetW;
    SF SetX;
    SF SetY;
    SF SetZ;
    SF SetW;
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
    wrapper8_t(const wrapper8_t& w)  
    //     GetXImpl(w.GetXImpl),
    //     GetYImpl(w.GetYImpl),
    //     GetZImpl(w.GetZImpl),
    //     GetWImpl(w.GetWImpl),
    //     SetXImpl(w.SetXImpl),
    //     SetYImpl(w.SetYImpl),
    //     SetZImpl(w.SetZImpl),
    //     SetWImpl(w.SetWImpl),
    //     Destroy(w.Destroy),
        /*Copy(w.Copy)*/ {
            //model_.resize(sizeof(w.model_.size()));
            w.Copy(&w.model_[0], &model_[0]);   
        } 
    template < typename T >
    wrapper8_t(const T& t) {
        //model_.resize(sizeof(t));
        new (&model_[0]) T(t);
        BuildVTable< T >();
    }
    template < typename T >
    T& get() {
        return *reinterpret_cast< T* >(&model_[0]);
    }
    std::vector< char > model_; //using stack based allocation is ~20% faster
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
static const int NUM_ELEMENTS = 1024;//122222;

//------------------------------------------------------------------------------
std::vector< shared_ptr< Vector4TestV > > A(NUM_ELEMENTS),
                                          B(NUM_ELEMENTS),
                                          C(NUM_ELEMENTS);
void test(int NUM_TESTS) {
    for (int n = 0 ; n != NUM_TESTS; ++n)
        for (int i=0; i != NUM_ELEMENTS ; ++i) {
            C[i]->SetX(A[i]->GetX() + B[i]->GetX());
            C[i]->SetY(A[i]->GetY() + B[i]->GetY());
            C[i]->SetZ(A[i]->GetZ() + B[i]->GetZ());
            C[i]->SetW(A[i]->GetW() + B[i]->GetW());
        }
}

std::vector< wrapper_t > Aw(NUM_ELEMENTS, wrapper_t(Vector4Test())),
                         Bw(NUM_ELEMENTS, wrapper_t(Vector4Test())),  
                         Cw(NUM_ELEMENTS, wrapper_t(Vector4Test()));
void testw(int NUM_TESTS) {
    for (int n = 0 ; n != NUM_TESTS; ++n)
        for (int i=0; i != NUM_ELEMENTS ; ++i) {
            Cw[i].SetX(Aw[i].GetX() + Bw[i].GetX());
            Cw[i].SetY(Aw[i].GetY() + Bw[i].GetY());
            Cw[i].SetZ(Aw[i].GetZ() + Bw[i].GetZ());
            Cw[i].SetW(Aw[i].GetW() + Bw[i].GetW());
        }
}

std::vector< wrapper2_t > Aw2(NUM_ELEMENTS, wrapper2_t((Vector4Test()))),
                          Bw2(NUM_ELEMENTS, wrapper2_t((Vector4Test()))),  
                          Cw2(NUM_ELEMENTS, wrapper2_t((Vector4Test())));
void testw2(int NUM_TESTS) {
    for (int n = 0 ; n != NUM_TESTS; ++n)
        for (int i=0; i != NUM_ELEMENTS ; ++i) {
            Cw2[i].SetX(Aw2[i].GetX() + Bw2[i].GetX());
            Cw2[i].SetY(Aw2[i].GetY() + Bw2[i].GetY());
            Cw2[i].SetZ(Aw2[i].GetZ() + Bw2[i].GetZ());
            Cw2[i].SetW(Aw2[i].GetW() + Bw2[i].GetW());
        }
}

std::vector< wrapper3_t > Aw3(NUM_ELEMENTS, wrapper3_t(Vector4Test())),
                          Bw3(NUM_ELEMENTS, wrapper3_t(Vector4Test())),  
                          Cw3(NUM_ELEMENTS, wrapper3_t(Vector4Test()));
void testw3(int NUM_TESTS) {
    for (int n = 0 ; n != NUM_TESTS; ++n)
        for (int i=0; i != NUM_ELEMENTS ; ++i) {
            Cw3[i].SetX(Aw3[i].GetX() + Bw3[i].GetX());
            Cw3[i].SetY(Aw3[i].GetY() + Bw3[i].GetY());
            Cw3[i].SetZ(Aw3[i].GetZ() + Bw3[i].GetZ());
            Cw3[i].SetW(Aw3[i].GetW() + Bw3[i].GetW());
        }
}


std::vector< wrapper4_t > Aw4(NUM_ELEMENTS, wrapper4_t(Vector4Test())),
                          Bw4(NUM_ELEMENTS, wrapper4_t(Vector4Test())),  
                          Cw4(NUM_ELEMENTS, wrapper4_t(Vector4Test()));
void testw4(int NUM_TESTS) {
    for (int n = 0 ; n != NUM_TESTS; ++n)
        for (int i=0; i != NUM_ELEMENTS ; ++i) {
            Cw4[i].SetX(Aw4[i].GetX() + Bw4[i].GetX());
            Cw4[i].SetY(Aw4[i].GetY() + Bw4[i].GetY());
            Cw4[i].SetZ(Aw4[i].GetZ() + Bw4[i].GetZ());
            Cw4[i].SetW(Aw4[i].GetW() + Bw4[i].GetW());
        }
}

std::vector< wrapper5_t > Aw5(NUM_ELEMENTS, wrapper5_t(Vector4Test())),
                          Bw5(NUM_ELEMENTS, wrapper5_t(Vector4Test())),  
                          Cw5(NUM_ELEMENTS, wrapper5_t(Vector4Test()));
void testw5(int NUM_TESTS) {
    for (int n = 0 ; n != NUM_TESTS; ++n)
        for (int i=0; i != NUM_ELEMENTS ; ++i) {
            Cw5[i].SetX(Aw5[i].GetX() + Bw5[i].GetX());
            Cw5[i].SetY(Aw5[i].GetY() + Bw5[i].GetY());
            Cw5[i].SetZ(Aw5[i].GetZ() + Bw5[i].GetZ());
            Cw5[i].SetW(Aw5[i].GetW() + Bw5[i].GetW());
        }
}

std::vector< wrapper6_t > Aw6(NUM_ELEMENTS, wrapper6_t(Vector4Test())),
                          Bw6(NUM_ELEMENTS, wrapper6_t(Vector4Test())),  
                          Cw6(NUM_ELEMENTS, wrapper6_t(Vector4Test()));
void testw6(int NUM_TESTS) {
    for (int n = 0 ; n != NUM_TESTS; ++n)
        for (int i=0; i != NUM_ELEMENTS ; ++i) {
            Cw6[i].SetX(Aw6[i].GetX() + Bw6[i].GetX());
            Cw6[i].SetY(Aw6[i].GetY() + Bw6[i].GetY());
            Cw6[i].SetZ(Aw6[i].GetZ() + Bw6[i].GetZ());
            Cw6[i].SetW(Aw6[i].GetW() + Bw6[i].GetW());
        }
}

std::vector< wrapper7_t > Aw7(NUM_ELEMENTS, wrapper7_t(Vector4Test())),
                          Bw7(NUM_ELEMENTS, wrapper7_t(Vector4Test())),  
                          Cw7(NUM_ELEMENTS, wrapper7_t(Vector4Test()));
void testw7(int NUM_TESTS) {
    for (int n = 0 ; n != NUM_TESTS; ++n)
        for (int i=0; i != NUM_ELEMENTS ; ++i) {
            Cw7[i].SetX(Aw7[i].GetX() + Bw7[i].GetX());
            Cw7[i].SetY(Aw7[i].GetY() + Bw7[i].GetY());
            Cw7[i].SetZ(Aw7[i].GetZ() + Bw7[i].GetZ());
            Cw7[i].SetW(Aw7[i].GetW() + Bw7[i].GetW());
        }
}

std::vector< wrapper8_t > Aw8(NUM_ELEMENTS, wrapper8_t(Vector4Test())),
                          Bw8(NUM_ELEMENTS, wrapper8_t(Vector4Test())),  
                          Cw8(NUM_ELEMENTS, wrapper8_t(Vector4Test()));
void testw8(int NUM_TESTS) {
    for (int n = 0 ; n != NUM_TESTS; ++n)
        for (int i=0; i != NUM_ELEMENTS ; ++i) {
            Cw8[i].SetX(Aw8[i].GetX() + Bw8[i].GetX());
            Cw8[i].SetY(Aw8[i].GetY() + Bw8[i].GetY());
            Cw8[i].SetZ(Aw8[i].GetZ() + Bw8[i].GetZ());
            Cw8[i].SetW(Aw8[i].GetW() + Bw8[i].GetW());
        }
}

//==============================================================================


// struct B1 {
//     void Foo() const {
//         std::cout << "B1::Foo" << std::endl;
//     }
// };

// struct B2 {
//     void Foo() const {
//         std::cout << "B2::Foo" << std::endl;
//     }
// };

// typedef const type_info* TypeId;
// template < typename T, typename...Args >
// int MapLocations(unordered_map< TypeId, int >& m,
//                  int offset) {
//     m[&typeid(T)] = m.size() ? offset : 0;
//     offset += sizeof(T);
//     if(sizeof...(Args) > 0) return MapLocations< Args... >(m, offset);
//     else return offset;
// }

// template < typename 


// struct Wrapper {
//     template < typename Args... >
//     Wrapper(const Args&...args) {
//         new (&buf_[0]) tuple< Args >(args...);
//         MapLocations< Args... >(typeLocationMap_, 0);
//     };
//     template < typename T >
//     const T& Get() const {
//         *static_cast< const T* >(&buf_[offset[&typeid(T)]]    
//     }
//     unordered_map< TypeId, int > 
// };




template < typename T, typename...Args >
struct TotalSizeof {
    enum { value = sizeof(T) + TotalSize(Args...) };
};

template <>
struct TotalSizeof<> {
    enum { value = 0 };  
};

template < typename T, typename...Args >
void InitBuf(void* p, T&& arg, Args&&...args) {
    new (p) T(arg);
    if(sizeof...(args) > 0) InitBuf(p + sizeof(T), args);
}


template < typename....Args >
void f(Args&&...args) {
    buf_.resize(TotalSize< Args... >::value);
    const void* t[] = {&typeid(Args)...}; 
    const void* sizes = {sizeof(Args)...};
    const ptrdiff_t offsets[sizeof...(Args)];
    offsets[0] = 0;
    for(int i = 1; i != sizeof...(Args); ++i) {
        offsets[i] = offsets[i - 1] + sizes[i];
    }
    InitBuf(&buf_[0], args);
    offsets_ = std::vector< const ptrdiff_t >(offsets, offsets + sizeof...(Args));
    types_   = std::vector< const type_info* >(t, t + sizeof...(Args));
}
 
get<T>() {
    return W
} 
template < typename T,> 
struct W {
    enum { id = -1 };
};

template < typename V >
struct Wrapper {
    template < typename VtableGenT, typename....Args >
    void Wrapper(VtabeGenT&& vf, Args&&...args) {
        buf_.resize(TotalSize< Args... >::value);
        const void* t[] = {&typeid(Args)...}; 
        const void* sizes = {sizeof(Args)...};
        const ptrdiff_t offsets[sizeof...(Args)];
        offsets[0] = 0;
        for(int i = 1; i != sizeof...(Args); ++i) {
            offsets[i] = offsets[i - 1] + sizes[i];
        }
        InitBuf(&buf_[0], args);
        offsets_ = std::vector< const ptrdiff_t >(offsets, offsets + sizeof...(Args));
        types_   = std::vector< const type_info* >(t, t + sizeof...(Args));
        vf(vtable_);
    }
    template < typename T >
    const T& Get() const {
        for(std::vector< ptrdiff_t >::const_iterator i = types_.begin();
            i != types_.end();
            ++i)
            if(*i == &typeid(T))
                return *static_cast< const T* >(FindInstance< T >());
        throw std::logic_error("IVALID TYPE"); 
    }
    template < typename T >
    const void* FindInstance() const {
        return &buf_[0] + type_map_[&typeid()];
    }
    V vtable_;
    std::vector< char > buf_;
    std::vector< ptrdiff_t > offsets_;
    std::vector< const std::type_info* > types_;   
};


template < typename T, int Pos, int Cnt, typename HeadT, typename...TailT >
struct OffsetOf{
    enum {value = sizeof(HeadT) + OffsetOf< T, Pos, Cnt, TailT... >::value};
};

template < typename T, int Pos, int Cnt, typename...TailT >
struct OffsetOf{
    enum {value = sizeof(T) + OffsetOf< T, Pos, Cnt + 1, TailT... >::value};
};


template < typename T, int Pos, typename...Tail >
struct OffsetOf< T, Pos, Pos, T, Tail... > {
    enum {value = 0};
};


template < typename T, typename...Args >
void Copy(const void* src, void* target) {
    new (target) T(*static_cast< const T* >(src));
    if(sizeof...(Args) > 0) Copy(src + sizeof(T), target + sizeof(T));
}

template < typename T, typename...Args >
void Move(void* src, void* target) {
    new (target) T(std::move(*static_cast< T* >(src)));
    if(sizeof...(Args) > 0) Move(src + sizeof(T), target + sizeof(T));
}

template < typename T, typename...Args >
void Swap(void* v1, void* v2) {
    swap(*static_cast< T* >(v1), *static_cast< T* >(v2));
    if(sizeof...(Args) > 0) Swap(src + sizeof(T), target + sizeof(T));
}

template < typename T, typename...Args >
void Destroy(const void* v1) {
    static_cast< const T* >(v1)->T::~T();
    if(sizeof...(Args) > 0) Destroy(v1 + sizeof(T));
}


template < typename T, int N, int Cnt, typename...Args >
struct Find {
    enum {value = Find< T, N, Cnt, Args... >::value};
};

template < typename T, int N, int Cnt, typename T, typename...Args >
struct Find< T, N, Cnt, Args... > {
    enum { value = Find< T, N, Cnt + 1, Args... >::value };
};

template < typename T, int N, typename T, typename...Args >
struct Find< T, N, N, T, Args... > {
    enum { value = 1 };
};

template < typename T, int N, int Cnt >
struct Find< T, N, Cnt > {
    enum { value = 0; }
};

//OR
// template < typename T, typename...Args >
// struct Destroy {
//     static void Apply(const void* p) {
//         static_cast< const T* >(v1)->T::~T();
//         Destroy::Apply< Args... >(p + sizeof(T));    
//     }
// };

// template <>
// struct Destroy {
//     static void Apply(const void*);
// };



template < typename...Args >
struct VT {
    using Base = VT< Args... >;
    using types = std::tuple< Args... >;
    template < typename T >
    static const T& Get(const void* p) const {
        return *static_cast< const T* >
            (p + OffsetOf< T, 0, 0, Args... >::value);
    }
    template < typename T, int count >
    static const T& GetInstance(const void* p) const {
        static_assert(Find< T, count, 0, Args... >::value,
                      "Type not found");
        return *static_cast< const T* >
            (p + OffsetOf< T, count, 0, Args... >::value);
    }
    static void Copy(const void* src, void* target) {


    }
    static void Move(const void* src, void* target) {

    }
    static void Swap(void* src, void* target) {

    }
    static void Destroy(void* p) {

    }
};

struct AVtable : VT< std::string, std::vector< int > > {
    size_t size(const void* p) const {
        return Base::GetInstance< std::string, 0 >(p).size();
    }

};






//Vtable has a type map!
template < typename V >
struct Wrapper {
    template < typename V2 >
    Wrapper(const Wrapper< V2 >& V) {
        static_assert(std::is_same(V2::types, V1::types)::value); 
    }
    template < typename V, typename....Args >
    void Wrapper(V&& vf, Args&&...args) {
        buf_.resize(TotalSize< Args... >::value);
        const void* t[] = {&typeid(Args)...}; 
        const void* sizes = {sizeof(Args)...};
        const ptrdiff_t offsets[sizeof...(Args)];
        offsets[0] = 0;
        for(int i = 1; i != sizeof...(Args); ++i) {
            offsets[i] = offsets[i - 1] + sizes[i];
        }
        InitBuf(&buf_[0], args);
        offsets_ = std::vector< const ptrdiff_t >(offsets, offsets + sizeof...(Args));
        types_   = std::vector< const type_info* >(t, t + sizeof...(Args));
        vf(vtable_);
    }
    template < typename T >
    const T& Get() const {
        return vtable_.Get< T >(&buf_[0]);
    }
    template < typename T >
    const void* FindInstance() const {
        return &buf_[0] + type_map_[&typeid()];
    }
    V vtable_;
    std::vector< char > buf_;
    std::vector< ptrdiff_t > offsets_;
    std::vector< const std::type_info* > types_;   
};


//------------------------------------------------------------------------------
int main(int argc, char** argv) {

  


    for(int i = 0; i != NUM_ELEMENTS; ++i) {
        A[i] = shared_ptr< Vector4TestV >(new Vector4TestV);
        B[i] = shared_ptr< Vector4TestV >(new Vector4TestV);
        C[i] = shared_ptr< Vector4TestV >(new Vector4TestV);
    }
    int numtests = argc == 1 ? 1 : stoi(argv[1]);
    const int calls_per_iteration = 12;
    const int calls = numtests * NUM_ELEMENTS * calls_per_iteration;
    using myclock_t = chrono::high_resolution_clock;
    using duration  = chrono::high_resolution_clock::duration;
    using timepoint = chrono::high_resolution_clock::time_point;


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
