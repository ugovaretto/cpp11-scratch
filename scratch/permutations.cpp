#include <iostream>
#include <vector>

// template < typename IterT >
// bool nextperm(IterT beg, IterT end) {
//     if(beg == end) return false;
//     IterT i = end; //point to one past end of sequence
//     --i; //point to last element in sequence
//     if(beg = i) return false; //return false when
//                               //asked to start at last element
//     while(true) {
//         IterT i1, i2;
//         i1 = i; //point to last element
//         --i; //point to previous element
//         if(*i < *i1) {  //if previous element is less than last element
//             i2 = end; //point to one past last element
//             --i2; //point to last element
//             //look backward for first element equal or smaller than the last
//             //but one
//             while(!(*i < *i2)) --i2;
//             //swap
//             std::iter_swap(i, i2);
//             std::reverse(i1, last);
//             return true;
//         }
//         if(first == i) {
//             std::reverse(first, last);
//             return false;
//         }
//     }                          
// }


template < typename T >
void Permutations(const std::vector< T >& in,
                  std::vector< std::vector< T > >& out ) {
    for(std::vector< T >::const_iterator i = in.begin();
        i != in.end();
        ++i) {
        std::vector< T > v1;
        std::push_back_iterator< T > pbi(v1);
        std::copy(in.begin(), i, pbi);
        std::vector< T >::const_iterator i2 = i;
        if(++i2 != in.end()) 
            std::copy(i2, in.end(), pbi);
        std::vector< std::vector< T > > ov;
        Permutations(v1, ov);
        for(auto vi: ov) {
            std::vector< T > v1;
            std::push_back_iterator< T > pbi(v1);
            v1.push_back(*i);
            std::copy(vi.begin(), vi.end(), pbi);
            std::vector< T > v2 = vi;
            v2.push_back(*i);
            out.push_back(v1);
            out.push_back(v2);
        }
    }
}


//------------------------------------------------------------------------------
int main(int, char**) {
    using Chars = std::vector< char >;
    using CharsArray = std::vector< Chars >;
    const Chars text = {'a', 'b', 'a', 'h'};
    CharArray textperm;
    Permutations(text, textperm); 
    for(auto& ca: textperm) {
        for(auto& c: ca) {
            std::cout << c;
        }
        std::cout << std::endl;
    }
    return 0;
}
