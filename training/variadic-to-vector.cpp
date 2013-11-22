//Author: Ugo Varetto
//variadic list -> std::vector<void*> holding pointers to
//passed value references, works with temporary value references as well;
//pointers to temporary references are of course invalid outside the scope
//where the temporary is istantiated
//Useful to abstract calls to (C) functions taking a void ** type holding
//pointers to values to be copied; one example: cuLaunchKernel function in the
//CUDA driver API.

#include <vector>
#include <cassert>
#include <type_traits>
#include <tuple>
#include <sstream>
#include <functional>

//------------------------------------------------------------------------------
// create a vector of pointers to values
// WARNING: it works with rvalue references so the pointers are valid only
// for the lifetime of the temporary objects e.g. temporary parameters passed to
// a function are valid only inside the function body 
void make_vector_helper(std::vector< void* >&) {}

template < typename H, typename...Args > 
void make_vector_helper(std::vector< void* >& v, H&& h, Args&&...args) {
    v.push_back((void*)(&h));
    make_vector_helper(v, std::forward< Args >(args)...);            
}

template < typename T, typename... Args >
std::vector< void* > make_vector(T&& h, Args&&...args) {
    std::vector< void* > v;
    v.reserve(sizeof...(Args));
    v.push_back((void*)(&h));
    make_vector_helper(v, std::forward< Args >(args)...);
    return v;
}
std::vector< void* > make_vector() { return std::vector< void* >(); }

//------------------------------------------------------------------------------
//print_helper is declared as a template because it is invoked by other
//print_helper specializations as print_helper<>() 
template < int = 0 >
void print_helper(std::ostream& os, std::vector< void* >::const_iterator& ) {
    os << '\n';
}

template < typename H, typename...Args >
void print_helper(std::ostream& os, std::vector< void* >::const_iterator& i) {
    os <<
        *reinterpret_cast< typename std::remove_reference< H >::type* >(*i++)
              << ' ';
    //explicitly calls a template function declared as f<...>()          
    print_helper< Args... >(os, i); 
}

void print(std::ostream& os) {
    std::vector< void* >::const_iterator i;
    print_helper(os, i);
}

template < typename H, typename...Args >
void print(std::ostream& os, H&& h, Args&&...args) {
    os << h << ' ';
    const std::vector< void* > v = make_vector(std::forward<Args>(args)...);
    std::vector< void* >::const_iterator i = begin(v);
    print_helper< Args... >(os, i);
}

//------------------------------------------------------------------------------
template < typename T >
void to_tuple_helper(const T&, std::vector< void* >&) {}

template <typename T, typename H, typename... Args >
void to_tuple_helper(T& t, std::vector< void* >& v) {
    constexpr size_t i = std::tuple_size< T >::value - sizeof...(Args);
    std::get< i - 1 >(t) = std::move(*reinterpret_cast< const H* >(v[i - 1]));
    to_tuple_helper< T, Args... >(t, v);
}

template < typename H, typename...Args >
std::tuple< H, Args... > to_tuple(std::vector< void* >& v) {
    assert(sizeof...(Args) < v.size());
    std::tuple< H, Args... > t;
    std::get< 0 >(t) = std::move(*reinterpret_cast< const H* >(v.front()));
    to_tuple_helper< std::tuple< H, Args... >, Args... >(t, v);
    return t;
}

//------------------------------------------------------------------------------
template < typename T >
void to_tupleref_helper(const T&, std::vector< void* >&) {}

template <typename T, typename H, typename... Args >
void to_tupleref_helper(T& t, std::vector< void* >& v) {
    constexpr size_t i = std::tuple_size< T >::value - sizeof...(Args);
    std::get< i - 1 >(t) = reinterpret_cast< H* >(v[i - 1]);
    to_tupleref_helper< T, Args... >(t, v);
}

template < typename H, typename...Args >
std::tuple< H*, Args*... > to_tupleref(std::vector< void* >& v) {
    assert(sizeof...(Args) < v.size());
    using tt = 
    std::tuple< H*, Args*... >;
    tt t;
    std::get< 0 >(t) = reinterpret_cast< H* >(v.front());
    to_tupleref_helper< tt, Args... >(t, v);
    return t;
}

//==============================================================================
//version 2: shorter and cleaner

//------------------------------------------------------------------------------
template < typename... Args >
std::vector< void* > make_vector_2(Args&&...args) {
    void* v[] = {const_cast< void* >(
                   static_cast< const void* >(
                     /*const_cast< OR*/
                     (typename std::remove_reference<Args>::type* const)
                        (&args)))...};
        //{(void*) &args...};

    return std::vector< void* >(v, v + sizeof...(Args));
}

//------------------------------------------------------------------------------
template < int... >
struct indexes_t {};

template < int N, int...I >
struct gen_indexes_t : gen_indexes_t< N - 1, N - 1, I... > {};

template < int...I >
struct gen_indexes_t< 0, I... > {
    typedef indexes_t< I... > type;
};

template < typename...Args, int...I >
std::tuple< Args... > make_tuple_helper(
    indexes_t< I... >,
    std::vector< void* >& v) {
    return std::tuple< Args... >(*reinterpret_cast< Args* >(v[I])...);
}

template < typename...Args >
std::tuple< Args...> make_tuple(std::vector< void* >& v) {
    return make_tuple_helper<Args...>(
        typename gen_indexes_t< sizeof...(Args) >::type(), v);
}

template < typename...Args, int...I >
std::tuple< Args*... > make_tuple_ptr_helper(
    indexes_t< I... >,
    std::vector< void* >& v) {
    return std::tuple< Args*... >(reinterpret_cast< Args* >(v[I])...);
}

template < typename...Args >
std::tuple< Args*...> make_tuple_ptr(std::vector< void* >& v) {
    return make_tuple_ptr_helper<Args...>(
        typename gen_indexes_t< sizeof...(Args) >::type(), v);
}

template < typename...Args, int...I >
std::tuple< std::reference_wrapper< Args >... > make_tuple_ref_helper(
    indexes_t< I... >,
    std::vector< void* >& v) {
    return std::tuple< std::reference_wrapper< Args >... >(
        *reinterpret_cast< Args* >(v[I])...);
}

template < typename...Args >
std::tuple< std::reference_wrapper< Args >... >
make_tuple_ref(std::vector< void* >& v) {
    return make_tuple_ref_helper<Args...>(
        typename gen_indexes_t< sizeof...(Args) >::type(), v);
}

//------------------------------------------------------------------------------
template < typename...Dummy >
void dummy__(Dummy&&...) {}

template< typename F, typename T >
int call__(F&& f, T&& t) {
    f(t);
    return 0;
}

template < typename T, typename... F, int...I >
void apply_to_tuple_helper(indexes_t< I...>, T&& t, F&&... f) {
    using TT = typename std::remove_reference< T >::type;
    dummy__(call__(std::forward< F >(f), 
          std::forward< typename std::tuple_element< I, TT >::type >(
                                                        std::get< I >(t)))...);
}

template< typename T, typename... F >
void apply_to_tuple(T&& t, F&&... f) {
    using TT = typename std::remove_reference< T >::type;
    apply_to_tuple_helper(
        typename gen_indexes_t< std::tuple_size< TT >::value >::type(),
        std::forward< T >(t),
        std::forward< F >(f)...);
}

//in case the size of F... different from tuple size
//error: pack expansion contains parameter packs 'f' and 'I' that have 
//different lengths (1 vs. 3)

//------------------------------------------------------------------------------
void foo(std::ostream& os, int i)    { os << i << ' ';}
void foo(std::ostream& os, float f)  { os << f << ' ';}
void foo(std::ostream& os, double d) { os << d << ' ';}


//------------------------------------------------------------------------------
int main(int, char**) {

    int a = 1;
    float b = 2.0f;
    double d = 1.0;
    int& ra = a;
    
    std::vector< void* > vp = make_vector_2(ra, b, d);
    assert(&a == vp[0]);
    assert(&b == vp[1]);
    assert(&d == vp[2]);
    
    auto t2 = make_tuple< int, float, double >(vp);
    assert(std::get< 0 >(t2) == a);
    assert(std::get< 1 >(t2) == b);
    assert(std::get< 2 >(t2) == d);

    std::ostringstream oss;
    
    using namespace std::placeholders;
    using fooint_t    = void (*)(std::ostream&, int);
    using foofloat_t  = void (*)(std::ostream&, float);
    using foodouble_t = void (*)(std::ostream&, double);
    apply_to_tuple(t2,
                   std::bind((fooint_t)    foo, std::ref(oss), _1),
                   std::bind((foofloat_t)  foo, std::ref(oss), _1),
                   std::bind((foodouble_t) foo, std::ref(oss), _1));
    assert(oss.str() == "1 2 1 ");
    oss.str("");
   
    print(oss, a, b);
    assert(oss.str() == "1 2 \n");
    oss.str("");
    print(oss, 2, 5, "hey");
    assert(oss.str() == "2 5 hey \n");
    oss.str("");
    print(oss);
    assert(oss.str() == "\n");
   
    std::tuple< int, float, double > t = to_tuple< int, float, double >(vp);
    assert(std::get< 0 >(t) == a);
    assert(std::get< 1 >(t) == b);
    assert(std::get< 2 >(t) == d);
   
    auto tref = to_tupleref< int, float, double >(vp);
    *std::get< 0 >(tref) = 10;
    assert(a == 10);

    auto rw = make_tuple_ref< int, float, double >(vp); 
    assert(std::get< 0 >(rw).get() == a);
    std::get< 0 >(rw).get() = 12;
    assert(a == 12);

    return 0;
}
