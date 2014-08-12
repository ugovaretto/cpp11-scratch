//Author: Ugo Varetto
//Extract parameter from variadic argument list.

#include <iostream>
#include <cassert>

//------------------------------------------------------------------------------
template < int i >
struct Params {
    template < typename HeadT, typename...TailT >
    static auto Get(const HeadT& h, const TailT&...t) 
           -> decltype(Params< i - 1 >::Get(t...)) {
        return Params< i - 1 >::Get(t...);
    }
};

template <>
struct Params< 0 > {
    template < typename HeadT, typename...TailT >
    static const HeadT& Get(const HeadT& h, const TailT&...t) { return h; }
};

template < int i, typename...Args >
auto Extract(const Args&...args) 
-> decltype(Params< i >::Get(args...)) {
    return Params< i >::Get(args...);
}

void ExtractParameter() {
    auto p = Extract< 2 >("a const char string", 1.2f, 11, 5.0);
    assert(p == 11);
    std::cout << "Extract Parameter: PASSED" << std::endl;
}

//------------------------------------------------------------------------------
int main(int, char**) {
    ExtractParameter();
    return 0;
}
