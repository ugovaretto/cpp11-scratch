#include <iostream>
#include <coroutine>
#include <string>


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
    HelloCoroutine get_return_object() {
      return std::coroutine_handle<HelloPromise>::from_promise(*this);
    }
    std::suspend_never initial_suspend() {return {};}
    std::suspend_always final_suspend() noexcept {return {};}
    void return_value(int value) { std::cout << "got " << value << "\n";}
    void unhandled_exception() {}
  };

  using promise_type = HelloPromise;
  HelloCoroutine(std::coroutine_handle<HelloPromise> h) : handle(h) {}
  std::coroutine_handle<HelloPromise> handle;
};

HelloCoroutine count_to_ten() {
  LifeTimeInspector l("count_to_ten");
  for(int i = 0; i != 10; ++i) {
    if(i == 5) {
      co_await std::suspend_always{};
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
    std::cout << "destroying stack frame" << std::endl;
    mycoro.handle.destroy();
    return 0;
}