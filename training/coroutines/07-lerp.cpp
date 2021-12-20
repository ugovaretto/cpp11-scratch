#include <cassert>
#include <cmath>
#if !defined(__clang__)
#include <concepts> //cannot make std::concept to work on apple clang 13 
#endif
#include <iostream>
#include <vector>
#include <utility>

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

//------------------------------------------------------------------------------
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
        auto operator++(int) { return operator++(); }
        const T& operator*() const { return h_.promise().value_; }
        const T* operator->() const { return std::addressof(operator*()); }
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

//------------------------------------------------------------------------------

// lerp implementation

template <typename T>
auto LinValue(T start, T stop, size_t index, size_t n) {
    assert(n > 1 && index < n);
    const auto amount = static_cast<T>(index) / (n - 1);
    return std::lerp(start, stop, amount);
}

// eager range
template <typename T>
auto LinSpaceEager(T start, T stop, size_t n) {
    auto v = std::vector<T>{};
    for (auto i = 0u; i != n; ++i) {
        v.push_back(LinValue(start, stop, i, n));
    }
    return v;
}

// lazy callback
template <typename T, typename F>
#if !defined(__clang__)
std::invocable<F, const T&> //cannot make std::concept to work on apple clang 13
#endif
void LinSpaceCBack(T start, T stop, size_t n, F&& f) {
    for (auto i = 0u; i != n; ++i) f(LinValue(start, stop, i, n));
}

// iterator
template <typename T>
struct LinSpace {
    LinSpace(T start, T stop, size_t n) : begin_{start, stop, 0, n}, end_{n} {}
    struct Iterator {
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using reference = value_type /*&*/;
        using pointer = value_type*;
        using difference_type = void;
        auto operator++() { return ++i_; }
        bool operator==(size_t i) const { return i == i_; }
        bool operator==(const Iterator& it) const { return it.i_ == i_;}
        value_type operator*() { return LinValue(start_, stop_, i_, n_); }
        T start_{};
        T stop_{};
        size_t i_{};
        size_t n_{};
    };
    auto begin() { return begin_; }
    auto end() { return end_; }

   private:
    Iterator begin_{};
    size_t end_{};
};

template <typename T>
auto LinSpaceIter(T start, T stop, size_t n) {
    return LinSpace<T>(start, stop, n);
}

template <typename T>
auto LinSpaceGen(T start, T stop, size_t n) -> Generator<T> {
    for(auto i = 0u; i != n; ++i) co_yield LinValue(start, stop, i, n);
}


int main(int argc, char const* argv[]) {
    for (auto i : LinSpaceEager(0., 10., 20)) std::cout << i << ", ";
    std::cout << std::endl;
    LinSpaceCBack(0., 10., 20, [](auto v) { std::cout << v << ", "; });
    std::cout << std::endl;
    for (auto x : LinSpaceIter(0., 10., 20)) std::cout << x << ", ";
    std::cout << std::endl;
    for (auto x : LinSpaceGen(0., 10., 20)) std::cout << x << ", ";
    std::cout << std::endl;
    return 0;
}
