#include <vector>
#include <iostream>
#include <ranges>
#include <algorithm>
#include <fstream>
#include "generator.h"

template <typename Range>
auto GapEncode(const Range& ids, auto base) -> Generator<int> {
    auto lastId = base;
    for(auto id: ids) {
        const auto gap = id - lastId;
        lastId = id;
        co_yield gap;
    }
}

template <typename Range>
auto GapDecode(Range& gaps, auto base) -> Generator<int> {
    auto lastId = base;
    for(auto gap: gaps) {
        const auto id = gap + lastId;
        co_yield id;
        lastId = id;
    }
}


auto vb_encode_num(int n) -> Generator<std::uint8_t> {
    for (auto cont = std::uint8_t{0}; cont == 0;) {
        auto b = static_cast<std::uint8_t>(n % 128);
        n /= 128;
        cont = (n == 0) ? 128 : 0;
        co_yield (b + cont);
    }
}

template <std::ranges::range R>
auto vb_encode(R& r) -> Generator<std::uint8_t> {
    for(auto n : r) {
        auto bytes = vb_encode_num(n);
        for(auto b: bytes) {
            co_yield b;
        }
    }
}

template <std::ranges::range R>
auto vb_decode(R& bytes) -> Generator<int> {
    auto n = 0;
    auto weight = 1;
    for(auto b: bytes) {
        if(b < 128) {
            n += b * weight;
            weight *= 128;
        } else {
            n += (b - 128) * weight;
            co_yield n;
            n = 0;
            weight = 1;
        }
    }
}

template <std::ranges::range R>
auto compress(R& ids) -> Generator<int> {
  auto gaps = GapEncode(ids, 0);
  
  auto bytes = vb_encode(gaps);

  for (auto b : bytes) {
    co_yield b;
  }
}

template <std::ranges::range R>
auto decompress(R& bytes) -> Generator<int> {
  auto gaps = vb_decode(bytes);
  auto ids = GapDecode(gaps, 0);
  for (auto id : ids) {
    co_yield id;
  }
}

template <typename Range>
void write(const std::string& path, Range& bytes) {
  auto out = std::ofstream{path, std::ios::out | std::ofstream::binary};
  std::ranges::copy(bytes.begin(), bytes.end(),    
                    std::ostreambuf_iterator<char>(out));
}
auto read(std::string path) -> Generator<std::uint8_t> {
  auto in = std::ifstream {path, std::ios::in | std::ofstream::binary};
  auto it = std::istreambuf_iterator<char>{in};
  const auto end = std::istreambuf_iterator<char>{};
  for (; it != end; ++it) {
    co_yield *it;
  }
}

template <std::ranges::range R>
auto ToVector(R&& r) {
    std::vector<std::ranges::range_value_t<R>> v;

    //try to reserve size
    if constexpr (requires { std::ranges::size(r); }) {
        v.reserve(std::ranges::size(r));
    }

    // push all the elements
    for (auto&& e : r) {
        v.push_back(static_cast<decltype(e)&&>(e));
    }

    return v;
}

int main(int argc, const char** argv) {
    auto ids = std::vector{10, 11, 12, 14};
    auto gaps = GapEncode(ids, 0);
    auto v = ToVector(gaps);
    for(auto&& gap: v) std::cout << gap << ", ";
    std::cout << std::endl;
    auto idss = GapDecode(v, 0);
    for(auto id: idss) std::cout << id << ", ";
    std::cout << std::endl;
{
    auto documents = std::vector{367, 438, 439, 440};
    auto bytes = compress(documents);
    write("values.bin", bytes);
}
{
    auto bytes = read("values.bin");
    auto documents = decompress(bytes);
    for (auto doc : documents) {
      std::cout << doc << ", ";
    }
}
    std::cout << std::endl;
    return 0;
}