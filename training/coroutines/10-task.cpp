#include <iostream>
#include <tuple>
#include <variant>
#include <exception>
#include <chrono>
#include <thread>

#include "coroutines.h"

using namespace std;
using namespace chrono;

// to co_await on a Task it needs to implement to awaitable interface
template <typename T>
class [[nodiscard]] Task {
    struct Promise {
        std::variant<std::monostate, T, std::exception_ptr> result_;
        //type erased handle, can hold any coroutine type
        CORO::coroutine_handle<> continuation_;
        auto get_return_object() noexcept { return Task{*this}; }
        void return_value(T value) {
            result_.template emplace<1>(std::move(value));
        }
        void unhandled_exception() noexcept {
            result_.template emplace<2>(std::current_exception());
        }
        auto initial_suspend() { return std::suspend_always{};}
        auto final_suspend() noexcept {
            struct Awaitable {
                bool await_ready() noexcept { return false; }
                auto await_suspend(CORO::coroutine_handle<Promise> h) noexcept {
                    return h.promise().continuation_;
                }
                void await_resume() noexcept {}
            };
            return Awaitable{};
        }
    };
    
    CORO::coroutine_handle<Promise> h_;
    explicit Task(Promise& p) noexcept
        : h_{CORO::coroutine_handle<Promise>::from_promise(p)} {}

   public:
    using promise_type = Promise;
    Task(Task&& t) noexcept 
    : h_{exchange(t.h_, {})} {}
    ~Task() {
        if (h_) h_.destroy();
    }
    // Awaitable interface
    bool await_ready() { return false; }
    auto await_suspend(CORO::coroutine_handle<> c) {
        h_.promise().continuation_ = c;
        return h_;
    }
    auto await_resume() -> T {
        auto& result = h_.promise().result_;
        if (result.index() == 1) {
            return std::get<1>(std::move(result));
        } else {
            std::rethrow_exception(std::get<2>(std::move(result)));
        }
    }
    void Resume() {h_.resume();}
};


template <>
class [[nodiscard]] Task<void> {
  
  struct Promise {
    std::exception_ptr e_;   // No std::variant, only exception
    std::coroutine_handle<> continuation_; 
    auto get_return_object() noexcept { return Task{*this}; }
    void return_void() {}   // Instead of return_value() 
    void unhandled_exception() noexcept { 
      e_ = std::current_exception(); 
    }
    auto initial_suspend() { return CORO::suspend_always{}; }
    auto final_suspend() noexcept {
      struct Awaitable {
        bool await_ready() noexcept { return false; }
        auto await_suspend(CORO::coroutine_handle<Promise> h) noexcept {
          return h.promise().continuation_;
        }
        void await_resume() noexcept {}
      };
      return Awaitable{};
    }
  };
  std::coroutine_handle<Promise> h_;
  explicit Task(Promise& p) noexcept 
      : h_{std::coroutine_handle<Promise>::from_promise(p)} {}
public:
  using promise_type = Promise;
 
  Task(Task&& t) noexcept : h_{std::exchange(t.h_, {})} {}
  ~Task() { if (h_) h_.destroy(); }
  // Awaitable interface
  bool await_ready() { return false; }
  auto await_suspend(std::coroutine_handle<> c) {
    h_.promise().continuation_ = c;
    return h_;
  }
  void await_resume() {
    if (h_.promise().e_)
      std::rethrow_exception(h_.promise().e_);
  }
};


auto Exec() {
  struct Awaitable {
    int data{};
    bool await_ready() const noexcept { return false;}
    void await_suspend(CORO::coroutine_handle<> h ) {
      this_thread::sleep_for(2s);
      data = 10;
    }
    int await_resume() const noexcept {return data;}
  };
  return Awaitable{};
}

Task<int> Launch(int i) {
  auto x = co_await Exec();
  cout << x << endl;
  co_return 34;
}


int main(int, const char**) { 
  auto c = Launch(555);
  c.Resume();
  //c.Resume();
  //cout << get<1>(c.h_.promise().result_) << endl;
  return 0;
}
