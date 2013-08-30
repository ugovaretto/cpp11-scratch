#include <cassert>   //assert
#include <algorithm> //swap
#include <vector>

//------------------------------------------------------------------------------
#if 0
template <typename T>
class MemoryHandler {
private:
    struct Proxy {
        T* ptr;  
    };
public:
    MemoryHandler(const Proxy& p) : ptr_(p.ptr) {}
    MemoryHandler(MemoryHandler& mh) : ptr_(mh.ptr_) {
        mh.ptr_ = 0;
    }
    explicit MemoryHandler(T* ptr = 0) : ptr_(ptr) {}
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
        return *this;
    }
private:
    void Swap(MemoryHandler& mh) {
        std::swap(mh.ptr_, ptr_);
    } 
private:
    T* ptr_;
};
#else
template <typename T>
class MemoryHandler {
private:
    struct Proxy {
        T* ptr;  
    };
public:
    MemoryHandler(MemoryHandler&& mh) : ptr_(mh.ptr_) {
        mh.ptr_ = 0;
    }
    MemoryHandler(MemoryHandler& mh) : ptr_(mh.ptr_) {
        mh.ptr_ = 0;
    }
    explicit MemoryHandler(T* ptr = 0) : ptr_(ptr) {}
    T* ptr() { return ptr_; }
    ~MemoryHandler() { 
        delete ptr_; //it is fine to call 'delete 0'
    }
    MemoryHandler& operator=(MemoryHandler&& mh) {
        MemoryHandler(mh).Swap(*this);
        return *this;
    }
    MemoryHandler& operator=(MemoryHandler& mh) {
        MemoryHandler(mh).Swap(*this);
        return *this;
    }
private:
    void Swap(MemoryHandler& mh) {
        std::swap(mh.ptr_, ptr_);
    } 
private:
    T* ptr_;
};
#endif

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
    std::vector< MemoryHandler< int > > mhandlers1;
    mhandlers1.emplace_back(std::move(MemoryHandler< int >(new int(4))));
    std::vector< MemoryHandler< int > > mhandlers2(1);
    MemoryHandler< int > mh(new int(1));
    mhandlers2[0] = mh;
}


//------------------------------------------------------------------------------
int main(int, char**) {
    PrintCppStd();
    Test();
    return 0;
}