//Author: Ugo Varetto
//Implementation of task-based concurrency (similar to Java's Executor)
//and wrapper for concurrent access
//gcc >= 4.8 or clang llvm >= 3.2 with libc++ required
//
//do specify -pthread when compiling if not you'll get a run-time error
//g++ executor.cpp -std=c++11 -pthread 

#include <iostream>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <future>
#include <type_traits>
#include <utility>
#include <deque>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <map>
#include <cstdlib> //EXIT_*

//------------------------------------------------------------------------------
//synchronized queue (could be an inner class inside Executor):
// - acquire lock on insertion and notify after insertion
// - on extraction: acquire lock then if queue empty wait for notify, extract
//   element
template < typename T >
class SyncQueue {
public:
    void Push(const T& e) {
        //simple scoped lock: acquire mutex in constructor,
        //release in destructor
        std::lock_guard< std::mutex > guard(mutex_);
        queue_.push_front(e);
        cond_.notify_one(); //notify 
    }
    T Pop() {
        //cannot use simple scoped lock here because lock passed to
        //wait must be able to acquire and release the mutex 
        std::unique_lock< std::mutex > lock(mutex_);
        //stop and wait for notification if condition is false;
        //continue otherwise
        cond_.wait(lock, [this]{return !queue_.empty();});
        T e = queue_.back();
        queue_.pop_back();
        return e;
    }
    friend class Executor; //to allow calls to Clear  
private:
    void Clear() { queue_.clear(); }    
private:
    std::deque< T > queue_;
    std::mutex mutex_;
    std::condition_variable cond_;
};

//------------------------------------------------------------------------------
//interface and base class for callable objects 
struct ICaller {    
    virtual bool Empty() const = 0;    
    virtual void Invoke() = 0;    
    virtual ~ICaller() {}
};

//callable object stored in queue shared among threads: parameters are
//bound at object construction time
template < typename ResultType >
class Caller : public ICaller {
public:
    template < typename F, typename... Args >    
    Caller(F&& f, Args...args) : 
        f_(std::bind(std::forward<F>(f),
                     std::forward<Args>(args)...)),
        empty_(false) {}
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

//specialization for void return type
template <>
class Caller<void> : public ICaller {
public:
    template < typename F, typename... Args >    
    Caller(F f, Args...args) : f_(std::bind(f, args...)), empty_(false) {}
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

//------------------------------------------------------------------------------
//task executor: asynchronously execute callable objects. Specify the max number
//of threads to use at Executor construction time; threads are started in
//the constructor and joined in the destructor
class Executor {
    typedef SyncQueue< ICaller* > Queue;
    typedef std::vector< std::thread > Threads;
public:
    Executor(int numthreads = std::thread::hardware_concurrency()) 
        : nthreads_(numthreads) {
        StartThreads();    
    }
#ifdef USE_RESULT_OF    
    //deferred call to f with args parameters
    //1. all the arguments are bound to a function object taking zero parameters
    //   which is put into the shared queue
    //2. std::future is returned
    template < typename F, typename... Args > 
    auto operator()(F&& f, Args... args)
    -> std::future< typename std::result_of< F (Args...) >::type > {    
        if(threads_.empty()) throw std::logic_error("No active threads");
        typedef typename std::result_of< F (Args...) >::type ResultType; 
        Caller< ResultType >* c = 
            new Caller< ResultType >(std::forward< F >(f),
                                     std::forward< Args >(args)...);
        std::future< ResultType > ft = c->GetFuture();
        queue_.Push(c);
        return ft;
    }
#else    
    template < typename F, typename... Args > 
    auto operator()(F&& f, Args... args)
    -> std::future< decltype(f(args...)) > {    
        if(threads_.empty()) throw std::logic_error("No active threads");
        typedef decltype(f(args...)) ResultType; 
        Caller< ResultType >* c = 
            new Caller< ResultType >(std::forward< F >(f),
                                     std::forward< Args >(args)...);
        std::future< ResultType > ft = c->GetFuture();
        queue_.Push(c);
        return ft;
    }
#endif    
    //stop and join all threads; queue is cleared by default call with 
    //false to avoid clearing queue
    //to "stop" threads an empty  Caller instance per-thread is put into the
    //queue; threads interpret an empty Caller as as stop signal and exit from
    //the execution loop as soon as one is popped from the queue
    //Note: this is the only safe way to terminate threads, other options like
    //invoking explicit terminate functions where available are similar
    //to killing a process with Ctrl-C, since however threads are not
    //processes the resources allocated/acquired during the thread lifetime
    //are not automatically released
    void Stop(bool clearQueue = true) { //blocking
        for(int t = 0; t != threads_.size(); ++t) queue_.Push(new Caller<void>); 
        std::for_each(threads_.begin(), threads_.end(), [](std::thread& t)
                                                            {t.join();});
        threads_.clear();
        queue_.Clear();
    }
    //start or re-start with numthreads threads, queue is cleared by default
    //call with ..., false to avoid clearing queue
    void Start(int numthreads, bool clearQueue = true) { //non-blocking
        if(numthreads < 1) {
            throw std::range_error("Number of threads < 1");
        }
        Stop(clearQueue);
        nthreads_ = numthreads;
        StartThreads();
    }
    //same as Start; in case the Executor is created with zero threads
    //it makes sense to call start; if it's created with a number of threads
    //greater than zero call Restart in client code
    void Restart(int numthreads, bool clearQueue = true) {
        Start(numthreads, clearQueue); 
    }
    //join all threads
    ~Executor() { Stop(); }
private:
    //start threads and put them into thread vector
    void StartThreads() {
        for(int t = 0; t != nthreads_; ++t) {
            threads_.push_back(std::move(std::thread( [this] {
                while(true) {
                    ICaller* c = queue_.Pop();
                    if(c->Empty()) { //interpret an empty Caller as a
                                     //'terminate' message
                        break;
                    }
                    c->Invoke(); 
                    delete c;
                }
            })));    
        }
    }        
private:
    int nthreads_; //number of OS threads requested
    Queue queue_;  //command queue
    Threads threads_; //std::thread array; size == nthreads_ 
};

//------------------------------------------------------------------------------
template < typename T >
class ConcurrentAccess {
    T data_; //warning 'mutable' cannot be applied to references
    bool done_ = false;
    SyncQueue< std::function< void () > > queue_;
    std::future< void > f_;
public:
    ConcurrentAccess(T data, Executor& e) : data_(data), 
        f_(e([=]{
                while(!done_) queue_.Pop()();
             }))
    {}
    template < typename F >
    auto operator()(F f) 
    -> std::future< decltype(f(data_)) > {
        using R = decltype(f(data_));
        auto p = std::make_shared< std::promise< R > >(std::promise< R >());
        auto ft = p->get_future();
        queue_.Push([=]{
            try {
                p->set_value(f(data_));
            } catch(...) {
                p->set_exception(std::current_exception());
            }
        });
        return ft;
    }
    ~ConcurrentAccess() {
        queue_.Push([=]{done_ = true;});
        f_.wait();
    }
};

//------------------------------------------------------------------------------
int main(int argc, char** argv) {
    try {
        if(argc > 1 && std::string(argv[1]) == "-h") {
            std::cout << argv[0] << "[task sleep time (ms)] "
                                 << "[number of tasks] "
                                 << "[number of threads]\n"
                                 << "default is (0,20,4)\n";
            return 0;                      
        }

        const int sleeptime_ms = argc > 1 ? atoi(argv[1]) : 0;
        const int numtasks = argc > 2 ? atoi(argv[2]) : 4;
        const int numthreads = argc > 3 ? atoi(argv[3]) : 2;
        std::cout << "Running tasks...\n";
        std::cout << "Run-time configuration:\n"
                  << "  " << numtasks     << " tasks\n"
                  << "  " << numthreads   << " threads\n"
                  << "  " << sleeptime_ms << " ms task sleep time\n"
                  << std::endl;
        using namespace std;
        Executor exec(numthreads);
        Executor exec_aux(1);
        string msg = "start\n";
        //if single threaded use a different executor for concurrent access
        //if not it deadlocks
        ConcurrentAccess< string& > text(msg, numthreads > 1 ? exec : exec_aux);
        vector< future< void > > v;
        for(int i = 0; i != numtasks; ++i)
            v.push_back(exec([&, i]{
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(sleeptime_ms));
                text([=](string& s) {
                    s += to_string(i) + " " + to_string(i);
                    s += "\n";
                    return 0; //<- !!! Need a return value since void return
                              //   type not supported yet
                });
                text([](const string& s) {
                    cout << s;
                    return 0; //<- !!! Need a return value since void return
                              //   type not supported yet
                });
            }));
        for(auto& f: v) f.wait();
        std::cout << "Done\n";
        return 0;
    } catch(const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }    
    return 0;    
}

