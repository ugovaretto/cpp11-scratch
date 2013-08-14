
void f1() {}

void f2() {}

template <typename T> void F() {}

//template <> F<&f1>() {} 


int main()
{
    F<void ()>();
 return 0;
}