//Author: Ugo Varetto
//mini tuple implementation - C++98

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

//------------------------------------------------------------------------------
//tuple implementation

template < typename T, int id >
struct tuple_storage_t {
    T data_;
    typedef T& reference_type;
    typedef const T& const_reference_type;
    tuple_storage_t(const_reference_type v) : data_( v ) {}
    const T& get() const { return data_; }
    T& get() { return data_; }
};

template < typename T, int id >
struct tuple_storage_t< T&, id > {
    T& data_;
    typedef T& reference_type;
    typedef const T& const_reference_type;
    tuple_storage_t(reference_type v) : data_( v ) {}
    const T& get() const { return data_; }
    T& get() { return data_; }
};

template < typename T, int id >
struct tuple_storage_t< const T&, id > {
    const T& data_;
    typedef const T& const_reference_type;
    tuple_storage_t(const_reference_type v) : data_( v ) {}
    const T& get() const { return data_; }
};


template <>
struct tuple_storage_t< empty_t > {};

template < typename L >
struct tuple_storage_t : tuple_storage_t< typename tail< L >::type > {
    typedef typename head< L >::type value_type;
    typedef value_type& reference_type;
    typedef const reference_type const_reference_type;
    tuple_storage_t(const value_type& v) : data_(v) {}
    const_reference_type get() const { return data; };
    reference_type get() { return data; }
};


template < typename T0,
           typename T1 = empty_t,
           typename T2 = empty_t,
           typename T3 = empty_t,
           typename T4 = empty_t >
struct tuple_t : tuple_storage_t< typename cons< T0,
                                   typename cons< T1, 
                                    typename cons< T2,
                                     typename cons< T3,
                                      typename const< T4 >::type
                                     >::type
                                    >::type
                                   >::type
                                  >::type > {

                                    
    tuple_t(T0 v0 = T0(),
            T1 v1 = T1(),
            T2 v2 = T2(),
            T3 v3 = T3(),
            T4 v4 = T4() ) : typename nth_type< base_t, 0 >::type(v0),
                             typename nth_type< base_t, 1 >::type(v1),
                             typename nth_type< base_t, 2 >::type(v2),
                             typename nth_type< base_t, 3 >::type(v3),
                             typename nth_type< base_t, 4 >::type(v4) {}
    
    template < int i >
    typename nth_type< types, i >::type::const_reference_type  get() const {
        return nth_type< types, i >::type::get();
    }
    template < int i >
    typename nth_type< types, i >::type::reference_type  get() {
        return nth_type< types, i >::type::get();
    }                           
};

template < int id, typename T >
typename tuple_storage_t< T, id >::const_reference_type get( 
    const tuple_storage_t< T, id >& t ) {
    return t.get();
}

template < int id, typename T > 
typename tuple_storage_t< T, id >::reference_type get(
    tuple_storage_t< T, id >& t ) {
    return t.get();
}

//------------------------------------------------------------------------------
int main(int, char**) {
    tuple_t< int, char, double > icd(1, '1', 1.0);
    std::cout << get< 0 >(icd) << std::endl;
    int a = 2;
    double b = 2.0;
    tuple_t< int&, double& > r(a, b);
    r.get< 1 >() = 4.0;
    std::cout << b << std::endl;
}

