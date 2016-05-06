//
// Author: Ugo Varetto
//
// Reference custom allocator
//


#include <cstddef> //std::size_t
#include <memory>  //std::allocator
#include <stdexcept>

//http://en.cppreference.com/w/cpp/concept/Allocator
template<typename T>
class Allocator {
public :
    using value_type = T;
    using pointer = T *;                    // optional
    using const_pointer = const T *;        // optional
    using void_pointer = void *;            // optional
    using const_void_pointer = const void *; // optional
    using reference = T &;
    using const_reference = const T &;
    using size_type    = std::size_t;       // optional
    using difference_type = std::ptrdiff_t; // optional
    // Allocator< T > -> Allocator< U >
    template<typename U>                    // optional
    struct rebind {
        typedef Allocator<U> other;
    };
    //C++14
    using propagate_on_container_move_assignment = std::true_type;
    //C++17
    using is_always_equal =    std::true_type;

public :
    explicit Allocator() { }

    ~Allocator() { }

    explicit Allocator(Allocator const &) { }

    template<typename U>
    explicit Allocator(Allocator<U> const &) { }

    pointer address(reference r) { return &r; }

    const_pointer address(const_reference r) { return &r; }

    pointer allocate(size_type count) {
        return reinterpret_cast< pointer >(::operator new(count * sizeof(T)));
    }
    // optional
    pointer allocate(size_type,
                     typename std::allocator<void>::const_pointer) {
        throw std::logic_error(
                "allocate(size_type, const_pointer) not implemented!");
    }

    void deallocate(pointer p, size_type) {
        ::operator delete(p);
    }

    // optional
    size_type max_size() const {
        return std::numeric_limits<size_type>::max() / sizeof(T);
    }

    // construction
    void construct(pointer p, const T &t) { new(p) T(t); }

    // optional - destruction
    void destroy(pointer p) { p->~T(); }

    bool operator==(Allocator const &) { return true; }

    bool operator!=(Allocator const &a) { return !operator==(a); }
};



#include <iostream>
#include <string>

using namespace std;

int main(int, char **) {
    Allocator<int> a1;
    int *a = a1.allocate(10);

    a[9] = 7;

    cout << a[9] << endl;

    a1.deallocate(a, 10);

    Allocator<std::string> a2;


    decltype(a1)::rebind<string>::other a2_1;

    // same, but obtained by rebinding from the type of a1 via allocator_traits
    std::allocator_traits<decltype(a1)>::rebind_alloc<string> a2_2;

    std::string *s = a2.allocate(2); // space for 2 strings

    a2.construct(s, "foo");
    a2.construct(s + 1, "bar");

    std::cout << s[0] << ' ' << s[1] << '\n';

    a2.destroy(s);
    a2.destroy(s + 1);
    a2.deallocate(s, 2);

    return 0;
}

