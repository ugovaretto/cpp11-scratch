#include <cassert>   //assert
#include <algorithm> //swap
#include <vector>

//------------------------------------------------------------------------------
#if __cplusplus < 201103L  // C++ 98
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
#else // C++ 11
template <typename T>
class MemoryHandler {
public:
    MemoryHandler(MemoryHandler&& mh) : ptr_(mh.ptr_) {
        mh.ptr_ = 0;
    }
    MemoryHandler(const MemoryHandler&) = delete;
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
        MemoryHandler(std::move(mh)).Swap(*this);
        return *this;
    }
    MemoryHandler& operator=(const MemoryHandler& mh) = delete;
private:
    void Swap(MemoryHandler& mh) {
        std::swap(mh.ptr_, ptr_);
    } 
private:
    T* ptr_;
};
// generic handler
template <typename T,
          typename ValidationPolicy,
          typename ResetPolicy,
          typename ReleasePolicy>
class Handler {
public:
    Handler(Handler&& h) : res_(h.res_) {
        ReleasePolicy::Reset(h.res_);
    }
    Handler(const Handler&) = delete;
    explicit Handler(T res = T()) : res_(res) {
        ReleasePolicy::Reset(res_);
    }
    T& get() { return res_; }
    const T& get() const { return res_; }
    ~Handler() { 
        if(ValidationPolicy::Valid(res_)) {
            ReleasePolicy::Release(res_);
        }
    }
    Handler& operator=(Handler&& h) {
        Handler(h).Swap(*this);
        return *this;
    }
    Handler& operator=(Handler& h) {
        Handler(std::move(h)).Swap(*this);
        return *this;
    }
    Handler& operator=(const Handler&) = delete;
private:
    void Swap(Handler& h) {
        std::swap(h.res_, res_);
    } 
private:
    T res_;
};
#endif

//------------------------------------------------------------------------------
MemoryHandler< int > MemHandlerFactory(int i) {
    return MemoryHandler< int >(new int (i));
}

void Test() {
    MemoryHandler< int > mh1(new int(1));
    assert(*mh1.ptr() == 1);
    MemoryHandler< int > mh2(std::move(mh1));
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
    //a constant reference; it is supported in the C++11 version however.
#if __cplusplus >= 201103L
    std::vector< MemoryHandler< int > > mhandlers1;
    mhandlers1.push_back(MemoryHandler< int >(new int(4)));
    assert(*mhandlers1.back().ptr() == 4);
    std::vector< MemoryHandler< int > > mhandlers2(1);
    MemoryHandler< int > mh(new int(1));
    mhandlers2[0] = mh;
    assert(*mhandlers2[0].ptr() == 1);
#endif    
}


//------------------------------------------------------------------------------
int main(int, char**) {
    Test();
    return 0;
}