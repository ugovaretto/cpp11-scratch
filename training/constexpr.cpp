//Author: Ugo Varetto
//constexpr

#include <iostream>

using namespace std;

//------------------------------------------------------------------------------
constexpr long double operator"" _f(long double f) {
    return (f - 32) / 1.8;
}

constexpr unsigned long long operator"" _f(unsigned long long f) {
    using ull = unsigned long long;
    return ull((f - 32) / 1.8);
}

//------------------------------------------------------------------------------
template < unsigned long long T >
struct Temp {
    enum : unsigned long long {t = T};
    constexpr operator unsigned long long () { return t; }
};


//------------------------------------------------------------------------------
int main(int, char**) {
    const char* F = "\u2109";
    const char* C = "\u2103";
    cout << "65 " << F << " = " << 65_f   << ' ' << C << endl;
    cout << "65 " << F << " = " << 65.0_f << ' ' << C << endl;
    Temp< 72_f > temp;
    cout << "72 " << F << " = " << temp   << ' ' << C << endl;
    Temp< (Temp< 212_f >()) > temp2;
    cout << "212 " << F << " = " << temp2  << ' ' << C << endl;
    return 0;
}
