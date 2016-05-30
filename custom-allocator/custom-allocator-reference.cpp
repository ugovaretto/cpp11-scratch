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
    using pointer = T *;                     // optional
    using const_pointer = const T *;         // optional
    using void_pointer = void *;             // optional
    using const_void_pointer = const void *; // optional
    using reference = T&; // non-void only
    using const_reference = const T&; // non-void only
    using size_type = std::size_t;           // optional - non-void only
    using difference_type = std::ptrdiff_t;  // optional - non-void only
    // Allocator< T > -> Allocator< U >
    template<typename U>                     // optional
    struct rebind {
        using other = Allocator<U>;
    };
    //C++14
    using propagate_on_container_copy_assignment = std::true_type; // optional
    using propagate_on_container_move_assignment = std::true_type; //optional
    using propagate_on_container_swap = std::true_type; //optional
    //C++17
    using is_always_equal = std::true_type;
public :
    explicit Allocator() { }

    ~Allocator() { }

    explicit Allocator(Allocator const &) { }

    template<typename U>
    explicit Allocator(Allocator<U> const &) { }

    //implemented by std::allocator
    pointer address(reference r) { return &r; }
    //implemented by std::allocator
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
    void construct(pointer p, const T &t) { new (p) T(t); }

    // optional - destruction
    void destroy(pointer p) { p->~T(); }

    bool operator==(Allocator const &) { return true; }

    bool operator!=(Allocator const &a) { return !operator==(a); }
};

#include <iostream>
template < typename T >
struct MinimalAllocator {
    using value_type = T;
    T* allocate(size_t n) {
        std::cout << "\tallocate(" << n << ")\n";
        return reinterpret_cast< T* >(::operator new(n * sizeof(T)));
    }
    void deallocate(T* p, size_t n) {
        std::cout << "\tdeallocate(" << n << ")\n";
        delete(p);
    }
};



#include <string>
#include <vector>
#include <thread>
#include <chrono>
using namespace std;

int main(int, char **) {

    //allocator
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

    //minimal allocator with std::vector
    //in C++ >= 11 access to allocator happens through std::allocator_traits
    {
        cout << "\n";
        vector<int, MinimalAllocator<int >> vm;
        cout << "vector::assign\n";
        this_thread::sleep_for(5s);
        vm = {1, 2, 3};
        cout << "vector::push_back\n";
        vm.push_back(4);
        cout << "vector::push_back\n";
        vm.push_back(5);
        cout << "vector::pop_back\n";
        vm.pop_back();
        cout << "vector::shrink_to_fit\n";
        vm.shrink_to_fit();
        cout << "vector::~vector\n";
    }
// clang version: Apple LLVM version 6.0 (clang-600.0.57) (based on LLVM 3.5svn)
// vector::assign
//            allocate(3)
//    vector::push_back
//            allocate(6)
//    deallocate(3)
//    vector::push_back
//            vector::pop_back
//    vector::shrink_to_fit
//            allocate(4)
//    deallocate(6)
//    vector::~vector
//            deallocate(4)

    return 0;
}

