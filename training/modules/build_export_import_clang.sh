#build with modules support with CLang 12

FLAGS="-std=c++20 -stdlib=libc++ -fmodules"

clang++ $FLAGS -Xclang -emit-module-interface -c ../export_import.cpp -o io.pcm

clang++ $FLAGS -fprebuilt-module-path=. ../use_export_import.cpp ../export_import.cpp -o print.out

rm *.pcm