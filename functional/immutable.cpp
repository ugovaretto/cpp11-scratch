#include <iostream>

using namespace std;

struct C {
  int i;
  C(C &&c) : i(c.i) { cout << "&&" << endl; };
  C(int i) : i(i) {}
  C(const C &c) : i(c.i) { cout << "const &" << endl; }
  C operator=(C &&c) {
    i = c.i;
    cout << "=" << endl;
    return *this;
  }
};

C set_i(C &&c, int i) {
  c.i = i;
  return std::move(c);
}

int main(int argc, char *argv[]) {
  C c{2};
  C c2 = std::move(set_i(std::move(c), 10));
  cout << c2.i << endl;

  return 0;
}
