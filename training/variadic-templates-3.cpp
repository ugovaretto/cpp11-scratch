#include <cassert>

template < int pos, int count, int Head, int... Tail > 
struct Get {
    enum {value = Get< pos, count + 1, Tail... >::value};
};

template < int p, int Head, int... Tail >
struct Get< p, p, Head, Tail... > {
    enum {value = Head};
};

template < int... I >
 struct Ints {
    template < int p >
    int get() { return Get< p, 0, I... >::value; }
};

//------------------------------------------------------------------------------
int main(int, char**) {
  Ints< 0, 1, 2 > i;
  assert(i.get<0>() == 0
         && i.get<1>() == 1
         && i.get<2>() == 2);   
  return 0;
}
