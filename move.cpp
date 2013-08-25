#include <iostream>
#include <cassert>

//template <typename T>
//struct ResourceProxy {
//      T* res_;
//};

template< typename T, 
          typename DisposePolicy, 
          typename ValidPolicy,
          typename ResetPolicy>
class ResourceHandler : 
    DisposePolicy, 
    ValidPolicy,
    ResetPolicy {
    struct ResourceProxy {
        T* res_;
    };      
public:
    ResourceHandler(ResourceHandler& rh) : res_(rh.res_) {
        rh.res_ = ResetPolicy::Reset(rh.res_);
    }	
    ResourceHandler(T res) : res_(res) {}
    ResourceHandler(ResourceProxy rm) : res_(*rm.res_) {
        std::cout << this << " Creation from proxy" << std::endl;
        *rm.res_ = ResetPolicy::Reset(*rm.res_);
    }
    ResourceHandler& operator=(ResourceHandler& rh) {
        swap(ResourceHandler(rh));
        return *this;
    }
    ResourceHandler& operator=(ResourceProxy rp) {
        ResourceHandler(rp).swap(*this);
        return *this;
    }
    void swap(ResourceHandler& rh) {
        std::swap(res_, rh.res_);
    }
    operator ResourceProxy()  {
        std::cout << this << " Conversion to proxy" << std::endl;
        ResourceProxy  p; p.res_ = &res_;
        return p;
    }
    T& res() { return res_; }
    const T& res() const { return res_; }
	~ResourceHandler() {
        if(ValidPolicy::Valid(res_))
            DisposePolicy::Dispose(res_); 
	}
private:
    T res_;    
};
struct PointerDispose {
    template < typename T >
    void Dispose(T* p) {
        delete p;
    }
};
struct PointerValid {
    template < typename T >
    bool Valid(T* p) {
        return p != 0; 
    }
};
struct PointerReset {
    template < typename T >
    T* Reset(const T*) {
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



