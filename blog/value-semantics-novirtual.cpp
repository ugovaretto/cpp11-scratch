//Author: Ugo Varetto
//Benchmark of various approaches to value semantincs with and without virtual
//functions
//TODO: check destroy policy: for non-virtual derived class the run-time invokes
//the base class destructor it is therefore required that the base destructor
//invokes the derived instance destructor
#include <cassert>
#include <functional>
#include <memory>
#include <chrono>
#include <ctime>
#include <string>
#include <vector>
#include <cstring>
#include <iostream>
#include "function.h"
#include "callback.hpp"

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
        template < typename T >
        base_t(const T& t) {
            const T* p = &static_cast< const model_t< T >* >(this)->d;
            T* pp = &static_cast< model_t< T >* >(this)->d;
            GetX = [p]() {
                //return static_cast< const model_t< T >& >(*this).d.GetX();
                return p->GetX();            
            };
            GetY = [p]() {
                //return static_cast< const model_t< T >& >(*this).d.GetY(); 
                return p->GetY();            
            };
            GetZ = [p]() {
                //return static_cast< const model_t< T >& >(*this).d.GetZ(); 
                return p->GetZ();            
            };
            GetW = [p]() {
                //return static_cast< const model_t< T >& >(*this).d.GetW(); 
                return p->GetW();            
            };
            SetX = [pp](float x_) {
                //return static_cast< model_t< T >& >(*this).d.SetX(x_);   
                return pp->SetX(x_);         
            };
            SetY = [pp](float y_) {
                //return static_cast< model_t< T >& >(*this).d.SetY(y_);   
                return pp->SetY(y_);           
            };
            SetZ = [pp](float z_) {
                //return static_cast< model_t< T >& >(*this).d.SetZ(z_);
                return pp->SetZ(z_);              
            };
            SetW = [pp](float w_) {
                //return static_cast< model_t< T >& >(*this).d.SetW(w_);  
                return pp->SetW(w_);            
            };
            Copy = [this]() {
                return static_cast< const model_t< T >& >(*this).Copy();
            };
            Destroy = [this]() {
                return static_cast< model_t< T >& >(*this).d.T::~T();
            };                    
        }
        std::function< float () > GetX;
        std::function< float () > GetY;
        std::function< float () > GetZ;
        std::function< float () > GetW;
        std::function< float (float) > SetX;
        std::function< float (float) > SetY;
        std::function< float (float) > SetZ;
        std::function< float (float) > SetW;
        std::function< base_t* () > Copy;
        std::function< void () > Destroy;
    };
    template < typename T >
    struct model_t : base_t {
        T d;
        model_t(const T& t) : base_t(t), d(t) {}    
        base_t* Copy() const { return new model_t(*this); }
    };
    unique_ptr< base_t > model_;
};


//------------------------------------------------------------------------------
//static function pointers
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
        
        static CP CopyImpl;
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
GF wrapper2_t::base_t::vtable::GetXImpl = nullptr;
GF wrapper2_t::base_t::vtable::GetYImpl = nullptr;
GF wrapper2_t::base_t::vtable::GetZImpl = nullptr;
GF wrapper2_t::base_t::vtable::GetWImpl = nullptr;
SF wrapper2_t::base_t::vtable::SetXImpl = nullptr;
SF wrapper2_t::base_t::vtable::SetYImpl = nullptr;
SF wrapper2_t::base_t::vtable::SetZImpl = nullptr;
SF wrapper2_t::base_t::vtable::SetWImpl = nullptr;
CP wrapper2_t::base_t::CopyImpl = nullptr;

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
//alternate std::function implementation
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
        template < typename T >
        base_t(const T& t) {
            const T* p = &static_cast< const model_t< T >* >(this)->d;
            T* pp = &static_cast< model_t< T >* >(this)->d;
            GetX = [p]() {
                //return static_cast< const model_t< T >& >(*this).d.GetX();
                return p->GetX();            
            };
            GetY = [p]() {
                //return static_cast< const model_t< T >& >(*this).d.GetY(); 
                return p->GetY();            
            };
            GetZ = [p]() {
                //return static_cast< const model_t< T >& >(*this).d.GetZ(); 
                return p->GetZ();            
            };
            GetW = [p]() {
                //return static_cast< const model_t< T >& >(*this).d.GetW(); 
                return p->GetW();            
            };
            SetX = [pp](float x_) {
                //return static_cast< model_t< T >& >(*this).d.SetX(x_);   
                return pp->SetX(x_);         
            };
            SetY = [pp](float y_) {
                //return static_cast< model_t< T >& >(*this).d.SetY(y_);   
                return pp->SetY(y_);           
            };
            SetZ = [pp](float z_) {
                //return static_cast< model_t< T >& >(*this).d.SetZ(z_);
                return pp->SetZ(z_);              
            };
            SetW = [pp](float w_) {
                //return static_cast< model_t< T >& >(*this).d.SetW(w_);  
                return pp->SetW(w_);            
            };
            Copy = [this]() {
                return static_cast< const model_t< T >& >(*this).Copy();
            };
            Destroy = [this]() {
                static_cast< model_t< T >& >(*this).d.T::~T();
            };                    
        }
        func::function< float () > GetX;
        func::function< float () > GetY;
        func::function< float () > GetZ;
        func::function< float () > GetW;
        func::function< float (float) > SetX;
        func::function< float (float) > SetY;
        func::function< float (float) > SetZ;
        func::function< float (float) > SetW;
        func::function< base_t* () > Copy;
        func::function< void () > Destroy;
    };
    template < typename T >
    struct model_t : base_t {
        T d;
        model_t(const T& t) : base_t(t), d(t) {}    
        base_t* Copy() const { return new model_t(*this); }
    };
    unique_ptr< base_t > model_;
};

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
        GF GetXImpl;
        GF GetYImpl;
        GF GetZImpl;
        GF GetWImpl;
        SF SetXImpl;
        SF SetYImpl;
        SF SetZImpl;
        SF SetWImpl;
        CP CopyImpl;
        ~base_t() {}
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

    };
    unique_ptr< base_t > model_;
};

//------------------------------------------------------------------------------
//pointer to members initialized in derived class
//------------------------------------------------------------------------------
#define ensure_forceinline __attribute__((always_inline)) // inline or die
template < typename R, typename P >
struct Callback {
    typedef R (*FuncType)(void*, P);
    Callback() : func(0), obj(0) {}
    Callback(const Callback&) = default;
    Callback(const FuncType f, void* __restrict__ o) : func(f), obj(o) {}
    R operator()(P p) { return func(obj, p); }
    void operator=(const Callback& c) const {
        FuncType& r = const_cast< FuncType& >(func);
        r = c.func;
        obj = c.obj; 
    }
    const FuncType func;
    mutable void* __restrict__ obj;
};

template < typename R, typename P >
struct CCallback {
    typedef R (*FuncType)(const void*, P);
    CCallback() : func(0), obj(0) {}
    CCallback(const CCallback&) = default;
    CCallback(const FuncType f, const void* __restrict__ o) : func(f), obj(o) {}
    R operator()(P p) { return func(obj, p); }
     void operator=(const CCallback& c) const {
        FuncType& r = const_cast< FuncType& >(func);
        r = c.func;
        obj = c.obj;  
    }
    const FuncType func;
    mutable const void* __restrict__ obj;
};

template < typename R >
struct Callback<R, void> {
    typedef R (*FuncType)(const void*);
    Callback(const FuncType f, const void* __restrict__ o) : func(f), obj(o) {}
    Callback(const Callback&) = default;
    R operator()() { return func(obj); }
    Callback() : func(0), obj(0) {}
    const FuncType func;
    mutable const void* __restrict__ obj;
     void operator=(const Callback& c) const {
        FuncType& r = const_cast< FuncType& >(func);
        r = c.func;
        obj = c.obj;  
    }
};


template < typename R, typename T, typename P, R (T::*Func)(P) >

    R Wrapper(void* __restrict__ obj, P p) {
        T* pp = reinterpret_cast< T* >(obj);
        return (pp->*Func)(p);
    } 



template < typename R, typename T, typename P, R (T::*Func)(P) const >

    static R WrapperC(const void* __restrict__ obj, P p) {
        const T* pp = reinterpret_cast< const T* >(obj);
        return (pp->*Func)(p);
    } 
    


template < typename R, typename T, R (T::*Func)() const >

    R WrapperV(const void* __restrict__ obj) {
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
   
    using CP = CCallback< base_t*, wrapper6_t* >;      
        CP Copy;
        func::function< void () > Destroy;
        ~base_t() {Destroy();}
    };
    template < typename T >
    struct model_t : base_t {
        using GF = Callback< float, void >;
        using SF = Callback< float, float >;
        T d;
        model_t(const T& t, wrapper6_t* w) : d(t) {
            Copy = CP(&WrapperC< base_t*, model_t< T >, wrapper6_t*, &model_t< T >::CopyImpl >, this);
            build_table(w);
        }
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
        base_t* CopyImpl(wrapper6_t* w) const { 
            model_t< T >* p = new model_t< T >(*this);
            p->build_table(w);
            return p;
        }
          
    };
    unique_ptr< base_t > model_;
};



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

#define NUM_ELEMENTS 122222

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
//------------------------------------------------------------------------------


class Foo {
public:
    Foo() {}
    float ConstMemberFunction()  { return 1.0f; }
    float MemberFunction(float x) { return x; }
};

// void test_Callback() {
//     Foo f;
//     Callback< float, float > c1(
//         &Wrapper< float, Foo, float, &Foo::MemberFunction>::Wrap, &f);
    
//     Callback< float, void > c2
//         (&WrapperV< float, Foo, &Foo::ConstMemberFunction>::Wrap, &f);
//     std::cout << c1(2.0f) << ' ' << c2() << endl;
//     cout << c1(3) << endl;
// }

//------------------------------------------------------------------------------
int main(int argc, char** argv) {
    //test_Callback();
    for(int i = 0; i != NUM_ELEMENTS; ++i) {
        A[i] = shared_ptr< Vector4TestV >(new Vector4TestV);
        B[i] = shared_ptr< Vector4TestV >(new Vector4TestV);
        C[i] = shared_ptr< Vector4TestV >(new Vector4TestV);
    }
    int numtests = argc == 1 ? 1 : stoi(argv[1]);
    const int calls = numtests * NUM_ELEMENTS; 
    using myclock_t = chrono::high_resolution_clock;
    using duration  = chrono::high_resolution_clock::duration;
    using timepoint = chrono::high_resolution_clock::time_point;

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
         << (chrono::nanoseconds(d).count()) / calls << endl;

    t1 = myclock_t::now();
    testw(numtests);
    t2 = myclock_t::now();
    d = t2 - t1;
    cout << "std::function:                   "
         << (chrono::nanoseconds(d).count()) / calls << endl;

    t1 = myclock_t::now();
    testw2(numtests);
    t2 = myclock_t::now();
    d = t2 - t1;
    cout << "Function pointers:               "
         << (chrono::nanoseconds(d).count()) / calls << endl;

    t1 = myclock_t::now();
    testw3(numtests);
    t2 = myclock_t::now();
    d = t2 - t1;
    cout << "Virtual:                         "
         << (chrono::nanoseconds(d).count()) / calls << endl;

    t1 = myclock_t::now();
    testw4(numtests);
    t2 = myclock_t::now();
    d = t2 - t1;
    cout << "Alternative to std::function:    "
         << (chrono::nanoseconds(d).count()) / calls << endl;

    t1 = myclock_t::now();
    testw5(numtests);
    t2 = myclock_t::now();
    d = t2 - t1;
    cout << "Pointer to members:              "
         << (chrono::nanoseconds(d).count()) / calls << endl;

    t1 = myclock_t::now();
    testw6(numtests);
    t2 = myclock_t::now();
    d = t2 - t1;
    cout << "XXXXXXXXXXXXXXXXXXX:              "
         << (chrono::nanoseconds(d).count()) / calls << endl;     

    return 0;
}
