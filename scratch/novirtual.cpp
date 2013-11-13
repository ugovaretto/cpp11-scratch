

struct IB {
    virtual int get(int i) const = 0;
};

struct D {// : IB {
    /*virtual*/ int get(int i) const { return i / 2; }
};

template < typename T >
T f(T t) {return t;}

template <>
int f(int);

int main(int, char**) {
    //D d;
    D b;
    int i = b.get(4);
    return 0;
}


