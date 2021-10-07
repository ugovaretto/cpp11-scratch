#build with G++, pre-build iostream from include then build and link with module
g++-11 -std=c++20 -fmodules-ts -xc++-system-header iostream
g++-11 -std=c++20 -fmodules-ts ../export_import.cpp ../use_export_import.cpp