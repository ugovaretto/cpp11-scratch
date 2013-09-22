#include <iostream>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <future>
#include <type_traits>
#include <utility>
#include <deque>
#include <vector>

//------------------------------------------------------------------------------
template < typename T >
class 
SyncQueue {
public:
    void Push(const T& e) {
        std::lock_guard< std::mutex > guard(mutex_);
        queue_.push_front(e);
        cond_.notify_one();
        cond_.notify_one();
        cond_.notify_one();
    }
    T Pop() {
        std::unique_lock< std::mutex > lock(mutex_);
        cond_
        .wait(lock, [this]{ return !queue_.empty();});
        T e = std::move(queue_.back());
        if(Remove(e)) queue_.pop_back();
        return e;
    }  
private:
    template < typename E >
    bool Remove(const E
        & e) { return true; } 
private:
    std::deque< T > queue_;
    std::mutex mutex_;
    std::condition_variable cond_;
};

//------------------------------------------------------------------------------
struct ICaller {    
    virtual bool Empty() const = 0;    
    virtual void Invoke() = 0;    
    virtual ~ICaller() {}
};


template < typename ResultType >
class Caller : public ICaller {
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
class Caller<void> : public 
ICaller {
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
            threads_.push_back(std::thread( [this] {
                while(true) {
                    ICaller* c = queue_.Pop();
                    if(c->Empty()) {
                        std::cout << "terminating" << std::endl;
                        break;
                    }
                    c->Invoke();
                    delete c;
                }
            }));    
        }    
    }
    template < typename F, typename... Args > 
    auto operator()(F f, Args... args) -> std::future< decltype(f(args...)) >{
        typedef decltype(f(args...)) ResultType;
        Caller< ResultType >* c = new Caller< ResultType >(f, args...);
        std::future< ResultType > ft = c->GetFuture();
        queue_.Push(c);
        return ft;
    }
    void operator()() {
        queue_.Push(new Caller<void>);
        //std::for_each(threads_.begin(), threads_.end(), [](std::thread& t)
        //    {t.join();});
    }
private:
    Queue queue_;
    Threads threads_;
};

//------------------------------------------------------------------------------
int sum(int start, int end, int step = 1) {
    for(;start < end; start += step)
        ;

        return start;
}

int main(int, char**) {
    Executor exec(3);
    auto f1 = exec(sum, 1, 10, 1);
    auto f2 = exec(sum, 10, 100, 1);
    auto f3 = exec(sum, 100, 1000, 100);
    exec();
    std::cout << f1.get() << ' ' << f2.get() << ' ' << f3.get() << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(10));
    return 0;    
}
