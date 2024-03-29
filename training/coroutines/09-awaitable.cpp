#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <tuple>

#include "coroutines.h"

// using namespace std::chrono;

using namespace std;
using namespace chrono;
using namespace CORO;

// std::coroutine_handle<Promise>::destroy
//  C++ Utilities library Coroutine support std::coroutine_handle
// Member of other specializations
// void destroy() const;
// (1)	(since C++20)
// Member of specialization std::coroutine_handle<std::noop_coroutine_promise>
// constexpr void destroy() const noexcept;
// (2)	(since C++20)
// 1) Destroys the coroutine state of the coroutine to which *this refers, or
// does nothing if the coroutine is a no-op coroutine. 2) Does nothing. The
// behavior is undefined if destroying is needed and *this does not refer a
// suspended coroutine.

// ReturnType Coroutine(args...)
// {
//     create coroutine state
//     copy parameters to coroutine frame
//     promise_type p;
//     auto return_object = p.get_return_object();

//     try {
//         co_await p.initial_suspend(); // ¹
//         coroutine function body²
//     } catch (...) {
//         p.unhandled_exception();
//     }
//     co_await p.final_suspend();
//     destruct promise p
//     destruct parameters in coroutine frame
//     destroy coroutine state
// }

class Resumable {
 public:
  struct Promise {
    Promise() { cout << "Promise::Promise" << endl; }
    ~Promise() { cout << "Promise::~Promise" << endl; }
    auto get_return_object() noexcept {
      return Resumable(coroutine_handle<Promise>::from_promise(*this));
    }
    // if initial_suspend_return causes the coroutine to be suspended,
    // the coroutine starts in a suspended state and will not be executed
    // unless resume() is called
    auto initial_suspend() noexcept {
      cout << "Promise::initial_suspend(): "
           << "suspend_always()" << endl;
      return suspend_always();
    }
    // The behavior is undefined if destroying is needed and *this does not
    // refer to a suspended coroutine.!!!
    auto final_suspend() noexcept {
      cout << "Promise::final_suspend(): "
           << "suspend_always()" << endl;
      return suspend_always();
    }
    // required when co_return used
    // void return_void() {}
    // void return_value(T v) {}
    void unhandled_exception() {
      cout << "Promise::unhandled_exception()" << endl;
      throw;
    }
    // state accessed from awaitable
    int count{};
    milliseconds d_;
    string msg_;
  };

 private:
  explicit Resumable(coroutine_handle<Promise> h) : h_(h) {
    cout << "Resumable::Resumable(coroutine_handle<Promise>)" << endl;
  }
  coroutine_handle<Promise> h_;

 public:
  // mandatory:
  using promise_type = Promise;
  ~Resumable() {
    cout << "Resumable::~Resumable()>"
         << "\t" << this << endl;
    // WARNING: co-routine MUST be in a suspended state if not
    // destroy() will trigger an exception
    if (h_ && h_.done()) {
      h_.destroy();
    }
  }
  void operator()() { Resume(); }
  operator bool() const { return Done(); }
  void Resume() {
    cout << "Resumable::Resume()" << endl;
    if (h_ && !h_.done()) h_.resume();  // operator () invokes resume
  }
  bool Done() const { return h_.done(); }
  string operator()(const string &msg) {
    if (msg.empty())
      return h_.promise().msg_;
    else {
      const string tmp = h_.promise().msg_;
      h_.promise().msg_ = msg;
      return tmp;
    }
  }
};

struct Bool {
  bool v_;
  Bool(bool v = false) : v_(v) {}
  operator bool() const { return v_; }
};

// the argument is the result of the expression found after
// the co_await keyword
template <class Rep, class Period>
auto operator co_await(tuple<duration<Rep, Period>, int> p) {
  // closure for returned awaitable
  thread_local int x{};
  thread_local string msg;
  struct Awaitable {
    system_clock::duration d_;
    int limit_{};
    coroutine_handle<Resumable::Promise> h_;
    Awaitable(duration<Rep, Period> d, int limit) : d_(d), limit_(limit) {
      std::cout << "Awaitable::Awaitable(duration<Rep, Period>, int)>\t" << this
                << endl;
    }
    // the following contructors are never called
    // Awaitable(Awaitable&& a) : d_(exchange(a.d_, {})) {
    //     cout << "Move>\t\t" << this << endl;
    // }
    // Awaitable(const Awaitable& a) : d_(a.d_) {
    //     cout << "Copy>\t\t" << this << endl;
    // }
    // Awaitable() { cout << "Default Constructor>\t" << endl; }

    // check if result ready:
    //   if ready call await_resume
    //   else call await_suspend THEN call await_resume after
    //        await_suspend returns AND
    bool await_ready() const {
      std::cout << "Awaitable::await_ready() false>\t\t" << this << endl;
      return false;
      // return d_.count() <= 0;
    }
    // called IF AND ONLY IF await_ready returns 'false'
    //‘await_suspend’ must return ‘void’, ‘bool’ or a coroutine handle
    void await_suspend(coroutine_handle<Resumable::Promise> h) {
      cout << "Awaitable::await_suspend(coroutine_handle<Resumable::"
              "Promise>)>\t"
           << this << endl;
      cout << "\tsleeping..." << duration_cast<milliseconds>(d_).count()
           << endl;
      this_thread::sleep_for(d_);
      h.promise().count++;
      h.promise().d_ += duration_cast<decltype(h.promise().d_)>(d_);
      cout << "\tCount: " << h.promise().count
           << " Elapsed: " << h.promise().d_.count() << " ms" << endl;
      h_ = h;
    }
    // co_await <expression> returns what await_resume returns
    // return false if number of invocations < limit, true signals 'done';
    // await_resume can return any value type
    Bool await_resume() {
      cout << "Awaitable::await_resume> - promise::count = "
           << h_.promise().count << "\t" << this << endl;
      return x++ >= limit_;
    }
    ~Awaitable() {
      cout << "Awaitable::~Awaitable()\t" << this << endl << endl;
    }
  };
  return Awaitable(get<0>(p), get<1>(p));
}

template <typename Rep, typename P>
Resumable coro(duration<Rep, P> d, int c) {
  while (!co_await make_tuple(d, c))
    ;
  // co_return;
}

int main(int argc, char const *argv[]) {
  const int count = 2;
  auto sleepTime = 500ms;
  auto c = coro(sleepTime, count);
  cout << endl;
  while (!c) {
    c();
  }
  cout << endl;
  return 0;
}