#include <functional>
#include <iostream>
#include <memory> //unique_ptr etc.
#include <cassert>
#include <stdexcept>

typedef const std::type_info* type_t;

#ifdef CHECK_TYPES
#ifndef CUSTOM_TYPE_CHECKER
#ifdef check_type
#error "check_type already #defined!\n"
#endif
#define check_type(T, t) { \
    if(&typeid(T) != t) { \
        std::cerr << __FILE__ << " BAD CAST line " << __LINE__ << std::endl; \
        std::cerr << typeid(T).name() << " TO " << t->name() << std::endl; \
        throw std::bad_cast(); \
    } \
}
#endif
#else
#define check_type(T, t)
#endif

//------------------------------------------------------------------------------
class data_t {
public:        
    template < typename T >
    data_t(const T& d) : type_(&typeid(T)), data_(new data_impl_t< T >(d)) {}
    data_t(const data_t& d) : type_(d.type_), data_(d.data_->copy()) {}
    data_t(data_t&&) = default;
    data_t& operator=(data_t d) {
        type_ = d.type_;
        data_ = std::move(d.data_);
        return *this;
    }
    template < typename T >
    operator T() const {
        return get< T >();
    }
    template< typename T >
    const T& get() const {
        check_type(T, type_);
        return data_->get< T >();
    }
    template< typename T >
    void set(const T& d) {
        check_type(T, type_);
        data_->set(d);
    }
private:    
    struct i_data_t {
        virtual i_data_t* copy() const = 0;
        template < typename T >
        const T& get() const {
            return static_cast< const data_impl_t< T >& >(*this).get();
        }
        template < typename T >
        void set(const T& d) {
            static_cast< data_impl_t< T >& >(*this).set(d);    
        }
    };
    template < typename T >
    struct data_impl_t : i_data_t {
        data_impl_t(const T& d) : data_(d) {}
        i_data_t* copy() const {
            return new data_impl_t< T >(*this);
        }
        const T& get() const {
            return data_;
        }
        void set(const T& d) {
            data_ = d;
        }
        T data_;
    };
private:         
    std::unique_ptr< i_data_t > data_;
    type_t type_;
};

int f(int i) {
    return i;
}

void data_t_test() {
    data_t d(2);
    assert(d.get< int >() == 2);
    assert(f(d) == 2);
    d.set(3);
    assert(int(d) == 3);
    try {
        float f = float(d);
        assert(false);
    } catch(...) {
        
    } 
}
//------------------------------------------------------------------------------
int main(int, char**) {
    data_t_test();
    return 0;
}