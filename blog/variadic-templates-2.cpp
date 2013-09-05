#if __cplusplus < 201103L  // C++ 11
#error "C++ 11 support required"
#endif 

#include <iostream>
#include <string>
#include <functional>

//------------------------------------------------------------------------------
struct P1 {
    void P1do() {}
};

struct P2 {
    void P2do() {}
};

struct P3 {
    void P3do() {}
};

template < typename... Policies >
class P123 : public Policies... {};

//------------------------------------------------------------------------------
class Wrapped {
public:
    Wrapped(int i1, const std::string& str) : i1_(i1), str_(str) {}
private:
    int i1_;
    std::string str_;    
}; 

template < typename T >
class Wrapper {
public:
    template < typename... Params >
    Wrapper(const Params&... params) 
        : wrapped_(new Wrapped(std::forward(params...))) {}
    ~Wrapper() { delete wrapped_; }    
private:
    Wrapped* wrapped_;    
};

//------------------------------------------------------------------------------
struct Base1 {
    int b1_;
    Base1(const int& i) : b1_( i ) {}
};
struct Base2 {
    int b2_;
    Base2(const int& i) : b2_( i ) {}
};
struct Base3 {
    int b3_;
    Base3(const int& i) : b3_( i ) {}
};

template < typename Head, typename... Tail >
struct Init {
    template < typename T, typename... Args >
    static void init(Head* p, const T& h, const Args&... t) {
        new (p) Head(h);
        Init< Tail... >::init(p, t...);
    }
};

template < typename T >
struct Init< T > {
    template < typename A >
    static void init(T* p, const A& a) {
        new (p) T(a);
    }
};



template < typename... Bases >
struct Derived : Bases... {
    template < typename... Args > 
    Derived( const Args&... args ) {
        Init< Bases... >::init(this, args...);
    }   
};


//------------------------------------------------------------------------------
int main(int, char**) {
    P123< P1, P2 > p12;
    P123< P1, P2, P3> p123;
    p12.P1do();
    p12.P2do();
    p123.P1do();
    p123.P2do();
    p123.P3do();
    Wrapped w(1, "2");
    Derived< Base1, Base2, Base3 > d(1, 2, 3);
    return 0;
}
