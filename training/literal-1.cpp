//Author: Ugo Varetto
//started playing with new literals and enums
#include <iostream>
#include <array>
#include <tuple>
#include <string>
#include <cstring>

using namespace std;

template < char...C >
size_t operator "" _numchar() {
    return sizeof...(C);
}

size_t operator "" _size(const char* c) {
    return strlen(c);
}

size_t operator "" _length(const char* c, size_t n) {
    return n;
}


enum class Unit : char {m, km};

template < Unit u >
struct Length {
    int size = 0;
    Length(int s) : size(s) {}
};

Length< Unit::m > operator "" _m(const char* v) {
    return Length< Unit::m >{stoi(v)};
}

Length< Unit::km > operator "" _km(const char* v) {
    return Length< Unit::km >{stoi(v)};
}

Length< Unit::m > operator+(const Length< Unit::km >& km,
                            const Length< Unit::m >& m ) {
    return Length< Unit::m >{km.size * 1000 + m.size};
}

Length< Unit::m > operator+(const Length< Unit::m >& m,
                            const Length< Unit::km >& km ) {
    return Length< Unit::m >{km.size * 1000 + m.size};
}

template < Unit u >
Length< u > operator+(const Length< u >& v1,
                     const Length< u >& v2 ) {
    return Length< u >{v1.size + v2.size};
}


ostream& operator<<(ostream& os, const Length< Unit::m >& m) {
    os << m.size << "m";
    return os;
}

ostream& operator<<(ostream& os, const Length< Unit::km >& km) {
    os << km.size << "km";
    return os;
}

int main(int, char**) {
    cout << 1234_numchar << endl;
    cout << 1234_size << endl;
    cout << "1234"_length << endl;
    cout << 10_km + 500_m << endl;
    return 0;
}