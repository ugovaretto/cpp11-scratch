
// Copyright (c) 2024 - Ugo Varetto
// Function chaining.
#include <concepts>
#include <expected>
#include <iostream>
#include <iterator>
#include <optional>
#include <tuple>
#include <type_traits>
#include <vector>
//----------------------------------------------------------------------------
/// Return reference to i-th argument pack parameter
template <size_t count, typename... ArgsT> auto &Get(ArgsT &...args) {
  return std::get<count>(std::forward_as_tuple(args...));
}

//----------------------------------------------------------------------------
/// Chained call
template <size_t idx, typename F, typename... Args> struct ChainCall {
  F fun;
  std::tuple<Args...> args;
  ChainCall(F f, Args... args) : fun(f), args(std::make_tuple(args...)) {}
};

template <size_t idx, typename F, typename CheckF, typename... Args>
struct MaybeChainCall {
  F fun;
  CheckF check;
  std::tuple<Args...> args;
  MaybeChainCall(F &&f, CheckF &&cf, Args... args)
      : fun(f), check(cf), args(std::make_tuple(args...)) {}
};

template <size_t idx, typename F, typename CheckF, typename... Args>
struct CheckedChainCall {
  F fun;
  CheckF check;
  std::tuple<Args...> args;
  CheckedChainCall(F &&f, CheckF &&cf, Args... args)
      : fun(f), check(cf), args(std::make_tuple(args...)) {}
};
template <typename F> struct Call {
  std::decay<F>::type fun;
  Call(F &&f) : fun(f) {}
};

//-----------------------------------------------------------------------------
/// Chain operators
template <typename T, typename F, size_t in, typename... ArgsT>
  requires(std::invocable<F, ArgsT...>)
auto operator>(const T &d, ChainCall<in, F, ArgsT...> &&f) {

  auto &param = std::get<in>(f.args);
  param = d;
  // if tuple returned can extract parameter
  return std::apply(f.fun, f.args);
}

template <typename T, typename F>
  requires(std::invocable<F, T>)
auto operator>(const T &v, Call<F> &&f) {
  return f.fun(v);
}

template <typename T, typename F>
  requires(std::invocable<F, T>)
auto operator>(const T &v, F &&f) {
  return f(v);
}

template <typename T, typename F, typename CheckF, size_t in, typename... ArgsT>
  requires(
      std::invocable<F, ArgsT...>,
      std::invocable<CheckF, typename std::invoke_result<F, ArgsT...>::type>)
auto operator>(const std::optional<T> &d,
               MaybeChainCall<in, F, CheckF, ArgsT...> &&f) {

  if (!d.has_value()) {
    return std::optional<T>();
  }
  auto &param = std::get<in>(f.args);
  param = d.value();
  // if tuple returned can extract parameter
  return f.check(std::apply(f.fun, f.args));
}

template <typename T, typename E, typename F, typename CheckF, size_t in,
          typename... ArgsT>
  requires(
      std::invocable<F, ArgsT...>,
      std::invocable<CheckF, typename std::invoke_result<F, ArgsT...>::type>)
auto operator>(const std::expected<T, E> &d,
               CheckedChainCall<in, F, CheckF, ArgsT...> &&f) {

  if (!d.has_value()) {
    return d;
  }
  auto &param = std::get<in>(f.args);
  param = d.value();
  // if tuple returned can extract parameter
  return f.check(std::apply(f.fun, f.args));
}

template <typename InF, typename F, size_t in, typename... ArgsT>
  requires(std::invocable<F, ArgsT...>, std::invocable<InF>)
auto operator|(InF &&g, ChainCall<in, F, ArgsT...> &&f) {
  // if tuple returned can extract parameter
  return [gg = std::move(g), ff = std::move(f)]() mutable {
    auto &param = std::get<in>(ff.args);
    param = gg();
    return std::apply(ff.fun, ff.args);
  };
}

template <typename InF, typename F, size_t in, typename... ArgsT>
auto operator>=(InF &&g, ChainCall<in, F, ArgsT...> &&f) {
  // if tuple returned can extract parameter
  return [gg = std::move(g), ff = std::move(f)](const auto &v) mutable {
    auto &param = std::get<in>(ff.args);
    param = gg(v);
    return std::apply(ff.fun, ff.args);
  };
}

template <typename T> auto Fun() {
  return [](const T &v) { return v; };
}

// template <typename T, typename F, size_t in, typename... ArgsT>
// auto operator>=(const T &, ChainCall<in, F, ArgsT...> &&f) {
//   // if tuple returned can extract parameter
//   return [ff = std::move(f)](const T &v) mutable {
//     auto &param = std::get<in>(ff.args);
//     param = v;
//     return std::apply(ff.fun, ff.args);
//   };
// }

template <typename T>
concept Range = requires(T r) {
  begin(r);
  end(r);
  *begin(r);
  begin(r)++;
  begin(r) != end(r);
};
template <Range R> void print(R r) {
  auto b = begin(r);
  while (b != end(r)) {
    std::cout << *b++ << std::endl;
  }
}

template <Range R, typename F, size_t in, typename... ArgsT>
  requires(std::invocable<F, ArgsT...>)
auto operator>(const R &r, ChainCall<in, F, ArgsT...> &&f) {
  auto b = begin(r);
  R ret;
  auto i = std::back_inserter(ret);
  while (b != end(r)) {
    auto &param = std::get<in>(f.args);
    param = *b++;
    *i++ = std::apply(f.fun, f.args);
  }
  // if tuple returned can extract parameter
  return ret;
}

template <Range R, typename F, size_t in, typename... ArgsT>
  requires(std::invocable<F, ArgsT...>)
auto operator|(const R &r, ChainCall<in, F, ArgsT...> &&f) {
  auto b = begin(r);
  R ret;
  auto i = std::back_inserter(ret);
  while (b != end(r)) {
    auto &param = std::get<in>(f.args);
    param = *b++;
    *i++ = std::apply(f.fun, f.args);
  }
  // if tuple returned can extract parameter
  return ret;
}
//-----------------------------------------------------------------------------
/// Helper functions to generate ChainCall objects from function and paraters.
template <size_t i, typename F, typename... ArgsT>
auto cc(F &&f, ArgsT &&...args) {
  return ChainCall<i, F, ArgsT...>(f, std::forward<ArgsT>(args)...);
}

template <typename F, typename... ArgsT> auto cc(F &&f, ArgsT &&...args) {
  return cc<0>(f, std::forward<ArgsT>(args)...);
}

template <typename F> auto cf(F &&f) { return Call(f); }

template <size_t i, typename F, typename CheckF, typename... ArgsT>
auto mcc(F &&f, CheckF &&cf, ArgsT &&...args) {
  return MaybeChainCall<i, F, CheckF, ArgsT...>(std::forward<F>(f),
                                                std::forward<CheckF>(cf),
                                                std::forward<ArgsT>(args)...);
}
template <size_t i, typename F, typename CheckF, typename... ArgsT>
auto ccc(F &&f, CheckF &&cf, ArgsT &&...args) {
  return CheckedChainCall<i, F, CheckF, ArgsT...>(std::forward<F>(f),
                                                  std::forward<CheckF>(cf),
                                                  std::forward<ArgsT>(args)...);
}

template <Range R, typename F> auto lazy_apply(const R &r, F &&f) {
  return [&, ff = std::move(f)]() mutable {
    static thread_local auto b = begin(r);
    return b != end(r) ? std::make_optional(ff(*b++)) : std::nullopt;
  };
}

//-----------------------------------------------------------------------------
/// Integer placehoder definition to be put in place of parameter being
/// subsituted.
#ifdef i_ph
#error "i_ph already defined"
#endif
#define _i_ std::forward<int>(0)

//----------------------------------------------------------------------------
/// Sample code
///
int times2(int i) { return 2 * i; }

int sum(int a, int b) { return a + b; }

template <typename T> std::optional<T> to_opt(const T &d) {
  return std::optional<T>(d);
}

template <typename T, typename E> std::expected<T, E> to_expected(const T &v) {
  return std::expected<T, E>(v);
}

void print(int i, int j, const std::string &s, float f) {
  std::cout << i << ' ' << j << ' ' << s << ' ' << f;
}

template <typename... ArgsT> void test_get(ArgsT... args) {
  Get<2>(args...) = "ciao";
  print(args...);
}

template <typename RetT, std::invocable F, size_t idx, typename... ArgsT>
auto Exec(RetT r, F f, ArgsT... args) -> decltype(f(args...)) {
  Get<idx>(args...) = r;
  return f(args...);
}

void foo(int i, const std::string &s) {
  std::cout << i << ' ' << s << std::endl;
}

enum class RangeError { TooBig, TooSmall };
//----------------------------------------------------------------------------
int main(int, char **) {
  test_get(1, 3, "this is it", 3.5);
  auto t = std::make_tuple(2, std::string("Hello"));
  std::apply(foo, t);
  auto neg = [](int x) {
    if (x < 0)
      return std::optional<int>();
    else
      return std::optional<int>(-x);
  };
  auto x = 8 > cc<1>(sum, 3, _i_) > cc<0>(sum, _i_, 3) > cc(times2, _i_) >
           cc<0>([](auto x) { return x / 2; }, _i_) > cc(to_opt<int>, _i_) >
           mcc<0>([](auto x) { return -x; }, neg, _i_);
  std::cout << (x.has_value() ? std::to_string(x.value()) : "NULLOPT")
            << std::endl;
  // auto x = 8 > cc<1>(sum, 3, _int) > cc<0>(sum, _int, 3) > cc(times2, _int) >
  //          cc<0>([](auto x) { return x / 2; }, _int);
  // std::cout << x << std::endl;
  std::vector<int> v = {1, 2, 3, 4, 5};
  print(v);
  auto vout = v > cc(times2, _i_) > cc(sum, _i_, 3);
  print(vout);
  auto ff = []() { return 10; };
  auto l = ff | cc(sum, _i_, 11) | cc(times2, _i_);
  std::cout << l() << std::endl;
  auto a = Fun<int>() >= cc(sum, _i_, 10) >= cc(times2, _i_) >= cc(times2, _i_);
  std::cout << a(100) << std::endl;
  auto next = lazy_apply(v, a);
  for (auto i = next(); i != std::nullopt; i = next()) {
    std::cout << (i.has_value() ? std::to_string(i.value()) : "?") << std::endl;
  }
  struct F {
    int operator()(int x) { return x * x; }
  };
  auto z = 3 > cf(times2) > cf(times2) > [](int x) { return 3 * x; } > F();
  std::cout << z << std::endl;
  auto iv = 10;
  auto w =
      iv > cf(times2) >
      cf(to_expected<std::invoke_result<decltype(times2), decltype(iv)>::type,
                     RangeError>) >
      ccc<0>(
          times2,
          [](int i) {
            return i > 10 ? std::unexpected(RangeError::TooBig)
                          : std::expected<int, RangeError>(i);
          },
          _i_);
  std::cout << (w.has_value() ? "YEAH" : ":-(") << std::endl;
}
