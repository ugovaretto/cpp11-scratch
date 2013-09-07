
#include <cassert>
#include <iostream>
#include <functional>

template < int Count, int Bound, typename HeadT, typename... TailT >
class TupleStorage : public TupleStorage< Count + 1, Bound, TailT... > {
public:
    typedef HeadT Type;
    typedef TupleStorage< Count + 1, Bound, TailT... > Base;
    TupleStorage(TupleStorage&& ts) : v_(std::move(ts.v_)), Base(ts) {}
    TupleStorage(const TupleStorage& ts) : v_(ts.v_), Base(ts) {}
    TupleStorage() : v_(HeadT()), 
                     Base() {}
    TupleStorage(const HeadT& v, const TailT&... params ) 
        : v_(v),  Base(params...) {}
    TupleStorage(HeadT&& v, TailT&&... params ) 
        : v_(v),  Base(params...) {}    
    const HeadT& Get() const { return v_; }
    HeadT& Get() { return v_; }
    TupleStorage& operator=(TupleStorage&& ts) {
        v_= std::move(ts.v_);
        Base::operator=(ts);
        return *this;
    }
    TupleStorage& operator=(const TupleStorage& ts) {
        v_= ts.v_;
        Base::operator=(ts);
        return *this;
    }
private:
    HeadT v_;        
};

template < int Bound, typename T>
class TupleStorage< Bound, Bound, T> {
public:
    typedef T Type;
    TupleStorage(TupleStorage&& ts) : v_(std::move(ts.v_)) {}
    TupleStorage(const TupleStorage& ts) : v_(ts.v_) {}
    TupleStorage() : v_(T()) {}
    TupleStorage(const T& v) : v_(v) {}
    TupleStorage(T&& v) : v_(v) {}
    const T& Get() const { return v_; }
    T& Get() { return v_; }
    TupleStorage& operator=(TupleStorage&& ts) {
        v_ = std::move(ts.v_);
        return *this;
    }
    TupleStorage& operator=(const TupleStorage& ts) {
        v_ = ts.v_;
        return *this;
    }
private:
    T v_;        
};

template < int I, int Count, size_t Bound, typename HeadT, typename... TailT >
struct GetType { 
    typedef typename GetType< I, Count + 1, Bound, TailT... >::Type Type;   
};

template < int I, size_t Bound, typename HeadT, typename... TailT >
struct GetType< I, I, Bound, HeadT, TailT... > { 
    typedef TupleStorage< I, Bound, HeadT, TailT... > Type;   
};

template < typename... Args > 
class Tuple : public TupleStorage< 0, sizeof... (Args) - 1, Args... > {
public:
    typedef Tuple< Args... > Type;
    typedef TupleStorage< 0, sizeof... (Args) - 1, Args... > Base;
    Tuple(Type&& t)
        : Base(t) {}
    Tuple(const Type& t)
        : Base(t) {}
    Tuple(Args&&... args)
        : Base(args...) {}
    Tuple(const Args&... args) 
        : Base(args...) {}
    Tuple() 
        : Base() {}    
    template < int i >
    typename GetType< i, 0, sizeof... (Args) - 1, Args... >::Type::Type Get() {
        typedef typename 
            GetType< i, 0, sizeof... (Args) - 1, Args... >::Type Base;
        return Base::Get();
    }
    Tuple& operator=(const Tuple& t) {
        //XXX: implement swap
        if(this == &t) return *this; 
        Base::operator=(t);
        return *this;
    }
    Tuple& operator=(Tuple&& t) {
        //XXX: implement swap
        if(this == &t) return *this; 
        Base::operator=(t);
        return *this;
    }    
};

//to do swap, move...

//-----------------------------------------------------------------------------
// template < int pos, int count, int Head, int... Tail > 
// struct Get {
//     enum {value = Get< pos, count + 1, Tail... >::value};
// };

// template < int p, int Head, int... Tail >
// struct Get< p, p, Head, Tail... > {
//     enum {value = Head};
// };

// template < int... I >
// struct Ints {
//     template < int p >
//     int get() { return Get< p, 0, I... >::value; }
// };

// int main(int, char**) {
//   Ints< 0, 1, 2 > i;
//   std::cout << i.get<1>() << std::endl;   
//   return 0;
// }

//------------------------------------------------------------------------------
int main(int, char**) {
    Tuple< int, float, char > t(1, 2.0f, '3');
    assert(t.Get< 0 >() == 1);
    assert(t.Get< 1 >() == 2.0f);
    assert(t.Get< 2 >() == '3');
    Tuple< int, float, char > t2;
    t2 = t;
    assert(t2.Get< 0 >() == 1);
    assert(t2.Get< 1 >() == 2.0f);
    assert(t2.Get< 2 >() == '3');
    Tuple< int, float, char > t3(t2);
    return 0;
}


