#include <iostream>
#include "coroutines.h"
#include <string>

using namespace CORO;

struct LifeTimeInspector {
  LifeTimeInspector(const std::string& m) : s(m) {
    std::cout << "Begin " << s << std::endl;
  }
  ~LifeTimeInspector() {
    std::cout << "End " << s << std::endl;
  }
  std::string s;
};


struct HelloCoroutine {
  struct HelloPromise {
    int value_{};
    HelloCoroutine get_return_object() {
      return coroutine_handle<HelloPromise>::from_promise(*this);
    }
    suspend_never initial_suspend() {return {};}
    suspend_always final_suspend() noexcept {return {};}
    void return_value(int value) { 
      std::cout << "got " << value << "\n";
      value_ = value;
    }
    void unhandled_exception() {}
  };

  using promise_type = HelloPromise;
  HelloCoroutine(coroutine_handle<HelloPromise> h) : handle(h) {}
  coroutine_handle<HelloPromise> handle;
  int operator()() const { return handle.promise().value_; }
};

HelloCoroutine count_to_ten() {
  LifeTimeInspector l("count_to_ten");
  for(int i = 0; i != 10; ++i) {
    if(i == 5) {
      std::cout << "Stopping..." << std::endl; 
      co_await suspend_always{};
      std::cout << "Resumed" << std::endl;
    }
    std::cout << i << std::endl;
  }
  co_return 42;
}


int main(int argc, const char** argv) {
    LifeTimeInspector i("main");
    HelloCoroutine mycoro = count_to_ten();
    std::cout << "calling resume" << std::endl;
    mycoro.handle.resume();
    std::cout << "value: " << mycoro() << std::endl;
    std::cout << "destroying stack frame" << std::endl;
    mycoro.handle.destroy();
    return 0;
}