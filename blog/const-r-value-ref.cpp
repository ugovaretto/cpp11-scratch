#include <iostream>
//const T&& - a use case: when keeping references we do not want to accept
//references to temporary objects
void bar(const int&&) {
    std::cout << "Noooooooo\n";
};
void bar(int&) {
    std::cout << "OK\n";
};
void bar(const int&) {
    std::cout << "OK\n";
}
int main(int, char**) {
    int i = int();
    bar(i);
    bar(1);
    bar(([]{return 1;})());
    return 0;
}