FLAGS="-std=c++20 -stdlib=libc++ -fbuiltin-module-map -fmodules"

clang++ $FLAGS -Xclang -emit-module-interface -c ../export_template.cpp -o t_io.pcm

clang++ $FLAGS -fprebuilt-module-path=. ../import_template.cpp ../export_template.cpp

rm *.pcm