#include <iostream>
#include <condition_variable>
#include <thread>
#include <chrono>
 
std::condition_variable cv;
std::mutex cv_m; // This mutex is used for three purposes:
                 // 1) to synchronize accesses to i
                 // 2) to synchronize accesses to std::cerr
                 // 3) for the condition variable cv
int i = 0;
 
void waits()
{
    std::unique_lock<std::mutex> lk(cv_m);
    std::cerr << "Waiting... \n";
std::this_thread::sleep_for(std::chrono::seconds(10));
    cv.wait(lk, []{return i == 1;});
    
    std::cerr << "...finished waiting. i == 1\n";

    
}
 
void signals()
{
       
    std::this_thread::sleep_for(std::chrono::seconds(1));
    {
        std::lock_guard<std::mutex> lk(cv_m);
        std::cerr << "Notifying...\n";
    }
    cv.notify_all();
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
     i = 1;
    {
        std::lock_guard<std::mutex> lk(cv_m);
        std::cerr << "Notifying again...\n";
    }
    cv.notify_all();
}

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
    bool Remove(const T& e) {
        //if used to signal thread termination return false
        return true;
    }   

private:
    std::deque< T > queue_;
    std::mutex mutex_;
    std::condition_variable cond_;
};

struct ICaller {    
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

class Executor {

};
 
int main()
{
    auto exec = MakeExecutor(4);
    std::future< int > exec<int>(counter, 10);
    std::future< int >
    std::thread t1(waits), t2(waits), t3(waits), t4(signals);
    t1.join(); 
    t2.join(); 
    t3.join();
    t4.join();
    return 0;
}
