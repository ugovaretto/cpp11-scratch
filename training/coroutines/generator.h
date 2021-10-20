#include <utility>
#include <iterator>
#include "coroutines.h"


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

//------------------------------------------------------------------------------