#include <concepts>
#include <iostream>

template <std::invocable<int> F> void call_if_callable(F func) {
  std::cout << "Callable!n";
  func(3);
}

// Usage
int main() {
  auto lambda = [](int x) { std::cout << "Lambda called." << std::endl; };
  call_if_callable(lambda); // Works fine
  // call_if_callable(5);    // Compilation error
  return 0;
}
