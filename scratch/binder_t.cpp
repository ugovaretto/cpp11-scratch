#include <functional>
#include <iostream>
#include <memory> //unique_ptr etc.
#include <cassert>
#include <stdexcept>
#include <vector>
#include <type_traits> //remove_reference

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
    data_t() {}
    template < typename T >
    data_t(const T& d) : type_(&typeid(T)), data_(new data_impl_t< T >(d)) {}
    data_t(const data_t& d) : type_(d.type_), data_(d.data_->copy()) {}
    data_t(data_t&&) = default;
    data_t& operator=(data_t d) {
        type_ = d.type_;
        data_ = std::move(d.data_);
        return *this;
    }
    bool empty() const {
        return !bool(data_);
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
    T& get() {
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
        T& get() {
            return static_cast< data_impl_t< T >& >(*this).get();
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
        T& get() {
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

void test_data() {
    data_t empty;
    assert(empty.empty());
    data_t d(2);
    assert(d.get< int >() == 2);
    assert(f(d) == 2);
    d.set(3);
    assert(int(d) == 3);
    try {
        float f = float(d);
    } catch(...) {
        std::cout << "data_t OK\n" << std::endl;
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

//got the following code form stack overflow: TODO: find author and give credit
//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------
void test_action() {
    action_t a = make_function([](int i){return 2 * i;});
    assert(a.exec<int>(4) == 8);
    std::cout << "action_t OK\n" << std::endl;  
}  

//==============================================================================
//------------------------------------------------------------------------------
template<int ...> struct integer_sequence {};
//first int parameter is a counter, second onwards are used to create the
//specialized type integer_sequence<0, 1, 2...> 
//at each inheritance step the counter is decremented and the new counter
//value is added to the integer sequence
template<int N, int ...Is> 
struct make_integer_sequence : make_integer_sequence< N - 1, N - 1, Is... > {};
template< int ...Is > 
struct make_integer_sequence< 0, Is... > { 
    typedef integer_sequence< Is... > type; 
};

struct binder_t {
    typedef std::reference_wrapper< data_t > dataout_t;
    typedef std::vector< std::reference_wrapper< const data_t > > datain_t;
    typedef std::reference_wrapper< action_t > action_ref_t;
    template < typename RetT, typename...Args >
    binder_t(dataout_t out, const datain_t& in,
             std::reference_wrapper< action_t > action, const RetT*,
                                                        const Args*... ) : 
       
       caller_(new caller_t< RetT, Args...>(std::move(out), in, 
               std::move(action)))
    {}
    binder_t(const binder_t&) = default;
    binder_t(binder_t&&) = default;
    void exec() {
        caller_->exec();
    }
    struct i_caller_t {
        virtual void exec() = 0;
    };

    template < typename RetT, typename...Args>
    struct caller_t : i_caller_t {
        caller_t(dataout_t&& out, const datain_t& in, action_ref_t&& action) 
            : out_(out), in_(std::cref(in)), action_ref_(action) {}
        void exec() override {
            exec_impl(typename make_integer_sequence< sizeof...(Args)>::type());
        }
        template < int...Is >
        void exec_impl(integer_sequence<Is...>) {
            out_.get().template get<RetT>() = 
                action_ref_.get().template exec< RetT >
                (in_.get()[Is].get().template get<Args>()...);
        }
        dataout_t out_;
        std::reference_wrapper< const datain_t > in_;
        action_ref_t action_ref_;
    };

    std::unique_ptr< i_caller_t > caller_;
};

template< typename RetT, typename...Args >
binder_t make_binder(binder_t::dataout_t out, const binder_t::datain_t& in, 
                     binder_t::action_ref_t action) {
    return binder_t(out, in, action, 
                    (typename std::remove_reference< RetT >::type*) nullptr, 
                    (typename std::remove_reference< Args >::type*) nullptr...);
} 

void test_binder() {
    const data_t in1 = 2;
    const data_t in2 = 5;
    data_t out = int();
    action_t square = make_function([](int i, int j){return i * j;});
    std::vector< std::reference_wrapper< const data_t > > v = 
        {std::cref(in1), std::cref(in2)};
    binder_t binder((make_binder<int, int, int>(
        std::ref(out), v, std::ref(square))));
    binder.exec();
    assert(int(out) == 10);
    std::cout << "binder_t OK\n" << std::endl;
}

//==============================================================================    
int main(int, char**) {
    test_data();
    test_action();
    test_binder();
    return 0;
}