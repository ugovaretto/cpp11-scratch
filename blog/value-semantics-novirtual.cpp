#include <iostream>
#include <cassert>
#include <functional>
#include <memory>
#include <chrono>
#include <ctime>
#include <string>
#include <vector>

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
#define V
#else
#define V virtual
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
        template < typename T >
        base_t(const T& t) {
            const T* p = &static_cast< const model_t< T >* >(this)->d;
            T* pp = &static_cast< model_t< T >* >(this)->d;
            GetXImpl = (GF)([](const void* self) {
                return reinterpret_cast< const model_t< T >* >(self)->d.GetX();
                           
            });
            GetYImpl = (GF)([](const void* self) {
                return reinterpret_cast< const model_t< T >* >(self)->d.GetY();
                           
            });
            GetZImpl = (GF)([](const void* self) {
                return reinterpret_cast< const model_t< T >* >(self)->d.GetZ();
                           
            });
            GetWImpl = (GF)([](const void* self) {
                return reinterpret_cast< const model_t< T >* >(self)->d.GetW();
                           
            });
            SetXImpl = (SF)([](void* self, float x_) {
                return reinterpret_cast< model_t< T >* >(self)->d.SetX(x_);
            });
            SetYImpl = (SF)([](void* self, float y_) {
                return reinterpret_cast< model_t< T >* >(self)->d.SetY(y_);
            });
            SetZImpl = (SF)([](void* self, float z_) {
                return reinterpret_cast< model_t< T >* >(self)->d.SetZ(z_);
            });
            SetWImpl = (SF)([](void* self, float w_) {
                return reinterpret_cast< model_t< T >* >(self)->d.SetW(w_);
            });
            Copy = [this]() {
                return static_cast< const model_t< T >& >(*this).Copy();
            };
            Destroy = [this]() {
                return static_cast< model_t< T >& >(*this).d.T::~T();
            };                    
        }
        float GetX() const { return GetXImpl(this); }
        float GetY() const { return GetYImpl(this); }
        float GetZ() const { return GetZImpl(this); }
        float GetW() const { return GetWImpl(this); }
        float SetX( float x_ ) { return SetXImpl(this, x_); }
        float SetY( float y_ ) { return SetYImpl(this, y_); }
        float SetZ( float z_ ) { return SetZImpl(this, z_); }
        float SetW( float w_ ) { return SetWImpl(this, w_); }
         
        GF GetXImpl;
        GF GetYImpl;
        GF GetZImpl;
        GF GetWImpl;
        SF SetXImpl;
        SF SetYImpl;
        SF SetZImpl;
        SF SetWImpl;
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


//from
//http://assemblyrequired.crashworks.org/2009/01/19/how-slow-are-virtual-functions-really

class Vector4Test {
  float x,y,z,w;
public:
    V float GetX() const { return x; }
    V float GetY() const { return y; }
    V float GetZ() const { return z; }
    V float GetW() const { return w; }
    V float SetX( float x_ ) { return x = x_; }
    V float SetY( float y_ ) { return y = y_; }
    V float SetZ( float z_ ) { return z = z_; }
    V float SetW( float w_ ) { return w = w_; }
    virtual ~Vector4Test() {}
};

std::vector< shared_ptr< Vector4Test > > A(1024),
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


int main(int argc, char** argv) {

    for(int i = 0; i != 1024; ++i) {
        A[i] = shared_ptr< Vector4Test >( new Vector4Test );
        B[i] = shared_ptr< Vector4Test >( new Vector4Test );
        C[i] = shared_ptr< Vector4Test >( new Vector4Test );
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
    return 0;
}

#endif

