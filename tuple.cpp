
#include <iostream>
// template < int I, typename T >
// class TupleStorage {
// public:
//     TupleStorage(const T& v) : v_(v) {}
//     const T& get() const { return v_; }
//     T& get() { return v_; }
// private:
//     T v_;        
// };

// struct Empty {};

// template < int I >
// class TupleStorage< I, Empty > {    
// };
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
    typedef typename ListT::Rest Type;
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
    typedef typename GetTypeHelper< I, N - 1, ListT >::Type Type;
    
};

template < int I, typename ListT >
struct GetTypeHelper< I, I, ListT > {
    typedef typename ListT::Head Type;
    
};

template < int I, typename ListT >
struct GetType {
    enum {idx = I,
          bound = Size<ListT>::size - 1};
    typedef typename GetTypeHelper< idx,
                                    bound, 
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

// template < typename T0,
//            typename T1 = Empty,
//            typename T2 = Empty> 
// class Tuple : TupleStorage< 0, T0 >,
//               TupleStorage< 1, T1 >,
//               TupleStorage< 2, T2 > {

// public:
//     Tuple(const T0& v0, const T1& v1 = T1(), const T2& v2 = T2()) :
//         TupleStorage< 0, T0 >(v0),
//         TupleStorage< 1, T1 >(v1),
//         TupleStorage< 2, T2 >(v2) {}
//     template < int i >
//     typename Type< i >::type get() {
//         return typename TupleStorage< i, typename 
//         Type< i >::type >::get();
//     }    
// };


// template < typename T0, typename T1, typename T2 >
// struct Tuple< T0, T1, T2 >::Type<0> {
//     typedef T0 type;
// };

// template < typename T0, typename T1, typename T2 >
// template <>
// struct Tuple< T0, T1, T2 >::Type<1> {
//     typedef T1 type;
// };

// template < typename T0, typename T1, typename T2 >
// template <>
// struct Tuple< T0, T1, T2 >::Type<2> {
//     typedef T2 type;
// };


int main(int, char**) {
    typedef Cons< float, Cons<int>::Type >::Type T;
    std::cout << Size<T>::size << std::endl;
    GetType< 0, T >::Type t;
    std::cout << TypeAssert< int, float >::Equal << std::endl;
    std::cout << TypeAssert< int, int >::Equal << std::endl;
    std::cout << TypeAssert< GetType< 1, T >::Type, int >::Equal << std::endl;
    //Tuple< int, float > t(1, 2.0f);
    //std::cout << t.get<0>() << ' ' << t.get<1>() << std::endl;
    return 0;
}



