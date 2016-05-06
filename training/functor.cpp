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

//C++17-like invokers
//only callable with a functor
template < typename F, typename... ArgsT >
typename result_of< F&&(ArgsT&&...) >::type Invoke(ArgsT&&...args) {
    return F()(std::forward<ArgsT>(args)...);
}

//callable with both functor and regular free function
template < typename F, typename... ArgsT >
typename result_of< F&&(ArgsT&&...) >::type Invoke(F&& f, ArgsT&&...args) {
    return f(std::forward<ArgsT>(args)...);
}

void TestAdd() {
    Add add;
    assert(add(1,2) == 3);
    assert(AddFun(1,2) == 3);
    assert(AddCachedFun(1,2) == 3);
    assert(AddFun(1,2) == AddFun(1,2));
    assert(Invoke< Add >(1,2) == 3);
    assert(Invoke(Add(), 1,2) == 3);
}

int main(int, char**) {
    TestAdd();
    return 0;
}