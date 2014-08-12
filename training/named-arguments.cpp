//Author: Ugo Varetto
//Named argument list: pass and extract function arguments by name using 
//variadic templates

#include <cassert>
#include <cstdint>

//Argument names
enum class ArgName : int8_t { DUMMY = -1, POSITION = -11, COLOR = -111 };

//------------------------------------------------------------------------------
//Argument <name, value> pair wrapper; could use an std::tuple or pair, 
//but this approach can be extended beyond the name-value case with 
//additional fields
template < ArgName N, typename T >
struct Argument {
    using type = T;
    static constexpr ArgName NAME = N;
    Argument(const T& v) : value(v) {}
    Argument(const Argument&) = default;
    T value = T();
};

namespace {
template < ArgName N, typename T >
Argument< N, T > P(T t) { return Argument< N, T >(t); }
}

//Argument extraction
//------------------------------------------------------------------------------
template < typename HeadT, typename...TailT >
struct Head {
    using type = HeadT;
};

//------------------------------------------------------------------------------
template < ArgName NAME, ArgName CURNAME >
struct Extract {
    template < typename HeadT, typename...TailT >
    static auto Get(HeadT h, TailT...t) 
    -> decltype(Extract< NAME, Head< TailT... >::type::NAME >::Get(t...)) {
        return Extract< NAME, Head< TailT... >::type::NAME >::Get(t...);
    }
};

template < ArgName NAME >
struct Extract< NAME, NAME > {
    template < typename HeadT, typename...TailT >
    static typename HeadT::type Get(HeadT h, TailT...t) {
        return h.value;
    }
};

//------------------------------------------------------------------------------
template < ArgName N, typename... Args >
auto Arg(Args...args) 
-> decltype(Extract< N, Head< Args... >::type::NAME >::Get(args...)) {
    return Extract< N, Head< Args... >::type::NAME >::Get(args...);
}

//------------------------------------------------------------------------------
struct Vec3 {
    Vec3() = default;
    Vec3(const Vec3&) = default;
    Vec3(Vec3&&) = default;
    ~Vec3() = default;
    Vec3(int v1, int v2, int v3) : v1_(v1), v2_(v2), v3_(v3) {}
    bool operator==(const Vec3& v) const {
        return v.v1_ == v1_ && v.v2_ == v2_ && v.v3_ == v3_;
    }
    int v1_ = int();
    int v2_ = int();
    int v3_ = int();
};

//------------------------------------------------------------------------------
void ExtractNamedArgumenteter() {
    auto p1 = Arg< ArgName::COLOR >(P< ArgName::DUMMY >(0),
                                      P< ArgName::COLOR >(Vec3{1,0,0}));
    assert(p1 == Vec3(1,0,0));
    auto p2 = Arg< ArgName::COLOR >(P< ArgName::COLOR >(Vec3{1,0,1}),
                                      P< ArgName::DUMMY >(0));
    assert(p2 == Vec3(1,0,1));
}

//------------------------------------------------------------------------------
int main(int, char**) {
    ExtractNamedArgumenteter();
    return 0;
}
