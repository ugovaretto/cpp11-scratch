#include <iostream>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <future>
#include <type_traits>
#include <utility>
#include <deque>

//------------------------------------------------------------------------------
template < typename T >
SyncQueue {
public:
    void Push(T&& e) {
        std::lock_guard< std::mutex > guard(mutex_);
        queue_.push_front(std::move(e));
        cond.notify_one();
    }
    T Pop() {
        std::unique_lock< std::mutex > lock(mutex_);
        cond.wait(mutex_, [this]{ return !queue_.empty();});
        T e = std::move(queue_.back());
        if(Remove(e)) queue_.pop_back();
        return e;
    }  
private:
    template < typename T >
    bool Remove(const T& e) { return true; } 
private:
    std::deque< T > queue_;
    std::mutex mutex_;
    std::condition_variable cond_;
};

//------------------------------------------------------------------------------
struct ICaller {    
    virtual bool Empty() const = 0;    
    virtual void Invoke() = 0;    
    virtual ~Caller() {}
};


template < typename F  >
class Caller {
    typedef std::result_of< F >::type ResultType;
public:    
    Caller(F f, Args...args) : f_(std::bind(f, args)) {}
    void Invoke() {
        try {
            ResultType r = f_();
            p_.set_value(r);
        } catch(const std::exception& e) {
            p_.set_exception(e);
        }
    }
private:
    std::promise< ResultType > p_;
    std::function< ResultType () > f_; 
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

bool Remove(ICaller* c) { return !c->Empty(); }

//------------------------------------------------------------------------------
class Executor {
    typedef SyncQueue< ICaller* > Queue;
    typedef std::vector< std::thread > Threads;
public:
    Executor(int numthreads = std::thread::hardware_concurrency()) {
        for(int t = 0; t != numthreads; ++t) {
            threads.push_back(std::thread( [this] {
                while(true) {
                    ICaller* c = queue_.Pop();
                    if(c->Empty()) {
                        delete c;
                        break;
                    }
                    c->Invoke();
                    delete c;
                }
            });    
        }    
    }
    template < typename F, typename... Args > 
    auto operator()(F f, Args... args) -> std::future< decltype(f(args...)) >{
        typedef decltype(f(args...)) ResultType
        Caller< ResultType >* c = new Caller< ResultType >(f, args...);
        queue_.Push(c);
        return c->GetFuture()
    }
    void operator()() {
        queue_.Push(new Caller<void>);
    }
private:
    Queue queue_;
    Threads threads_;
};

//------------------------------------------------------------------------------
int sum(int start, int end, int step = 1) {
    int ret = start;
    for(;start != end + 1; ++start) {
        ret += ste;
    }
}
 
int main(int, char**) {
    Executor exec(3);
    auto f1 = exec(sum, 1, 10);
    auto f2 = exec(sum, 10, 100);
    auto f3 = exec(sum, 100, 1000, 100);
    exec();
    std::cout << f1.get() << ' ' << f2.get() << ' ' << f3.get() << std::endl;
    return 0;    
}
