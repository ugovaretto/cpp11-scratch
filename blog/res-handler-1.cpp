#include <cassert>   //assert
#include <algorithm> //swap
#include <vector>

//------------------------------------------------------------------------------
template <typename T>
class MemoryHandler {
private:
    struct Proxy {
        T* ptr;  
    };
public:
    MemoryHandler(const MemoryHandler& mh) : ptr_(0), moved_(true) {
        if(!mh.moved_) assert(false);
        mh.MoveTo(*this);
    }
    MemoryHandler(const Proxy& p) : ptr_(p.ptr), moved_(true) {}
    MemoryHandler(MemoryHandler& mh) : ptr_(mh.ptr_), moved_(true) {
        mh.ptr_ = 0;
    }
    /*explicit*/ MemoryHandler(T* ptr = 0) : ptr_(ptr), moved_(false) {}
    T* ptr() { return ptr_; }
    ~MemoryHandler() { 
        delete ptr_; //it is fine to call 'delete 0'
    }
    operator Proxy() {
        T* ptr = ptr_;
        Proxy p;
        p.ptr = ptr;
        ptr_ = 0; //memory handler moved out: reset this->ptr_ pointer
        return p;
    }
    MemoryHandler& operator=(MemoryHandler& mh) {
        MemoryHandler(mh).Swap(*this);
        moved_(true);
        return *this;
    }
private:
    void Swap(MemoryHandler& mh) {
        std::swap(mh.ptr_, ptr_);
    } 
    void MoveTo(MemoryHandler& other) {
        other.ptr_ = ptr_;
        ptr_ = 0;
    }
private:
    T* ptr_;
    bool moved_;
};

//------------------------------------------------------------------------------
struct Printer {
    Printer(const char*) {}
    void Print(const char*) {}
};

void PrintCppStd() {
  MemoryHandler< Printer > mh(new Printer("remote printer 1"));
  Printer* p = mh.ptr();
  p->Print("./ISO_IEC_14882-2011.pdf");  
} //resource automatically destroyed by MemoryHandler destructor

MemoryHandler< int > MemHandlerFactory(int i) {
    return MemoryHandler< int >(new int (i));
}

void Test() {
    MemoryHandler< int > mh1(new int(1));
    assert(*mh1.ptr() == 1);
    MemoryHandler< int > mh2(mh1);
    assert(mh1.ptr() == 0);
    assert(*mh2.ptr() == 1);
    MemoryHandler< int > mh3(MemHandlerFactory(3));
    assert(*mh3.ptr() == 3);
    MemoryHandler< int > mh4;
    mh4 = mh3;
    assert(mh3.ptr() == 0);
    assert(*mh4.ptr() == 3);

    //The following is not supported and results in
    //compilation errors
    //const MemoryHandler<int> cmh(new int(66));
    // /*const*/ MemoryHandler<int> mh5(cmh);

    //Also trying to insert an object into an std::vector
    //is not supported due to the lack of a copy constructor accepting
    //a constant reference
    //std::vector< MemoryHandler< int > > mhandlers1;
    //mhandlers1.push_back(MemoryHandler< int >(new int(1)));
    //std::vector< MemoryHandler< int > > mhandlers2(1);
    //MemoryHandler< int > mh(new int(1));
    //mhandlers2[0] = mh;
}


//------------------------------------------------------------------------------
int main(int, char**) {
    PrintCppStd();
    Test();
    return 0;
}