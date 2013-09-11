#include <iostream>
//const T&& - a use case: when keeping references we do not want to accept
//references to temporary objects
#ifdef RVALREF
void bar(const int&&) {
    std::cout << "&& - !!!\n";
};
#endif
void bar(int&) {
    std::cout << "&\n";
};
void bar(const int&) {
    std::cout << "const &\n";
}
int main(int, char**) {
    int i = int();
    bar(i); // calls &
    bar(1); // calls && (would call const & if && overload not present)
    bar(([]{return 1;})()); // calls && (would call const & if &&
                            //   overload not present)
    return 0;
}