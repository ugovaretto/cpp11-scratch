#include <concepts>
#include <iostream>

// template <class F, class... Args>

// concept invokable = requires(F &&f, Args &&...args) {
//   std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
//   /* not required to be equality-preserving */
// };

template <typename D, typename CtxT> struct Monad {
  D data;
  CtxT ctx;
  Monad(const D &d, const CtxT &c) : data(d), ctx(c) {}
  Monad(D &&d, CtxT &&c) : data(d), ctx(c) {}
  Monad() = default;
  Monad(const Monad &) = default;
  Monad(Monad &&) = default;
  // template <std::invocable<D> F> Monad operator>>=(F f) {
  //   return Combine(*this, {f(data)});
  // }
};

struct Void {};

template <typename D, typename CtxT = Void> Monad<D, CtxT> Return(D d) {
  return {d, CtxT()};
}

Monad<int, int> inc(int i) { return {i + 1, 0}; }

template <typename T> struct A {
  T data;
};

template <typename D, typename CtxT, std::invocable<D> F>
Monad<D, CtxT> operator>>=(Monad<D, CtxT> a1,
                           F f) { // Monad<D, CtxT> (*f)(D)) {
  return Combine(a1, {f(a1.data)});
}
Monad<int, int> Combine(Monad<int, int> m1, Monad<int, int> m2) {
  return {m1.data, m1.ctx + m2.ctx};
}

int main(int argc, char *argv[]) {
  Monad<int, int> m;

  Monad<int, int> x = Monad<int, int>{0, 0} >>= // inc;
      [](int x) { return Monad<int, int>{}; };

  // N::A<int> a1;
  // N::A<int> a2;
  // auto a3 = a1 >>= a2;
  // std::cout << 31 << std::endl;

  return 0;
}
