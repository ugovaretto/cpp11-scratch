module;
//#include <iostream>
//does not work:
// GCC 11: if client imports string as a module, text is printed correctly
//         any const char* is however interpreted as a pointer and an hex
//         address printed.
//         char is converted to int.
//         #including <string> in client code instead of importing results in 
//         an endlesserror message loop when compiling

export module t_io;
//import <iostream>;
// works fine with GCC-11, broken with CLang, see errors below, importing
// iosfwd and string won't fix the problem
import <iostream>;
//import <iosfwd>;
//import <string>;
export
template <typename T>
void Print(const T& p) {
  std::cout << p << std::endl;
}

//gcc-11
//#including instead of importing results in the address of 'p' being printed
//importing works as expected
//std::string does not work: endless error endless message loop

//clang-12 BROKEN:
// In file included from ../import_template.cpp:1:
// /home/ugovaretto/projects/cpp11-scratch/tmp-scratch/modules/build/../export_template.cpp:11:13: error: explicit specialization of 'char_traits<char>' must be imported from one of the following modules before it is required:
//         std.__string
//         std.iosfwd
//   std::cout << p << '\n';
//             ^
// ../import_template.cpp:4:5: note: in instantiation of function template specialization 'Print<char [16]>' requested here
//     Print("Hello from main");
//     ^
// /usr/lib/llvm-12/bin/../include/c++/v1/__string:354:29: note: explicit specialization declared here is not reachable
// struct _LIBCPP_TEMPLATE_VIS char_traits<char>
//                             ^
// In file included from ../import_template.cpp:1:
// /home/ugovaretto/projects/cpp11-scratch/tmp-scratch/modules/build/../export_template.cpp:11:13: error: invalid operands to binary expression ('std::ostream' (aka 'basic_ostream<char>') and 'char const[16]')
//   std::cout << p << '\n';
//   ~~~~~~~~~ ^  ~
// /usr/lib/llvm-12/bin/../include/c++/v1/cstddef:141:3: note: candidate function template not viable: no known conversion from 'std::ostream' (aka 'basic_ostream<char>') to 'std::byte' for 1st argument
//   operator<< (byte  __lhs, _Integer __shift) noexcept
//   ^
// /usr/lib/llvm-12/bin/../include/c++/v1/ostream:1078:1: note: candidate template ignored: could not match 'shared_ptr<type-parameter-0-2>' against 'char const[16]'
// operator<<(basic_ostream<_CharT, _Traits>& __os, shared_ptr<_Yp> const& __p)
// ^
// 2 errors generated.

// with std::endl;
// /home/ugovaretto/projects/cpp11-scratch/tmp-scratch/modules/build/../export_template.cpp:21:21: error: reference to overloaded function could not be resolved; did you mean to call it?
//   std::cout << p << std::endl;
//                     ^~~~~~~~~
// ../import_template.cpp:4:5: note: in instantiation of function template specialization 'Print<std::string>' requested here
//     Print(std::string("Hello from main"));
//     ^
// /usr/lib/llvm-12/bin/../include/c++/v1/ostream:1003:1: note: possible target for call
// endl(basic_ostream<_CharT, _Traits>& __os)
// ^
// /usr/lib/llvm-12/bin/../include/c++/v1/ostream:1061:1: note: candidate function [with _CharT = char, _Traits = std::char_traits<char>] not viable: no overload of 'endl' matching 'basic_string_view<char>' for 2nd argument
// operator<<(basic_ostream<_CharT, _Traits>& __os,
// ^
// /usr/lib/llvm-12/bin/../include/c++/v1/cstddef:141:3: note: candidate template ignored: couldn't infer template argument '_Integer'
//   operator<< (byte  __lhs, _Integer __shift) noexcept
//   ^
// /usr/lib/llvm-12/bin/../include/c++/v1/ostream:1053:1: note: candidate template ignored: couldn't infer template argument '_Allocator'
// operator<<(basic_ostream<_CharT, _Traits>& __os,
// ^
// /usr/lib/llvm-12/bin/../include/c++/v1/ostream:1078:1: note: candidate template ignored: couldn't infer template argument '_Yp'
// operator<<(basic_ostream<_CharT, _Traits>& __os, shared_ptr<_Yp> const& __p)
// ^
// 1 error generated.