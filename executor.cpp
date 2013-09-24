//Author: Ugo Varetto
//Implementation of task-based concurrency (similar to Java's Executor)
//gcc >= 4.8 or clang llvm >= 3.2 with libc++ required
//specify -pthread if not you'll get a run-time error e.g.
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
template < typename T >
class SyncQueue {
public:
    void Push(const T& e) {
        std::lock_guard< std::mutex > guard(mutex_);
        queue_.push_front(e);
        cond_.notify_one();
    }
    T Pop() {
        std::unique_lock< std::mutex > lock(mutex_);
        cond_.wait(lock, [this]{ return !queue_.empty();});
        T e = queue_.back();
        queue_.pop_back();
        return e;
    }  
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
class Caller<void> : public ICaller {
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
    Executor(int numthreads = std::thread::hardware_concurrency()) 
        : nthreads_(numthreads) {
        StartThreads();    
    }
    template < typename F, typename... Args > 
    auto operator()(F f, Args... args) -> std::future< decltype(f(args...)) > {
        if(threads_.empty()) throw std::logic_error("No active threads");
        typedef decltype(f(args...)) ResultType;
        Caller< ResultType >* c = new Caller< ResultType >(f, args...);
        std::future< ResultType > ft = c->GetFuture();
        queue_.Push(c);
        return ft;
    }
    void Stop() { //blocking
        for(int t = 0; t != threads_.size(); ++t) queue_.Push(new Caller<void>); 
        std::for_each(threads_.begin(), threads_.end(), [](std::thread& t)
                                                            {t.join();});
        threads_.clear();
    }
    void Start(int numthreads) { //non-blocking
        if(numthreads < 1) {
            throw std::range_error("Number of threads < 1");
        }
        Stop();
        nthreads_ = numthreads;
        StartThreads();
    }
    void Restart(int nthreads) { Stop(); Start(nthreads); }
    ~Executor() { Stop(); }
private:
    void StartThreads() {
        for(int t = 0; t != nthreads_; ++t) {
            threads_.push_back(std::move(std::thread( [this] {
                while(true) {
                    ICaller* c = queue_.Pop();
                    if(c->Empty()) {
                        break;
                    }
                    c->Invoke(); 
                    delete c;
                }
            })));    
        }
    }        
private:
    int nthreads_;
    Queue queue_;
    Threads threads_;
};

//------------------------------------------------------------------------------
int sum(int start, int end, int step) {
    for(;start < end; start += step)
        ; //fixed "empty body" warning with clang
    return start;
}

int main(int argc, char** argv) {
    try {
        if(argc > 1 && std::string(argv[1]) == "-h") {
            std::cout << argv[0] << "[task sleep time (ms)] "
                                 << "[number of tasks] "
                                 << "[number of threads]\n"
                                 << "default is (0,20,4)\n";
            return 0;                      
        }
        //test Executor
        std::cout << "\nTesting Executor...";
        Executor exec(2);
        using namespace std::placeholders;
        auto f1 = exec(std::bind(sum, _1, _2, 1), 1, 10);
        auto f2 = exec(sum, 10, 100, 1);
        auto f3 = exec(sum, 100, 1000, 100);
        if(f1.get() != 10
           || f2.get() != 100
           || f3.get() != 1000) {
            std::cerr << "FAILED\n";
            return EXIT_FAILURE;
        }
        std::cout << "OK\n\n";
        //OK run tasks
        const int sleeptime_ms = argc > 1 ? atoi(argv[1]) : 0;
        const int numtasks = argc > 2 ? atoi(argv[2]) : 20;
        const int numthreads = argc > 3 ? atoi(argv[3]) : 4;
        std::cout << "Run-time configuration:\n"
                  << "  " << numtasks     << " tasks\n"
                  << "  " << numthreads   << " threads\n"
                  << "  " << sleeptime_ms << " ms task sleep time\n\n";
        std::mutex iomutex;
        std::map< std::thread::id, int > counter; 
        exec.Restart(numthreads);
        for(int t = 0; t != numtasks; ++t) {
            exec([&iomutex, &counter, &sleeptime_ms](int i) {
                {
                    std::lock_guard<std::mutex> lk(iomutex); //also serializes
                                                             //access to 
                                                             //counter
                    std::cout << "Hello from task: " << i << "\t("
                              << std::hex 
                              << std::this_thread::get_id() << ')'
                              << std::dec
                              << std::endl;
                    counter[std::this_thread::get_id()]++;
                }
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(sleeptime_ms));          
            }, t);
        }
        exec.Stop();
        std::cout << std::endl;
        typedef std::map< std::thread::id, int >::value_type CounterEntry;
        std::for_each(counter.begin(), counter.end(), 
                        [](const CounterEntry& e) {
                            std::cout << "Thread " 
                                      << std::hex << e.first << std::dec
                                      << " executed " << e.second 
                                      << " times\n";        
                        });
        std::cout << "\nRun with -h for info on usage options\n" << std::endl;
        return 0;
    } catch(const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }    
    return 0;    
}

//SAMPLE OUTPUT: no arguments
// Testing Executor...OK
//
// Run-time configuration:
//   20 tasks
//   4 threads
//   0 ms task sleep time
//
// Hello from task: 0  (7fd1533a2700)
// Hello from task: 1  (7fd1523a0700)
// Hello from task: 2  (7fd152ba1700)
// Hello from task: 5  (7fd1523a0700)
// Hello from task: 4  (7fd1533a2700)
// Hello from task: 3  (7fd151b9f700)
// Hello from task: 6  (7fd152ba1700)
// Hello from task: 8  (7fd1533a2700)
// Hello from task: 7  (7fd1523a0700)
// Hello from task: 9  (7fd152ba1700)
// Hello from task: 10 (7fd1533a2700)
// Hello from task: 12 (7fd151b9f700)
// Hello from task: 11 (7fd1523a0700)
// Hello from task: 13 (7fd152ba1700)
// Hello from task: 14 (7fd1533a2700)
// Hello from task: 15 (7fd151b9f700)
// Hello from task: 16 (7fd1523a0700)
// Hello from task: 17 (7fd152ba1700)
// Hello from task: 18 (7fd151b9f700)
// Hello from task: 19 (7fd1533a2700)
//
// Thread 7fd151b9f700 executed 4 times
// Thread 7fd1523a0700 executed 5 times
// Thread 7fd152ba1700 executed 5 times
// Thread 7fd1533a2700 executed 6 times
//
//Run with -h for info on usage options

//SAMPLE OUTPUT: ./a.out 30 17 10
// Testing Executor...OK
//
// Run-time configuration:
//   17 tasks
//   10 threads
//   30 ms task sleep time
//
// Hello from task: 0  (7fcd6fad4700)
// Hello from task: 2  (7fcd677fe700)
// Hello from task: 4  (7fcd6e2d1700)
// Hello from task: 3  (7fcd6ead2700)
// Hello from task: 1  (7fcd702d5700)
// Hello from task: 5  (7fcd6cace700)
// Hello from task: 6  (7fcd67fff700)
// Hello from task: 7  (7fcd6f2d3700)
// Hello from task: 8  (7fcd6d2cf700)
// Hello from task: 9  (7fcd6dad0700)
// Hello from task: 10 (7fcd6fad4700)
// Hello from task: 11 (7fcd677fe700)
// Hello from task: 12 (7fcd6ead2700)
// Hello from task: 13 (7fcd6e2d1700)
// Hello from task: 14 (7fcd702d5700)
// Hello from task: 15 (7fcd6cace700)
// Hello from task: 16 (7fcd67fff700)
//
// Thread 7fcd677fe700 executed 2 times
// Thread 7fcd67fff700 executed 2 times
// Thread 7fcd6cace700 executed 2 times
// Thread 7fcd6d2cf700 executed 1 times
// Thread 7fcd6dad0700 executed 1 times
// Thread 7fcd6e2d1700 executed 2 times
// Thread 7fcd6ead2700 executed 2 times
// Thread 7fcd6f2d3700 executed 1 times
// Thread 7fcd6fad4700 executed 2 times
// Thread 7fcd702d5700 executed 2 times
//
// Run with -h for info on usage options

