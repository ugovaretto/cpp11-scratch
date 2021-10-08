//GCC
//gcc version 11.1.0 (Ubuntu 11.1.0-1ubuntu1~21.04) 
//
//Build iostream module first (can use -O flag to optimize):
// g++-11 -std=c++20 -fmodules-ts -xc++-system-header iostream
//Build executable
// g++-11 -std=c++20 -fmodules-ts ../import_test.cpp
//
//Pre-compiled modules reside in: gcm.cache folder
//gcm.cache/usr/include/c++/11/iostream.gcm:
// 5517360 bytes no flags
// 5733504 bytes -O3

//CLang
//Ubuntu clang version 12.0.0-3ubuntu1~21.04.2
//
//install (Ubuntu/Debian):
// sudo apt install clang-12 libc++-12-dev libc++abi-12-dev
//
//CLang has already pre-built stdlib modules when libc++ is used
//and does not require explicit compilation of iostream...
//
// clang++ -std=c++20 -fmodules -stdlib=libc++ ../import_test.cpp
// ...but it does require explicit generation of module interface files
// when compiling modules


//import <iostream>;
#include <iostream>

int main(int argc, const char** argv) {
    std::cout << "Hello Modules!" << std::endl;
    return 0;
}