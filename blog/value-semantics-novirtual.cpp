#include <iostream>
#include <cassert>
#include <functional>
#include <memory>
#include <chrono>
#include <ctime>
#include <string>
#include <vector>
#include "function.h"

using namespace std;
#if 0
#if novirtual
struct wrapper_t {
    //std::function< void () > foo;
    void foo() {model_->foo();}
    template < typename T >
    wrapper_t(const T& t) : model_(new model_t< T >(t)) {
        //can point directly to the model_t.d data member but then
        //at each copy/assignment functions have to be recreated
        
    }
    template < typename T >
    T& get() {
        return static_cast< model_t< T >& >(*model_).d;
    }
    struct base_t {
        template < typename T >
        base_t(const T& t) {
            foo = [=]() {
                static_cast< model_t< T >& >(*this).d.foo();            
            };  
        }
        std::function< void () > foo;
    };
    template < typename T >
    struct model_t : base_t {
        T d;
        model_t(const T& t) : base_t(t), d(t) {}    
    };
    unique_ptr< base_t > model_;
};
#else



struct wrapper_t {
    template < typename T >
    wrapper_t(const T& t) : model_(new model_t< T >(t)) {
    }
    void foo() { model_->foo();}
    struct base_t {
        virtual void foo() = 0;
        virtual ~base_t() {}
    };
    template < typename T >
    T& get() {
        return static_cast< model_t< T >& >(*model_).d;
    }
    template < typename T >
    struct model_t : base_t {
        T d;
        void foo() { d.foo(); }
        model_t(const T& t) : d(t) {}    
    };
    unique_ptr< base_t > model_;
};
#endif

inline double diff(const timespec& start,
                   const timespec& stop) {
    return stop.tv_nsec - start.tv_nsec;
} 
struct atype_t {
    int i;
    void foo() {
        i = 2 + 4;
        i *= i;
    }
};

int main(int, char**) {
    timespec start, stop;
    using myclock_t = chrono::high_resolution_clock;
    timespec time1, time2;
     atype_t at;
     wrapper_t w((atype_t()));
    for(int i = 0; i != 1000; ++i) {

    clock_gettime(CLOCK_MONOTONIC, &start); 
    // t1 = myclock_t::now();
    w.foo();
    // t2 = myclock_t::now();
    // d = t2 - t1;
    // cout << chrono::nanoseconds(d).count() << endl;
    // cout << endl;
    clock_gettime(CLOCK_MONOTONIC, &stop); 
    cout << diff(start, stop) << endl;
    cout << endl;

    clock_gettime(CLOCK_MONOTONIC, &start); 
    //auto t1 = myclock_t::now();
    at.foo();
    //auto t2 = myclock_t::now();
    //auto d = t2 - t1;
    clock_gettime(CLOCK_MONOTONIC, &stop);
    //cout << chrono::nanoseconds(d).count() << endl;
    cout << diff(start, stop) << endl;

    clock_gettime(CLOCK_MONOTONIC, &start); 
    // //t1 = myclock_t::now();
    w.get<atype_t>().foo();
    // //t.foo();
    // //t2 = myclock_t::now();
    // d = t2 - t1;
    // cout << chrono::nanoseconds(d).count() << endl;
    clock_gettime(CLOCK_MONOTONIC, &stop);
    //cout << chrono::nanoseconds(d).count() << endl;
    cout << diff(start, stop) << endl;
   

    }
    return 0;
}
#else

#ifdef NOVIRTUAL
#define VIR
#else
#define VIR virtual
#endif

//------------------------------------------------------------------------------
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
    wrapper2_t(wrapper2_t&& w) {
        if(w.storage_.empty()) return;
        storage_ = std::move(w.storage_);
        model_ = (base_t*) &storage_[0];
    }

    wrapper2_t(const wrapper2_t& w) {
        if(w.storage_.empty()) return;
        storage_.resize(w.storage_.size());
        w.model_->Copy(&storage_[0]);
        model_ = (base_t*) &storage_[0];
    }
    template < typename T >
    wrapper2_t(const T& t)  {
        storage_.resize(sizeof(model_t<T>));
        new (&storage_[0]) model_t< T >(t);
        model_ = (base_t*) &storage_[0];
    }

    wrapper2_t& operator=(wrapper2_t w) {
        storage_ = move(w.storage_);
        if(!storage_.empty())
            model_ = (base_t*) &storage_[0];
        return *this;
    }
    
    template < typename T >
    T& get() {
        return static_cast< model_t< T >& >(*model_).d;
    }
    struct base_t {
        using GF = float (*)(const void* );
        using SF = float (*)(void*, float);
        template < typename T >
        base_t(const T& t) {
            const T* p = &static_cast< const model_t< T >* >(this)->d;
            T* pp = &static_cast< model_t< T >* >(this)->d;
            GetXImpl = (GF)([](const void* self) {
                return reinterpret_cast< const T* >(self)->GetX();
                           
            });
            GetYImpl = (GF)([](const void* self) {
                return reinterpret_cast< const T* >(self)->GetY();
                           
            });
            GetZImpl = (GF)([](const void* self) {
                return reinterpret_cast< const T* >(self)->GetZ();
                           
            });
            GetWImpl = (GF)([](const void* self) {
                return reinterpret_cast< const T* >(self)->GetW();
                           
            });
            SetXImpl = (SF)([](void* self, float x_) {
                return reinterpret_cast< T* >(self)->SetX(x_);
            });
            SetYImpl = (SF)([](void* self, float y_) {
                return reinterpret_cast< T* >(self)->SetY(y_);
            });
            SetZImpl = (SF)([](void* self, float z_) {
                return reinterpret_cast< T* >(self)->SetZ(z_);
            });
            SetWImpl = (SF)([](void* self, float w_) {
                return reinterpret_cast< T* >(self)->SetW(w_);
            });
            Copy = [this](void* p) {
                static_cast< const model_t< T >& >(*this).Copy(p);
            };
            Destroy = [this]() {
                return static_cast< model_t< T >& >(*this).d.T::~T();
            };                    
        }
        inline float GetX() const { return GetXImpl(self); }
        inline float GetY() const { return GetYImpl(self); }
        inline float GetZ() const { return GetZImpl(self); }
        inline float GetW() const { return GetWImpl(self); }
        inline float SetX( float x_ ) { return SetXImpl(self, x_); }
        inline float SetY( float y_ ) { return SetYImpl(self, y_); }
        inline float SetZ( float z_ ) { return SetZImpl(self, z_); }
        inline float SetW( float w_ ) { return SetWImpl(self, w_); }
         
        static GF GetXImpl;
        static GF GetYImpl;
        static GF GetZImpl;
        static GF GetWImpl;
        static SF SetXImpl;
        static SF SetYImpl;
        static SF SetZImpl;
        static SF SetWImpl;
        std::function< void (void*) > Copy;
        std::function< void () > Destroy;
        void* self;
    };
    template < typename T >
    struct model_t : base_t {
        T d;
        model_t(const T& t) : base_t(t), d(t) { self = &d;}    
        void Copy(void* storage) const { new (storage) model_t(*this); }
    };
    std::vector< char > storage_;
    base_t* model_;
    ~wrapper2_t() {model_->Destroy();}
};
using GF = float (*)(const void* );
using SF = float (*)(void*, float);
GF wrapper2_t::base_t::GetXImpl = nullptr;
GF wrapper2_t::base_t::GetYImpl = nullptr;
GF wrapper2_t::base_t::GetZImpl = nullptr;
GF wrapper2_t::base_t::GetWImpl = nullptr;
SF wrapper2_t::base_t::SetXImpl = nullptr;
SF wrapper2_t::base_t::SetYImpl = nullptr;
SF wrapper2_t::base_t::SetZImpl = nullptr;
SF wrapper2_t::base_t::SetWImpl = nullptr;
//from
//http://assemblyrequired.crashworks.org/2009/01/19/how-slow-are-virtual-functions-really

class Vector4TestV final {
  float x,y,z,w;
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
  float x,y,z,w;
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


std::vector< shared_ptr< Vector4TestV > > A(1024),
                                         B(1024),
                                         C(1024);
void test(int NUM_TESTS) {
    for (int n = 0 ; n != NUM_TESTS; ++n)
        for (int i=0; i != 1024 ; ++i) {
            C[i]->SetX(A[i]->GetX() + B[i]->GetX());
            C[i]->SetY(A[i]->GetY() + B[i]->GetY());
            C[i]->SetZ(A[i]->GetZ() + B[i]->GetZ());
            C[i]->SetW(A[i]->GetW() + B[i]->GetW());
        }
}

std::vector< wrapper_t > Aw(1024, wrapper_t(Vector4Test())),
                         Bw(1024, wrapper_t(Vector4Test())),  
                         Cw(1024, wrapper_t(Vector4Test()));
void testw(int NUM_TESTS) {
    for (int n = 0 ; n != NUM_TESTS; ++n)
        for (int i=0; i != 1024 ; ++i) {
            Cw[i].SetX(Aw[i].GetX() + Bw[i].GetX());
            Cw[i].SetY(Aw[i].GetY() + Bw[i].GetY());
            Cw[i].SetZ(Aw[i].GetZ() + Bw[i].GetZ());
            Cw[i].SetW(Aw[i].GetW() + Bw[i].GetW());
        }
}

std::vector< wrapper2_t > Aw2(1024, wrapper2_t(Vector4Test())),
                          Bw2(1024, wrapper2_t(Vector4Test())),  
                          Cw2(1024, wrapper2_t(Vector4Test()));
void testw2(int NUM_TESTS) {
    for (int n = 0 ; n != NUM_TESTS; ++n)
        for (int i=0; i != 1024 ; ++i) {
            Cw2[i].SetX(Aw2[i].GetX() + Bw2[i].GetX());
            Cw2[i].SetY(Aw2[i].GetY() + Bw2[i].GetY());
            Cw2[i].SetZ(Aw2[i].GetZ() + Bw2[i].GetZ());
            Cw2[i].SetW(Aw2[i].GetW() + Bw2[i].GetW());
        }
}

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
    // wrapper3_t(wrapper3_t&& w) : model_(&storage_0) {
    //     memcpy(storage_, w.storage, 0x100);
    // }

    wrapper3_t(const wrapper3_t& w) : model_((base_t*)&storage_[0]) {
        w.model_->Copy(&storage_[0]);
    }
    template < typename T >
    wrapper3_t(const T& t) : model_((base_t*)&storage_[0]) {
        new (&storage_[0]) model_t< T >(t);
    }

    wrapper3_t& operator=(wrapper3_t w) {
        memcpy(storage_, w.storage_, 0x100);
        return *this;
    }
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
        virtual void Copy(void* ) const = 0;
        virtual ~base_t() {}
    };
    template < typename T >
    struct model_t final: base_t {
        T d;
        model_t(const T& t) : d(t) {}    
        base_t* Copy() const { return new model_t(*this); }
        virtual float GetX() const { return d.GetX(); }
        virtual float GetY() const { return d.GetY(); }
        virtual float GetZ() const { return d.GetZ(); }
        virtual float GetW() const { return d.GetW(); }
        virtual float SetX( float x_ ) { return d.SetX(x_); }
        virtual float SetY( float y_ ) { return d.SetY(y_); }
        virtual float SetZ( float z_ ) { return d.SetZ(z_); }
        virtual float SetW( float w_ ) { return d.SetW(w_); }
        virtual void Copy(void* addr) const { new (addr) model_t< T >(*this);}

    };
    base_t* model_;
    char storage_[0x100];
    //unique_ptr< base_t > model_;
    ~wrapper3_t() {model_->base_t::~base_t();}
};

std::vector< wrapper3_t > Aw3(1024, wrapper3_t(Vector4Test())),
                          Bw3(1024, wrapper3_t(Vector4Test())),  
                          Cw3(1024, wrapper3_t(Vector4Test()));
void testw3(int NUM_TESTS) {
    for (int n = 0 ; n != NUM_TESTS; ++n)
        for (int i=0; i != 1024 ; ++i) {
            Cw3[i].SetX(Aw3[i].GetX() + Bw3[i].GetX());
            Cw3[i].SetY(Aw3[i].GetY() + Bw3[i].GetY());
            Cw3[i].SetZ(Aw3[i].GetZ() + Bw3[i].GetZ());
            Cw3[i].SetW(Aw3[i].GetW() + Bw3[i].GetW());
        }
}
//------------------------------------------------------------------------------
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
                return static_cast< model_t< T >& >(*this).d.T::~T();
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
std::vector< wrapper4_t > Aw4(1024, wrapper4_t(Vector4Test())),
                          Bw4(1024, wrapper4_t(Vector4Test())),  
                          Cw4(1024, wrapper4_t(Vector4Test()));
void testw4(int NUM_TESTS) {
    for (int n = 0 ; n != NUM_TESTS; ++n)
        for (int i=0; i != 1024 ; ++i) {
            Cw4[i].SetX(Aw4[i].GetX() + Bw4[i].GetX());
            Cw4[i].SetY(Aw4[i].GetY() + Bw4[i].GetY());
            Cw4[i].SetZ(Aw4[i].GetZ() + Bw4[i].GetZ());
            Cw4[i].SetW(Aw4[i].GetW() + Bw4[i].GetW());
        }
}

struct Base {
    void bar() {}
    void foo() {std::cout << "B\n";}
    void mimpl() { (this->*m)();}
    using M = void (Base::*)();
    M m;
};

struct Derived : Base {
    Derived() {
        m = (M) &Derived::foo;
    }
    void foo() {std::cout << "C\n";}
};

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
    // wrapper5_t(wrapper5_t&& w) : model_(&storage_0) {
    //     memcpy(storage_, w.storage, 0x100);
    // }

    wrapper5_t(const wrapper5_t& w) : model_((base_t*)&storage_[0]) {
        w.model_->Copy(&storage_[0]);
    }
    template < typename T >
    wrapper5_t(const T& t) : model_((base_t*)&storage_[0]) {
        new (&storage_[0]) model_t< T >(t);
    }

    wrapper5_t& operator=(wrapper3_t w) {
        memcpy(storage_, w.storage_, 0x100);
        return *this;
    }
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
        void Copy(void* addr) const { (this->*CopyImpl)(addr);}
        using GF = float (base_t::*)() const;
        using SF = float (base_t::*)(float);
        using CP = void (base_t::*)(void*) const; 
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
        base_t* Copy() const { return new model_t(*this); }
        float GetX() const { return d.GetX(); }
        float GetY() const { return d.GetY(); }
        float GetZ() const { return d.GetZ(); }
        float GetW() const { return d.GetW(); }
        float SetX( float x_ ) { return d.SetX(x_); }
        float SetY( float y_ ) { return d.SetY(y_); }
        float SetZ( float z_ ) { return d.SetZ(z_); }
        float SetW( float w_ ) { return d.SetW(w_); }
        void Copy(void* addr) const { new (addr) model_t< T >(*this);}

    };
    base_t* model_;
    char storage_[0x100];
    //unique_ptr< base_t > model_;
    //~wrapper5_t() {model_->base_t::~base_t();}
};
std::vector< wrapper5_t > Aw5(1024, wrapper5_t(Vector4Test())),
                          Bw5(1024, wrapper5_t(Vector4Test())),  
                          Cw5(1024, wrapper5_t(Vector4Test()));
void testw5(int NUM_TESTS) {
    for (int n = 0 ; n != NUM_TESTS; ++n)
        for (int i=0; i != 1024 ; ++i) {
            Cw5[i].SetX(Aw5[i].GetX() + Bw5[i].GetX());
            Cw5[i].SetY(Aw5[i].GetY() + Bw5[i].GetY());
            Cw5[i].SetZ(Aw5[i].GetZ() + Bw5[i].GetZ());
            Cw5[i].SetW(Aw5[i].GetW() + Bw5[i].GetW());
        }
}

int main(int argc, char** argv) {

    Derived der;
    Base* b=&der;
    b->mimpl();

    for(int i = 0; i != 1024; ++i) {
        A[i] = shared_ptr< Vector4TestV >( new Vector4TestV );
        B[i] = shared_ptr< Vector4TestV >( new Vector4TestV );
        C[i] = shared_ptr< Vector4TestV >( new Vector4TestV );
    }
    int numtests = argc == 1 ? 1 : stoi(argv[1]);
    using myclock_t = chrono::high_resolution_clock;

    auto t1 = myclock_t::now();
    testw(numtests);
    auto t2 = myclock_t::now();
    auto d = t2 - t1;
    cout << double(chrono::nanoseconds(d).count()) / numtests << endl;

    t1 = myclock_t::now();
    test(numtests);
    t2 = myclock_t::now();
    d = t2 - t1;
    cout << double(chrono::nanoseconds(d).count()) / numtests << endl;

    t1 = myclock_t::now();
    testw2(numtests);
    t2 = myclock_t::now();
    d = t2 - t1;
    cout << double(chrono::nanoseconds(d).count()) / numtests << endl;

    t1 = myclock_t::now();
    testw3(numtests);
    t2 = myclock_t::now();
    d = t2 - t1;
    cout << double(chrono::nanoseconds(d).count()) / numtests << endl;

    t1 = myclock_t::now();
    testw4(numtests);
    t2 = myclock_t::now();
    d = t2 - t1;
    cout << double(chrono::nanoseconds(d).count()) / numtests << endl;

    t1 = myclock_t::now();
    testw5(numtests);
    t2 = myclock_t::now();
    d = t2 - t1;
    cout << double(chrono::nanoseconds(d).count()) / numtests << endl;

    return 0;
}

#endif

