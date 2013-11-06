//Author: Ugo Varetto
//variadic list -> std::vector<void*> holding pointers to
//passed value references, works with temporary value references as well;
//pointers to temporary references are of course invalid outside the scope
//where the temporary is istantiated

#include <vector>
#include <cassert>
#include <iostream>
#include <type_traits>

//------------------------------------------------------------------------------
// create a vector of pointers to values
// WARNING: it works with rvalue references so the pointers are valid only
// for the lifetime of the temporary objects e.g. temporary parameters passed to
// a function are valid only inside the function body 
void make_vector_impl(std::vector< void* >&) {}

template < typename H, typename...Args > 
void make_vector_impl(std::vector< void* >& v, H&& h, Args&&...args) {
    v.push_back((void*)(&h));
    make_vector_impl(v, std::forward< Args >(args)...);            
}

template < typename T, typename... Args >
std::vector< void* > make_vector(T&& h, Args&&...args) {
    std::vector< void* > v;
    v.reserve(sizeof...(Args));
    v.push_back((void*)(&h));
    make_vector_impl(v, std::forward< Args >(args)...);
    return v;
}
std::vector< void* > make_vector() { return std::vector< void* >(); }

//------------------------------------------------------------------------------
//print_impl is declared as a template because it is invoked by other
//print_impl specializations as print_impl<>() 
template < int = 0 >
void print_impl(std::vector< void* >::const_iterator& ) {
    std::cout << std::endl;
}

template < typename H, typename...Args >
void print_impl(std::vector< void* >::const_iterator& i) {
    std::cout <<
        *reinterpret_cast< typename std::remove_reference< H >::type* >(*i++)
              << ' ';
    //explicitly calls a template function declared as f<...>()          
    print_impl< Args... >(i); 
}

void print() {
    std::vector< void* >::const_iterator i;
    print_impl(i);
}

template < typename H, typename...Args >
void print(H&& h, Args&&...args) {
    std::cout << h << ' ';
    const std::vector< void* > v = make_vector(std::forward<Args>(args)...);
    std::vector< void* >::const_iterator i = begin(v);
    print_impl< Args... >(i);
}

//------------------------------------------------------------------------------
int main(int, char**) {
    int a = 1;
    float b = 2.0f;
    std::vector< void* > vp = make_vector(a, b);
    assert(&a == vp[0]);
    assert(&b == vp[1]);
    print(a, b);
    print(2, 5, "hey");
    print();
    return 0;
}
