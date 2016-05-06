//Author: Ugo Varetto
//
//Functors
//

#include <iostream>
#include <cassert>
#include <type_traits>

using namespace std;

class AddFun {
public:
    AddFun(int a, int b) : a_(a), b_(b) {}
    operator int() const { return a_ + b_; }
private:
    int a_;
    int b_;
};

class AddCachedFun {
public:
    AddCachedFun(int a, int b) : res_(a + b) {}
    operator int() const { return res_; }
private:
    int res_;
};

int Sum(int a, int b) {
    return a + b;
}

struct Add {
    int operator()(int a, int b) const { return a + b; }
};

//only callable with a functor
template < typename F, typename... ArgsT >
auto Apply(ArgsT...args)
    -> typename result_of< F(ArgsT...) >::type {
    return F()(args...);
}

//callable with both functor and regular free function
template < typename F, typename... ArgsT >
auto Apply(const F& f, ArgsT...args)
        -> typename result_of< F(ArgsT...) >::type {
    return f(args...);
}

void TestAdd() {
    Add add;
    assert(add(1,2) == 3);
    assert(AddFun(1,2) == 3);
    assert(AddCachedFun(1,2) == 3);
    assert(AddFun(1,2) == AddFun(1,2));
    assert(Apply< Add >(1,2) == 3);
    assert(Apply(Add(), 1,2) == 3);
}

int main(int, char**) {
    TestAdd();
    return 0;
}