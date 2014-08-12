#include <iostream>





struct Vec3 {
    Vec3(int v1, int v2, int v3) : v1_(v1), v2_(v2), v3_(v3) {}
    int v1_;
    int v2_;
    int v3_;
};

std::ostream& operator<<(std::ostream& os, const Vec3& v) {
    os << v.v1_ << ' ' << v.v2_ << ' ' << v.v3_;
    return os;
}

template < typename...Args >
void Foo(Args...args) {
    auto col = Params< P::COLOR >::Get(args...);
    auto pos = Params< P::POS >::Get(args...);
    std::cout << "Position: " << pos << std::endl;
    std::cout << "Color:    " << col << std::endl;
}

int main(int, char**) {
    using p = Param;
    Foo(p{P::POS, {0,1,0}}, p{P::COLOR, {255, 0, 255});
    return 0;
}
