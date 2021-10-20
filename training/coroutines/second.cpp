
#include <cstring>
#include <iostream>
#include <utility>

// As of GCC 11, CLang 12, clang still requires
//   -fcoroutines-ts
//   -stdlib=libc++
//   #include <experimental/...>
//   namespace std::experimental::... 

// NOT: __GNUC__ might be defined even when using CLang
#if defined(__GNUC__) && !defined(__clang__)
#include <coroutine>
namespace CORO = std;
#elif defined(__clang__)
#include <experimental/coroutine>
namespace CORO = std::experimental;
#else
#error Unsupported compiler
#endif

class Resumable {
    struct Promise {
        auto get_return_object() {
            return Resumable{
                CORO::coroutine_handle<Promise>::from_promise(*this)};
        }
        auto initial_suspend() { return CORO::suspend_always{}; }
        auto final_suspend() noexcept { return CORO::suspend_always{}; }
        //auto return_value(T);
        void return_void() {}
        void unhandled_exception() { std::terminate(); }
    };
    CORO::coroutine_handle<Promise> h_;
    explicit Resumable(CORO::coroutine_handle<Promise> h) : h_{h} {}

   public:
    using promise_type = Promise;
    Resumable(Resumable&& r) : h_(std::exchange(r.h_, {})) {}
    ~Resumable() {
        if (h_) h_.destroy();
    }
    bool resume() {
        if (!h_.done()) {
            h_.resume();
        }
        return !h_.done();
    }
};

auto coroutine() -> Resumable {
    std::cout << "  3 ";
    co_await CORO::suspend_always{};
    std::cout << "  5 ";
}

int main(int, char**) {
    std::cout << "1 ";
    auto resumable = coroutine();
    std::cout << "2 ";
    resumable.resume();
    std::cout << "4 ";
    resumable.resume();
    std::cout << std::endl;
    return 0;
}