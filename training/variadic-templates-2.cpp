#if __cplusplus < 201103L  // C++ 11
#error "C++ 11 support required"
#endif 

#include <string>
#include <cassert>
#include <iostream>

//------------------------------------------------------------------------------
struct P1 {
    void P1do() { std::cout << "P1::P1do" << std::endl; }
};

struct P2 {
    void P2do() { std::cout << "P2::P2do" << std::endl; }
};

struct P3 {
    void P3do() { std::cout << "P3::P3do" << std::endl; }
};

template < typename... Policies >
class P123 : public Policies... {};

//------------------------------------------------------------------------------
class Wrapped {
public:
    Wrapped(const int& i1, const std::string& str) : i1_(i1), str_(str) {}
private:
    int i1_;
    std::string str_;
    friend class Wrapper;    
}; 

class Wrapper {
public:
    template < typename... Params >
    Wrapper(const Params&... params) 
        : wrapped_(new Wrapped(params...)) {}
    ~Wrapper() { delete wrapped_; }
    int GetInt() const { return wrapped_->i1_; }
    const std::string& GetString() const { return wrapped_->str_; }    
private:
    Wrapped* wrapped_;    
};

//------------------------------------------------------------------------------
struct Base1 {
    int b1_;
    Base1() : b1_(-1) {}
    Base1(const int& i) : b1_( i ) {}
    void Set(const int& i) {  b1_ = i; }
};
struct Base2 {
    int b2_;
    Base2() : b2_(-2) {}
    Base2(const int& i) : b2_( i ) {}
    void Set(const int& i) { b2_ = i; }
};
struct Base3 {
    int b3_;
    Base3() : b3_(-3) {}
    Base3(const int& i) : b3_( i ) {}
    void Set(int i) { b3_ = i; }
};

template < typename Head, typename... Tail >
struct Initialize {
    template < typename Derived, typename T, typename... Args >
    static void Init(Derived& p, const T& h, const Args&... t) {
        static_cast< Head& >(p) = Head(h);   
        Initialize< Tail... >::Init(p, t...);
    }
};

template < typename T >
struct Initialize< T > {
    template < typename Derived, typename A >
    static void Init(Derived& p, const A& a) {
        static_cast< T& >(p) = T(a);   
    }
};

template < typename... Bases >
struct Derived : Bases... {
    template < typename... Args > 
    Derived(const Args&... args ) {
        Initialize< Bases... >::Init(*this, args...);
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
    Wrapper w(1, "2");
    assert(w.GetInt() == 1);
    assert(w.GetString() == "2");
    Derived< Base1, Base2, Base3 > d(1, 2, 3);
    assert(d.b1_ == 1);
    assert(d.b2_ == 2);
    assert(d.b3_ == 3);
    return 0;
}
