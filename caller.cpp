#include <iostream>
#include <cmath>
#include <future>
#include <type_traits>
#include <utility>
#include <memory>

//------------------------------------------------------------------------------
//useful to derive from base class when putting instances inside
//e.g. a command queue for deferred execution: add to queue and return
//future then call Invoke when execution required

struct ICaller {
    virtual void Invoke() = 0;
    virtual ~ICaller() {}
};


template < typename ResultType >
class Caller : ICaller {
public:
    template < typename U, typename... Args >    
    Caller(U f, Args...args) : f_(std::bind(f, args...)), empty_(false) {}
    Caller() : empty_(true) {}
    std::future< ResultType > GetFuture() {
        return p_.get_future();
    }
    void Invoke() {
        try {
            ResultType r = ResultType(f_());
            p_.set_value(r);
        } catch(...) {
            p_.set_exception(std::current_exception());
        }
    }
    bool Empty() const { return empty_; }
private:
    std::promise< ResultType > p_;
    std::function< ResultType () > f_; 
    bool empty_;
}; 

template <>
class Caller<void> : ICaller {
public:
    template < typename U, typename... Args >    
    Caller(U f, Args...args) : f_(std::bind(f, args...)), empty_(false) {}
    Caller() : empty_(true) {}
    std::future< void > GetFuture() {
        return p_.get_future();
    }
    void Invoke() {
        try {
            f_();
            p_.set_value();
        } catch(...) {
            p_.set_exception(std::current_exception());
        }
    }
    bool Empty() const { return empty_; }
private:
    std::promise< void > p_;
    std::function< void () > f_; 
    bool empty_;
}; 



template <typename F, typename... Args >
auto MakeCaller(F f, Args... args) -> Caller< decltype(f(args...)) >* {
    return new Caller< decltype(f(args...)) >(f, args...);
}

//------------------------------------------------------------------------------
void foo() { std::cout << "foo\n";}

//consider returning std::unique_ptr from MakeCaller
int main(int, char**) {

    typedef std::remove_pointer< decltype(MakeCaller(sqrt, 2.0)) >::type C1;
    std::unique_ptr< C1 > caller(MakeCaller(sqrt, 2.0));
    auto f = caller->GetFuture();
    caller->Invoke();
    std::cout << f.get() << std::endl; 

    typedef std::remove_pointer< 
        decltype(MakeCaller(printf, "message %d\n", 1)) >::type C2;
    std::unique_ptr< C2 > printer(MakeCaller(printf, "message %d\n", 1));
    auto f2 = printer->GetFuture();
    printer->Invoke();
    f2.wait();

    typedef std::remove_pointer< decltype(MakeCaller(foo)) >::type C3;
    std::unique_ptr< C3 > v(MakeCaller(foo));
    auto f3 = v->GetFuture();
    v->Invoke();
    f3.wait();

    return 0;
}
