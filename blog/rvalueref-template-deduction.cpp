
template < typename T >
void Forward(T&& ) {
}

// void Forward(int&& i) {

// }

int main(int, char**) {
    int i = int();
    Forward(i);
    return 0;
}