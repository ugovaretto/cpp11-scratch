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

// [main]
// [Launch]
// 1) Call Launch() -> Task<T> as 'task', and wait at subroutine start
// [main]
// 2) Resume Launch:
//    2.1 execute Exec::Awaitable::await_ready() -> false
//    2.2 execute Exec::Awaitable::suspend and return execution to caller
// [WaitForTask]
// 3) Call WaitForTask(task):
//    3.1 Task<T>::await_ready() -> false
//    3.2 Task<T>::suspend() -> Launch coroutine handler as 'launch'
// [Launch]
// 4) Resume Launch: call Exec::Awaitable::await_resume() -> 5550
//    4.1 Task::Promise::return_value(5550)
//    4.2 Task::Promise::final_suspend() -> Task<T>::Promise::Awaitable() as
//    'awaitable' 4.3 awaitable.await_suspend() -> Launch 
//    4.4 Task::Promise::Awaitable::await_suspend() -> WaitForTask coroutine
// [WaitForTask]
// 5) Resume WaitForTask coroutine: WaitTask<T>::Promise::return_value(5550)
//    5.1 WaitTask<T>::final_suspend()
// [main]


// Offloading execution to another co-routine is achieved by returning
// coroutine_handle<> from an await_suspend() method: when a couroutine_handle()
// is returned it is interpreted as a continuation and ::resume() called
// immediately.

// Task execution is normally performed in the await_suspend() method.

// co_yield can be used instead of co_return for tasks

//------------------------------------------------------------------------------
int lineIdxG = 1;
struct LineNo {};

std::ostream& operator<<(std::ostream& os, LineNo) {
  cout << lineIdxG++ << ")\t";
  return os;
}
//------------------------------------------------------------------------------
// In order to co_await on a Task, the Task type needs to implement to
// awaitable interface
template <typename T>
class [[nodiscard]] Task {
  struct Promise {
    const char* name_ = "Task<T>::Promise";
    std::variant<std::monostate, T, std::exception_ptr> result_;
    // type erased handle, can hold any coroutine type
    coroutine_handle<> continuation_;
    auto get_return_object() noexcept { return Task{*this}; }
    void return_value(T value) {
      cout << LineNo() << "Task<T>::Promise::return_value" << endl;
      result_.template emplace<1>(std::move(value));
    }
    void unhandled_exception() noexcept {
      result_.template emplace<2>(std::current_exception());
    }
    auto initial_suspend() {
      cout << LineNo() << "Task<T>::Promise::initial_suspend()" << endl;
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
          // return coroutine (WaitForTask) to be resumed and invoke resume
          // on coroutine which in turn invoke Task<T>::resume
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
  // Task(Task&& t) noexcept : h_{exchange(t.h_, {})} {}
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
  auto await_suspend(coroutine_handle<> c) {
    cout << LineNo() << "Task<T>::await_suspend()" << endl;
    // record the coroutine (WaitForTask) to be resumed after coroutine (Launch)
    // returning this Task instance completes
    // returning Task<T> coroutine triggers an automatic call to
    //(Task<T> coroutine handle).resume()
    h_.promise().continuation_ = c;
    // return Launch (Task<T>) couroutine and automatically trigger resume on it
    return h_;
  }
  auto await_resume() -> T {
    cout << LineNo() << "Task<T>::await_resume()" << endl;
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
    void await_suspend(coroutine_handle<>) {
      cout << LineNo() << "Exec::Awaitable::await_suspend()" << endl;
      this_thread::sleep_for(2s);
      data_ = multiplier_ * 10;
    }
    int await_resume() const noexcept {
      cout << LineNo() << "Exec::Awaitable::await_resume()" << endl;
      return data_;
    }
    Awaitable(int m) : multiplier_{m} {}
  };
  return Awaitable{m};
}

//------------------------------------------------------------------------------
template <typename T>
struct WaitTask {
  struct Promise {
    T value_{};
    const char* name_ = "WaitTask::Promise";
    auto get_return_object() noexcept { return WaitTask{*this}; }
    void return_value(const T& v) {
      cout << LineNo() << "WaitTask<T>::Promise::return_value" << endl;
      value_ = v;
    }
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
template <typename T, template <typename> typename TaskT>
WaitTask<T> WaitForTask(TaskT<T>& t) {
  auto r = co_await t;
  cout << LineNo() << "WaitForTask() resumed" << endl;
  co_return r;
}

//------------------------------------------------------------------------------
Task<int> Launch(int i) {
  auto r = co_await Exec(i);
  cout << LineNo() << "Launch() resumed" << endl;
  co_return r;
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
