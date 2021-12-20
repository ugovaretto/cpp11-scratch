#include <chrono>
#include <exception>
#include <iostream>
#include <thread>
#include <tuple>
#include <variant>

#include "coroutines.h"

using namespace std;
using namespace chrono;
using namespace CORO;

//------------------------------------------------------------------------------
int lineIdxG = 1;
struct LineNo {};

std::ostream& operator<<(std::ostream& os, LineNo) {
  cout << lineIdxG++ << ")\t";
  return os;
}
//------------------------------------------------------------------------------
// to co_await on a Task it needs to implement to awaitable interface
template <typename T>
class [[nodiscard]] Task {
  struct Promise {
    std::variant<std::monostate, T, std::exception_ptr> result_;
    // type erased handle, can hold any coroutine type
    CORO::coroutine_handle<> continuation_;
    auto get_return_object() noexcept { return Task{*this}; }
    void return_value(T value) {
      result_.template emplace<1>(std::move(value));
    }
    void unhandled_exception() noexcept {
      result_.template emplace<2>(std::current_exception());
    }
    auto initial_suspend() {
      cout << LineNo() << "Task<T>::Promise::initial_suspend()" << endl;
      ;
      return suspend_always{};
    }
    // when final_suspend returns an awaiter with a and await_suspend
    // returning a coroutine handle the coroutine is automatically resumed;
    // if no coroutine handle is valid use noop_coroutine() to return a
    // valid no-op coroutine handle
    auto final_suspend() noexcept {
      cout << LineNo() << "Task<T>::Promise::final_suspend()" << endl;
      struct Awaitable {
        bool await_ready() noexcept { return false; }
        auto await_suspend(CORO::coroutine_handle<Promise> h) noexcept {
          cout << LineNo() << "Task<T>::Promise::Awaitable::await_suspend()"
               << endl;
          return h.promise().continuation_;
        }
        void await_resume() noexcept {
          cout << LineNo() << "Task<T>::Promise::Awaitable::await_resume()"
               << endl;
        }
      };
      return Awaitable{};
    }
  };

  CORO::coroutine_handle<Promise> h_;
  explicit Task(Promise& p) noexcept
      : h_{CORO::coroutine_handle<Promise>::from_promise(p)} {}

 public:
  using promise_type = Promise;
  Task(Task&& t) noexcept : h_{exchange(t.h_, {})} {}
  ~Task() {
    cout << LineNo() << "Task<T>::~Task(), coroutine done? " << boolalpha
         << h_.done() << endl;
    if (h_ && h_.done()) h_.destroy();
  }
  // Awaitable interface
  bool await_ready() {
    cout << LineNo() << "Task<T>::await_ready()" << endl;
    return false;
  }
  auto await_suspend(CORO::coroutine_handle<> c) {
    cout << LineNo() << "Task<T>::await_suspend()" << endl;
    h_.promise().continuation_ = c;
    return h_;
  }
  auto await_resume() -> T {
    cout << LineNo() << "Task<T>::final_suspend()::await_resume()" << endl;
    auto& result = h_.promise().result_;
    if (result.index() == 1) {
      return std::get<1>(std::move(result));
    } else {
      std::rethrow_exception(std::get<2>(std::move(result)));
    }
  }
  void Start() {
    cout << LineNo() << "Task<T>::Start() --> coroutine_handle::resume()"
         << endl;
    h_.resume();
  }
};

//------------------------------------------------------------------------------
auto Exec(int m) {
  struct Awaitable {
    int data_{};
    int multiplier_{1};
    bool await_ready() const noexcept {
      cout << LineNo() << "Exec::Awaitable::await_ready()" << endl;
      return false;
    }
    void await_suspend(CORO::coroutine_handle<> h) {
      cout << LineNo() << "Exec::Awaitable::await_suspend()" << endl;
      this_thread::sleep_for(2s);
      data_ = multiplier_ * 10;
    }
    int await_resume() const noexcept {
      cout << LineNo() << "Exec::Awaitable::await_suspend()" << endl;
      return data_;
    }
    Awaitable(int m) : multiplier_{m} {}
  };
  return Awaitable{m};
}

//------------------------------------------------------------------------------
Task<int> Launch(int i) { co_return co_await Exec(i); }

//------------------------------------------------------------------------------
template <typename T>
struct WaitTask {
  struct Promise {
    T value_{};
    auto get_return_object() noexcept { return WaitTask{*this}; }
    void return_value(const T& v) { value_ = v; }
    void unhandled_exception() noexcept {
      rethrow_exception(current_exception());
    }
    auto initial_suspend() {
      cout << LineNo() << "WaitTask<T>::initial_suspend()" << endl;
      return suspend_never{};
    }
    auto final_suspend() noexcept {
      cout << LineNo() << "WaitTask<T>::final_suspend()" << endl;
      return suspend_never{};
    }
  };
  coroutine_handle<Promise> h_;
  WaitTask(Promise& p) : h_(coroutine_handle<Promise>::from_promise(p)) {}
  using promise_type = Promise;
  const T& Get() const { return h_.promise().value_; }
};

//------------------------------------------------------------------------------
template <>
class WaitTask<void> {
  struct Promise {
    auto get_return_object() noexcept { return WaitTask{*this}; }
    void return_void() {}
    void unhandled_exception() noexcept {
      rethrow_exception(current_exception());
    }
    auto initial_suspend() { return suspend_never{}; }
    auto final_suspend() noexcept { return suspend_always{}; }
  };

 public:
  using promise_type = Promise;
  coroutine_handle<Promise> h_;
  WaitTask(Promise& p) : h_(coroutine_handle<Promise>::from_promise(p)) {}
};

//------------------------------------------------------------------------------
template <typename T, template <typename> typename TaskT>
WaitTask<T> WaitForTask(TaskT<T>& t) {
  co_return co_await t;
}

//------------------------------------------------------------------------------
template <typename VoidTaskT>
WaitTask<void> WaitForTask(VoidTaskT& t) {
  co_await t;
}

//------------------------------------------------------------------------------
int main(int, const char**) {
  cout << LineNo() << "start" << endl;
  auto task = Launch(555);
  task.Start();
  auto ret = WaitForTask(task);
  cout << LineNo() << "Task returned: " << ret.Get() << endl;
  cout << LineNo() << "finish" << endl;
  return 0;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// template <>
// class [[nodiscard]] Task<void> {
//   struct Promise {
//     std::exception_ptr e_;   // No std::variant, only exception
//     std::coroutine_handle<> continuation_;
//     auto get_return_object() noexcept { return Task{*this}; }
//     void return_void() {}   // Instead of return_value()
//     void unhandled_exception() noexcept {
//       e_ = std::current_exception();
//     }
//     auto initial_suspend() { return CORO::suspend_always{}; }
//     auto final_suspend() noexcept {
//       struct Awaitable {
//         bool await_ready() noexcept { return false; }
//         auto await_suspend(CORO::coroutine_handle<Promise> h) noexcept {
//           return h.promise().continuation_;
//         }
//         void await_resume() noexcept {}
//       };
//       return Awaitable{};
//     }
//   };
//   std::coroutine_handle<Promise> h_;
//   explicit Task(Promise& p) noexcept
//       : h_{std::coroutine_handle<Promise>::from_promise(p)} {}
// public:
//   using promise_type = Promise;

//   Task(Task&& t) noexcept : h_{std::exchange(t.h_, {})} {}
//   ~Task() { if (h_ && h_.done()) h_.destroy(); }
//   // Awaitable interface
//   bool await_ready() { return false; }
//   auto await_suspend(std::coroutine_handle<> c) {
//     h_.promise().continuation_ = c;
//     return h_;
//   }
//   void await_resume() {
//     if (h_.promise().e_)
//       std::rethrow_exception(h_.promise().e_);
//   }
// };