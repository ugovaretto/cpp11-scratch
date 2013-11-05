
template < int N, typename FwdT, typename H, typename...Args >
void fill_sequence(FwdT& n, H& h,  Args...args) {
    *n++ = &h;
    fill_sequence< N - 1 >(FwdT, n, args...);
}

template < typename FwdT >
void fill_sequence<0>(FwdT&){}

template < typename...Args >
void launch(CUfunction& f, Args&&...args) {
    std::vector< void* > p(sizeof...(Args), nullptr);
    fill_sequence< sizeof...(Args) >(begin(p), std::forward<Args>(args)...);
}