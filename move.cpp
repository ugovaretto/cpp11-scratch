#include <iostream>
#include <cassert>

//TODO: 
// USE SWAP IN ASSIGNMENT OPERATORS
// MAKE POLICIES TEMPLATED ON TYPES

template <typename T>
struct ResourceProxy {
      T* res_;
};

template< typename T, 
          typename DisposePolicy, 
          typename ValidPolicy,
          typename ResetPolicy>
class ResourceHandler : 
    DisposePolicy, 
    ValidPolicy,
    ResetPolicy {
public:
    ResourceHandler(ResourceHandler& rh) : res_(rh.res_) {
        rh.res_ = T(ResetPolicy::Reset());
        //need to qualify the Reset method: since it does not depend
        //on any template parameter the compiler will complain that a 
        //declaration must be available
    }	
    ResourceHandler(T res) : res_(res) {}
    ResourceHandler(ResourceProxy<T> rm) : res_(*rm.res_) {
        std::cout << this << " Creation from proxy" << std::endl;
        *rm.res_ = T(ResetPolicy::Reset());
        //need to qualify the Reset method: since it does not depend
        //on any template parameter the compiler will complain that a 
        //declaration must be available
    }
    ResourceHandler& operator=(ResourceHandler& rh) {
        if(Valid(res_)) Dispose(res_);
        res_ = rh.res_;
        rh.res_ = T(ResetPolicy::Reset());
        return *this;
    }
    ResourceHandler& operator=(ResourceProxy<T> rp) {
        if(Valid(res_)) Dispose(res_);
        res_ = *rp.res_;
        *rp.res_ = T(ResetPolicy::Reset());
        return *this;
    }
    operator ResourceProxy<T>() /*const*/ {
        std::cout << this << " Conversion to proxy" << std::endl;
        ResourceProxy<T>  p; p.res_ = &res_;
        return p;
    }
    T& res() { return res_; }
    const T& res() const { return res_; }
	~ResourceHandler() {
        if(Valid(res_))
            Dispose(res_); 
	}
private:
    T res_;    
};
struct PointerDispose {
    void Dispose(void* p) {
        delete p;
    }
};
struct PointerValid {
    bool Valid(void* p) {
        return p != 0; 
    }
};
struct PointerReset {
    void* Reset() {
        return 0;
    }
};



template < typename T >
struct PointerHandler {
    typedef ResourceHandler<T*, PointerDispose, PointerValid, PointerReset>
            type;
};

typedef PointerHandler<int>::type IH;

IH foo() {
    return IH(new int(4));
}

int main(int, char**) {
    PointerHandler<int>::type pi(new int(2));
    assert(*pi.res() == 2);
    PointerHandler<int>::type pi2(pi);
    assert(pi.res() == 0);
    assert(*pi2.res() == 2);
    IH pi3 = foo();
    pi3 = foo();
    return 0;
}



