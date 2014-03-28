//author: Ugo Varetto
//minimal C++11 implementation of timer and timer callback classes

#pragma once
#include <chrono>
#include <thread>
#include <memory>

//------------------------------------------------------------------------------
class Timer {    
public:
    using Tick = std::chrono::steady_clock::rep;
    using Period = std::chrono::steady_clock::period;
    using DurationType = std::chrono::steady_clock::duration;
    using TimePoint = std::chrono::steady_clock::time_point;
public:
    void Start() {
        stopped_ = false;
        start_ = std::chrono::steady_clock::now();
    }
    TimePoint Stop() { 
        ticks_ = std::chrono::steady_clock::now();
        stopped_ = true;
        return ticks_;
    }
    template < typename RepT, typename PeriodT >
    RepT Elapsed() const {
         t_ = std::chrono::steady_clock::now();
         if(stopped_) t_ = ticks_;
         return 
             std::chrono::duration_cast< 
                 std::chrono::duration< RepT, PeriodT > >(t_ - start_).count();
    }
    Tick ns() const {
        return Elapsed< Tick, std::nano >();   
    }
    Tick us() const {
        return Elapsed< Tick, std::micro >();   
    }
    Tick ms() const {
        return Elapsed< Tick, std::milli >();   
    }
    Tick s() const {
        return Elapsed< Tick, std::ratio< 1, 1 > >();
    }
    Tick Ticks() const { 
        t_ = std::chrono::steady_clock::now();
        if(stopped_) t_ = ticks_;
        return (t_ - start_).count(); }
    DurationType Duration() const {
        t_ = std::chrono::steady_clock::now();
        if(stopped_) t_ = ticks_;
        return t_ - start_;
    }
private:
    TimePoint ticks_;
    TimePoint start_;
    bool stopped_ = true;
    mutable TimePoint t_;
};


//------------------------------------------------------------------------------
class TimerCallback {
public:
    template < typename F, typename C, typename...Args >
    void Start(double interval, F&& f, C&& c, Args...args) {
        std::chrono::steady_clock::time_point tp;
        auto tf = [=](Args...args) {
            while(c()) {
                std::this_thread::sleep_until(
                    std::chrono::steady_clock::now() 
                    + std::chrono::duration< 
                        double, std::ratio< 1, 1 > >(interval));
                f(args...);
            }
        };
        thread_.reset(new std::thread(tf, args...));
    }
    void WaitForCompletion() {
        thread_->join();
    }
    //note: all threads *must* be terminated before the process exits 
    //~TimerCallback() { Stop(); }
private:
    std::unique_ptr< std::thread > thread_;    
};


//------------------------------------------------------------------------------
inline void Sleep(double seconds) {
    std::this_thread::sleep_until(
        std::chrono::steady_clock::now() 
        + std::chrono::duration< double, std::ratio< 1, 1 > >(seconds));
}


//template < typename Unit >
//class ScopedTimer {
//public:
//    template < T >
//    ScopedTimer(T&& f) : f_(f) {}
//    ~ScopedTimer() {
//        f_(ToUnits< Unit > t.Elapsed< Unit >());
//    }
//private:
//    std::function<void (Unit)> f_;    
//};
//
//Clock {
//    //record sleep() call latency
//    void Run() {
//        while(!done)
//        record time
//        tick();
//        delta = current time - record time
//        if(delta < interval) sleep(interval - delta - sleep latency)
//        
//    }
//            
//} 
