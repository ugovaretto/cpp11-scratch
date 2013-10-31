#include <functional>
#include <iostream>
#include <memory> //unique_ptr etc.
#include <cassert>
#include <stdexcept>

//==============================================================================
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

//==============================================================================
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


//==============================================================================
class action_t {
public:
    template < typename F >
    action_t(const F& f) : 
        type_(&typeid(F)), action_(new action_impl_t< F >(f)) {}
    template < typename F >
    action_t(F&& f) : 
        type_(&typeid(F)), action_(new action_impl_t< F >(f)) {}    
    action_t(const action_t& a) : type_(a.type_), action_(a.action_->copy()) {}
    action_t(action_t&&) = default;
    action_t& operator=(action_t a) {
        type_ = a.type_;
        action_ = std::move(a.action_);
        return *this;
    }
    template < typename RetT, typename...Args >
    RetT exec(Args...args) {
        check_type(std::function<RetT (Args...)>, type_);
        return action_->template exec< RetT >(args...);
    }
private:    
    struct i_action_t {
        template < typename RetT, typename...Args >
        RetT exec(Args...args) {
            return static_cast< action_impl_t< std::function< 
                RetT (Args...) > >& >(*this).template exec< RetT >(args...); 
        }
        virtual i_action_t* copy() const = 0; 
    }; 
    
    template < typename F >
    struct action_impl_t final : i_action_t {
        action_impl_t(const F& f) : f_(f) {}
        action_impl_t(F&& f) : f_(std::move(f)) {}
        template < typename RetT, typename...Args >
        RetT exec(Args...args) {
            return f_(args...);
        }
        i_action_t* copy() const override {
            return new action_impl_t(*this);
        }
        F f_;
    };
    type_t type_;
    std::unique_ptr< i_action_t > action_;
};

template < typename LambdaT >
auto lambda(LambdaT l) -> decltype(l) {
    return l;
}

// struct F {
//     void operator()(int i) {}
// };

// void foo(int i){}
int foo() { return 1;}
// template<typename T>
// std::function<T> make_function(T *t) {
//   return { t };
// }

struct F {
    int operator()() {return 1;}
};

struct A {
    operator int() { return Intwrapper(this); }
    struct Intwrapper {
        Intwrapper(A* a) {}
        operator int() { return 2;}
    };
};
template<typename T> struct remove_class { };
template<typename C, typename R, typename... A>
struct remove_class<R(C::*)(A...)> { using type = R(A...); };
template<typename C, typename R, typename... A>
struct remove_class<R(C::*)(A...) const> { using type = R(A...); };
template<typename C, typename R, typename... A>
struct remove_class<R(C::*)(A...) volatile> { using type = R(A...); };
template<typename C, typename R, typename... A>
struct remove_class<R(C::*)(A...) const volatile> { using type = R(A...); };

template<typename T>
struct get_signature_impl { using type = typename remove_class<
    decltype(&std::remove_reference<T>::type::operator())>::type; };
template<typename R, typename... A>
struct get_signature_impl<R(A...)> { using type = R(A...); };
template<typename R, typename... A>
struct get_signature_impl<R(&)(A...)> { using type = R(A...); };
template<typename R, typename... A>
struct get_signature_impl<R(*)(A...)> { using type = R(A...); };
template<typename T> using get_signature = typename get_signature_impl<T>::type;

template<typename F> using make_function_type = std::function<get_signature<F>>;
template<typename F> make_function_type<F> make_function(F &&f) {
    return make_function_type<F>(std::forward<F>(f)); }
//==============================================================================
int main(int, char**) {

    //auto f = make_function(F());
    action_t a = make_function([]{return 1;});
    std::cout << a.exec<int>();
   
    // int b = a.do<int>();
    return 0;
}