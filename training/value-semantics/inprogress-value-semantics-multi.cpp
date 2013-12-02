//Author: Ugo Varetto
//IN PROGRESS
//small library to simplify construction of value objects

#include <cassert>
#include <functional>
#include <memory>
#include <chrono>
#include <ctime>
#include <string>
#include <vector>
#include <cstring>
#include <iostream>
                        

using namespace std;

#ifdef NOVIRTUAL
#define VIR
#else
#define VIR virtual
#endif


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
