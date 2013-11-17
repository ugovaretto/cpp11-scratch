#include <cassert>

template < typename R, typename T, typename A, R (T::*M)(A) >
R Wrapper(void* obj, A a) {
    return (static_cast< T* >(obj)->*M)(a);
}


// template < typename R, typename A >
// struct Fun {
//     Fun( R (*f)(A) ) : f_(f) {}
//     template < typename T >
//     Fun( R (T::*f)(A) ) : f_(&Wrapper<R, T, A, f>) {}
//     R (*f_)(A);
//     R operator()(A a) { return f_(a); }
//     template < typename T >
//     R operator()(T& c, A a) {
//         return f_(&c, a);
//     }
// };



//------------------------------------------------------------------------------
int Twice(int i) { return 2 * i; }

struct Class {
    int Twice(int i) { return 2 * i; }
};

void test1 () {
    Fun< int, int > f(&Twice);
    assert(f(3) == 6);
    Fun < int, int > m (&Class::Twice);
    Class c;
    assert(m(c, 2) == 4);
    //Fun< int(int) > l = [](int i) { return 2 * i; } 
    //assert(l(10) == 20);
}

//------------------------------------------------------------------------------

int main(int, char**) {
    test1();
    return 0;
}               