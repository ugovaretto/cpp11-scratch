#include <array>
#include <iostream>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>


// generator

// NOTE: __GNUC__ might be defined even when using CLang
#if defined(__GNUC__) && !defined(__clang__)
#include <coroutine>
namespace CORO = std;
#elif defined(__clang__)
#include <experimental/coroutine>
namespace CORO = std::experimental;
#else
#error Unsupported compiler
#endif

template <typename T>
class Generator {
    struct Promise {
        T value_;
        auto get_return_object() -> Generator {
            return Generator{
                CORO::coroutine_handle<Promise>::from_promise(*this)};
        }
        auto initial_suspend() { return CORO::suspend_always{}; }
        auto final_suspend() noexcept { return CORO::suspend_always{}; }
        void return_void() {}
        void unhandled_exception() { throw; }
        auto yield_value(T&& value) {
            value_ = std::move(value);
            return CORO::suspend_always{};
        }
        auto yield_value(const T& value) {
            value_ = value;
            return CORO::suspend_always{};
        }
    };
    struct Sentinel {};
    struct Iterator {
        using iterator_category = std::input_iterator_tag;
        using value_type = T;
        using difference_type = ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;
        CORO::coroutine_handle<Promise> h_;
        Iterator& operator++() {
            h_.resume();
            return *this;
        }
        void operator++(int) { return (void)operator++(); }
        const T& operator*() const { return h_.promise().value_; }
        T* operator->() const { return std::addressof(operator*()); }
        bool operator==(Sentinel) const { return h_.done(); }
    };

    CORO::coroutine_handle<Promise> h_;
    explicit Generator(CORO::coroutine_handle<Promise> h) : h_{h} {}

   public:
    using promise_type = Promise;
    Generator(Generator&& g) : h_(std::exchange(g.h_, {})) {}
    ~Generator() {
        if (h_) h_.destroy();
    }
    auto begin() {
        h_.resume();
        return Iterator{h_};
    }
    auto end() { return Sentinel{}; }
};

template <typename T>
auto Seq() -> Generator<T> {
    for (T i = {};; ++i) co_yield i;
}

template <typename T>
auto TakeUntil(Generator<T>& gen, T value) -> Generator<T> {
    for (auto&& v : gen) {
        //‘co_return’ cannot be used in a function with a deduced return type
        if (v == value) co_return;
        co_yield v;
    }
}

template <typename T>
auto Add(Generator<T>& gen, T adder) -> Generator<T> {
    for (auto&& v : gen) {
        co_yield v + adder;
    }
}

int main(int argc, const char** argv) {
    auto s = Seq<int>();
    auto t = TakeUntil<int>(s, 10);
    auto a = Add<int>(t, 3);
    int sum = 0;
    for (auto&& v : a) sum += v;
    std::cout << sum << std::endl;  // returns 75
    return 0;
}