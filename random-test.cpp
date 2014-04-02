//author: Ugo Varetto
//test driver for Random.h

#include "Random.h"
#include <iostream>
#include <cassert>


int main(int, char**) {
    Random< int > rint(1,10);
    for(int i = 0; i != 100; ++i) {
        const int r = rint();
        assert(r <= 10 && r >= 1);
    }
    Random< float > rfloat = {-0.10f, 10.0f};
    for(int i = 0; i != 100; ++i) {
        const float r = rfloat();
        assert(r <= 10.0f && r >= -0.10f);
    }
    return 0;
}

