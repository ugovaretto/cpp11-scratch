#build with G++, pre-build iostream from include then build and link with module
g++-11 -std=c++20 -fmodules-ts -xc++-system-header iostream
# WARNING ALL DEPENDENT MODULES MUST BE BUILT BEFORE TOP-LEVEL MODULE
g++-11 -std=c++20 -fmodules-ts  ../top_child1_module.cpp \
        ../top_child2_module.cpp ../top_module.cpp ../import_module.cpp