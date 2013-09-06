
#include <cassert>
#include <iostream>

//-----------------------------------------------------------------------------
struct Nulltype {};

template < typename HeadT, typename TailT = Nulltype >
struct List {
    typedef HeadT Head;
    typedef TailT Tail;
};

template < typename ListT >
struct First {
    typedef typename ListT::Head Type;
};

template < typename ListT >
struct Rest {
    typedef typename ListT::Tail Type;
};

template < typename L >
struct Size {
    enum {size = 1 + Size< typename L::Tail >::size};
};

template <>
struct Size<Nulltype> {
    enum {size = 0};
};


template < typename NewT, typename L = Nulltype >
struct Cons {
    typedef List< NewT, L > Type;
};


template < int I, int N, typename ListT >
struct GetTypeHelper {
    typedef typename GetTypeHelper< I, N + 1, typename ListT::Tail >::Type Type;
    enum {t=false, n = N, i = I};
    
};

template < int I, typename ListT >
struct GetTypeHelper< I, I, ListT > {
    typedef typename ListT::Head Type;
    enum{t=true, i = I};
    
};

// template < int N, typename ListT >
// struct GetTypeHelper< 0, N, ListT > {
//     typedef typename ListT::Head Type;
// };

template < int I, typename ListT >
struct GetType {
    typedef typename GetTypeHelper< I,
                                    0, 
                                    ListT >::Type Type;    
};

template < typename T1, typename T2 >
struct TypeAssert {
    enum {Equal = 0};
};

template < typename T >
struct TypeAssert< T, T> {
    enum {Equal = 1};
};
//-----------------------------------------------------------------------------

template < int I, typename T >
class TupleStorage {
public:
    typedef T Type;
    TupleStorage(const T& v) : v_(v) {}
    const T& get() const { return v_; }
    T& get() { return v_; }
private:
    T v_;        
};

struct Empty {};

template < int I >
class TupleStorage< I, Empty > {
public:
    typedef Empty Type;
    TupleStorage(const Empty&) {}
};

template < typename T0,
           typename T1 = Empty,
           typename T2 = Empty,
           typename T3 = Empty > 
class Tuple : TupleStorage< 0, T0 >,
              TupleStorage< 1, T1 >,
              TupleStorage< 2, T2 >,
              TupleStorage< 3, T3 > {
typedef TupleStorage< 0, T0 > S0;
typedef TupleStorage< 1, T1 > S1;
typedef TupleStorage< 2, T2 > S2;
typedef TupleStorage< 3, T3 > S2;
typedef 
typename Cons< S0, 
    typename Cons< S1,
        typename Cons< S2,
            typename Cons< S3 >
::Type >
    ::Type >
        ::Type >
            ::Type Types;
public:
    Tuple(const T0& v0,
          const T1& v1 = T1(),
          const T2& v2 = T2(),
          const T3& v3 = T3()) :
        TupleStorage< 0, T0 >(v0),
        TupleStorage< 1, T1 >(v1),
        TupleStorage< 2, T2 >(v2),
        TupleStorage< 3, T3 >(v3) {}
    template < int i >
    typename GetType< i, Types >::Type::Type get() {
        return GetType< i, Types >::Type::get();
    }    
};

//-----------------------------------------------------------------------------

int main(int, char**) {
    typedef Cons< float, Cons<int, Cons<char>::Type >::Type >::Type List;
    assert(Size<List>::size == 3);
    typedef GetType< 0, List >::Type Type0;
    typedef GetType< 1, List >::Type Type1;
    typedef GetType< 2, List >::Type Type2;
    
    //Without the double parentheses in assert with clang llvm 3.2:
    //../tuple.cpp:133:40: error: too many arguments provided to function-like
    // macro invocation
    //assert(typename TypeAssert< Type0, float >::Equal == 1);
    //                                   ^
    //../tuple.cpp:133:5: error: use of undeclared identifier 'assert'
    assert((TypeAssert< Type0, float >::Equal == 1));
    assert((TypeAssert< Type1, int>::Equal == 1));
    assert((TypeAssert< Type2, char>::Equal == 1));
    Tuple< int, float > t(1, 2.0f);
    assert(t.get<0>() == 1);
    assert(t.get<1>() == 2.0f);
    return 0;
}



