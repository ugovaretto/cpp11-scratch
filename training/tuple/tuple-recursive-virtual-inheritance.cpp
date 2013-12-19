//Author: Ugo Varetto
//mini tuple implementation - C++98: virtual recursive inheritance version
//strategy:
// - derive from base class templated on a typelist with all of the tuple types
// - use virtual inheritance to allow the most derived class to initialize
//   explicitly all of the base classes
// - create a static data member to initialize default constructor in case
//   the type is a reference; declaring a default initialized tuple with
//   reference types correctly generates a compile-time error, but a default
//   constructor is required to initialize the base classes (see below)

//ALL virtual base classes are initialized by the most derived one (tuple_t), 
//without an explicit default constructor tuple_storage_t cannot initialize
//the base classes which in turn do not have a default constructor neither   
//removing the default tuple_storage_t constructor you get:
//error: constructor for 'tuple_storage_t<list_t<int, list_t<char, 
//   list_t<double, list_t<empty_t, list_t<empty_t, empty_t> > > > > >' 
//   must explicitly initialize the base class
//     'tuple_storage_t<typename tail<list_t<empty_t, list_t<empty_t, empty_t>
//      > >::type>' which does not have a default constructor
//  tuple_storage_t(value_type v) : data_(v) {}   

#include <iostream>

//------------------------------------------------------------------------------
//Metaprogramming utilities
struct empty_t {};

template < typename T1, typename T2 >
struct is_same {
    enum {value = 0};
};

template < typename T >
struct  is_same< T, T > {
    enum {value = 1};
};

template < typename H, typename T = empty_t >
struct list_t {
    typedef H head;
    typedef T tail;
};
template < typename L1, typename L2 = empty_t >
struct cons {
    typedef list_t< L1, L2 > type;
};
template < typename L >
struct head {
    typedef typename L::head type;
};

template < typename L >
struct tail {
    typedef typename L::tail type;
};

template < typename L >
struct length {
    enum {value = 1 + length< tail< L > >::value};
};

template <>
struct length<empty_t> {
    enum {value = 0};
};

template < typename L, int N, int Cnt = 0 >
struct nth_type {
    typedef typename nth_type< 
        typename tail< L >::type, N, Cnt + 1 >::type type;
};

template < typename L, int N >
struct nth_type< L, N, N > {
    typedef typename head< L >::type type;
};

template < int N, int Cnt > 
struct nth_type< empty_t, N, Cnt > {
    typedef empty_t type;
};

//need to have remainder< , 0 > return the list iself, if not it's a problem
//when implementing get<>() methods (would have to explicitly specialize get
//for 0 case)

template < typename L, int C, int Cnt = 0 >
struct remainder {
    typedef typename remainder< 
        typename tail< L >::type, C, Cnt + 1 >::type type;
};

template < typename L, int C >
struct remainder< L, C, C > {
    typedef L type;
}; 

template <int C, int Cnt >
struct remainder< empty_t, C, Cnt > {
    typedef empty_t type;
};

//------------------------------------------------------------------------------
//tuple implementation

template < typename LT >
struct tuple_storage_t : virtual tuple_storage_t< typename tail< LT >::type > {
    typedef tuple_storage_t< LT > type; 
    typedef typename head< LT >::type value_type;
    typedef value_type& reference_type;
    typedef const reference_type const_reference_type;
    tuple_storage_t(value_type v) : data_(v) {}
    const_reference_type get() const { return data_; }
    reference_type get() { return data_; }
    tuple_storage_t() : data_(dummy_) {}
private:
    value_type data_;
    static value_type dummy_;
}; 

template <>
struct tuple_storage_t< empty_t > {tuple_storage_t(){}};

template < typename T0,
           typename T1 = empty_t,
           typename T2 = empty_t,
           typename T3 = empty_t,
           typename T4 = empty_t >
struct tuple_t : virtual tuple_storage_t< 
                                  typename cons< T0,
                                   typename cons< T1, 
                                    typename cons< T2,
                                     typename cons< T3,
                                      typename cons< T4 >::type
                                     >::type
                                    >::type
                                   >::type
                                  >::type > {

    typedef 
    typename cons< T0,
      typename cons< T1, 
        typename cons< T2,
          typename cons< T3,
            typename cons< T4 >::type
          >::type
        >::type
      >::type
    >::type typelist_t;
    
    tuple_t(T0 v0 = T0(),
            T1 v1 = T1(),
            T2 v2 = T2(),
            T3 v3 = T3(),
            T4 v4 = T4()) :
        tuple_storage_t< typename remainder< typelist_t, 0 >::type >(v0),
        tuple_storage_t< typename remainder< typelist_t, 1 >::type >(v1),   
        tuple_storage_t< typename remainder< typelist_t, 2 >::type >(v2),
        tuple_storage_t< typename remainder< typelist_t, 3 >::type >(v3),
        tuple_storage_t< typename remainder< typelist_t, 4 >::type >(v4) {}
    
    template < int id >
    struct storage {
        typedef 
        tuple_storage_t< typename remainder< typelist_t, id >::type >
        type;
    };

    template < int i >
    typename tuple_storage_t<
                    typename remainder< typelist_t, i >::type >
                    ::const_reference_type get() const {
        return tuple_storage_t<
                    typename remainder< typelist_t, i >::type >::get();
    }
    template < int i >
    typename tuple_storage_t<
                    typename remainder< typelist_t, i >::type >
                    ::reference_type get() {
        return tuple_storage_t<
                    typename remainder< typelist_t, i >::type >::get();
    }       
};

//storage<> is a template dependent type of a template parameter: use
// 'template' keyword in declaration
//
// get<> is a template method of a template type: use 'template' keyword
// in invocation

template < int id, typename Tuple >
typename Tuple::template storage< id >::type::const_reference_type
get(const Tuple& t) {
    return t.template get< id >();
}

template < int id, typename Tuple >
typename Tuple::template storage< id >::type::reference_type
get(Tuple& t) {
    return t.template get< id >();
}

//------------------------------------------------------------------------------
int main(int, char**) {
#if 0    
    typedef cons< int, cons< char, cons< float >::type >::type >::type tl_t;
    typedef cons< char, cons< float >::type >::type r_t;
    std::cout << is_same< remainder< tl_t, 1 >::type, r_t >::value;   
#endif    
    tuple_t< int, char, double > icd(1, '1', 1.0);
    std::cout << icd.get< 0 >() << ' ' 
              << icd.get< 1 >() << ' '
              << icd.get< 2 >() << std::endl;
    std::cout << get< 0 >(icd) << ' ' 
              << get< 1 >(icd) << ' '
              << get< 2 >(icd) << std::endl;          
    int a = 2;
    double b = 2.0;
    tuple_t< const int&, double& > r(a, b);
    r.get< 1 >() = 4.0;
    std::cout << b << std::endl;
    return 0;
}

