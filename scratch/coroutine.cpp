

read >>= filter >>= print

template < typename T, typename F >
State< typename F::return_type > operator >>=(State<T> m, F f) {
    State< typename F::return_type > s = f(m().result);
    return Combine(m.context, s.context, s.result); 
}

[filename]() {
    std::istream is(filename);
    return make_context(is);
} >>= [](std::istream& is) {
                     std::string s;
                     getline(is, s);
                     return make_state(is, s);
  } >>= [](const std::string& s) {
                std::cout << s << '\n';
                return empty_state();
             };


return an std::optional at the end of the >>= chain so we can put eveything inside
a condition!

void f1() {}

void f2() {}

template <typename T> void F() {}

//template <> F<&f1>() {} 


int main()
{
    F<void ()>();
 return 0;
}
