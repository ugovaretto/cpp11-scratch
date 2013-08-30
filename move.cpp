//Sample implementation of resource handler with move semantics
//Author: Ugo Varetto

#include <iostream>
#include <cassert>
#include <vector>
#include <stdexcept>


//Resoure handler with resource-only wrapper used for move operations
#ifdef WRAP_RESOURCE
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
        ResourceProxy  p = {&res_};
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
#else //proxy wraps entire object
//Resource handler with proxy wrapper wrapping entire resource handler object;
//useful when resource handler objects contain additional data members that
//need to be copied, in this case a counter for the number of times the
//resource is moved
template< typename T, 
          typename DisposePolicyT, 
          typename ValidPolicyT,
          typename ResetPolicyT>
class ResourceHandler : 
    DisposePolicyT, 
    ValidPolicyT,
    ResetPolicyT {
    struct ResourceProxy {
        ResourceHandler* rh;
    };

public:    
    typedef DisposePolicyT DisposePolicy;
    typedef ValidPolicyT ValidPolicy;
    typedef ResetPolicyT ResetPolicy;
public:
    ResourceHandler(T res = T()) : res_(res), moves_(0) {
        res_ = ResetPolicy::Reset(res_);
    }
    ResourceHandler(ResourceProxy rm) : 
        res_(rm.rh->res_),
        moves_(rm.rh->moves_ + 1) {
        std::cout << this << " Creation from proxy" << std::endl;
        rm.rh->res_ = ResetPolicy::Reset(rm.rh->res_);
    }
    ResourceHandler& operator=(const ResourceProxy& rp) {
        std::cout << "Assignment" << std::endl;
        ResourceHandler(rp).swap(*this);
        return *this;
    }
    void swap(ResourceHandler& rh) {
         std::swap(res_, rh.res_);
         std::swap(moves_, rh.moves_);
    }
    operator ResourceProxy()  const {
        std::cout << this << " Conversion to proxy" << std::endl;
        ResourceProxy p = {const_cast< ResourceHandler* >(this)};
        return p;
    }
    void reset() {
        res_ = ResetPolicy::Reset(res_);
    }
    T& res() { return res_; }
    const T& res() const { return res_; }
    int moves() const { return moves_; }
    ~ResourceHandler() {
        if(ValidPolicy::Valid(res_))
            DisposePolicy::Dispose(res_);
    }
    template < typename U, typename D, typename V, typename R >
    friend
    typename ResourceHandler<U, D, V, R>::ResourceProxy
    move(ResourceHandler<U, D, V, R>& rm)  {
        typename ResourceHandler<U, D, V, R>::ResourceProxy rp = {&rm};
        return rp;
    } 
    //used ONLY to move from *temporary* object cast to const &
    template < typename U, typename D, typename V, typename R >
    friend
    typename ResourceHandler<U, D, V, R>::ResourceProxy
    move(const ResourceHandler<U, D, V, R>& crm) {
       ResourceHandler<U, D, V, R>& rm = 
            const_cast< ResourceHandler<U, D, V, R>& >(crm);
        typename ResourceHandler<U, D, V, R>::ResourceProxy rp;
        rp.rh = &rm;
        return rp;
    } 
private:
    T res_;
    int moves_;    
};
#endif

//------------------------------------------------------------------------------
//Policy implementation for pointer and number types

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

struct NumDispose {
    template < typename T >
    void Dispose(T n) {} 
    //could be close(n) for sockets or file descriptors
};
struct NumValid {
    template < typename T >
    bool Valid(T n) {
        return n != T(0); 
    }
};
struct NumReset {
    template < typename T >
    T Reset(T) {
        return T(0);
    }
};

//------------------------------------------------------------------------------
//helper typedefs for pointer and numerical types 
template < typename T >
struct PointerHandler {
    typedef ResourceHandler<T*, PointerDispose, PointerValid, PointerReset>
            type;
};

template < typename T >
struct NumericHandler {
    typedef ResourceHandler<T, NumDispose, NumValid, NumReset>
            type;
};

//------------------------------------------------------------------------------
typedef PointerHandler<int>::type IH;

IH foo() {
    return IH(new int(4));
}

void bar(const IH& ih) {
    std::cout << ih.res() << std::endl;
}

class Movable {
public:
    Movable(int* p) : data_(p), moved_(false) { dbg_instances++; }
    Movable(const Movable& m) : data_(0), moved_(false) {
        //only support the case of temporary returned by functions
        if(!m.moved()) throw std::logic_error("###");
        m.move_to(this);
        dbg_instances++;
    }
    Movable() : data_(0), moved_(false) { dbg_instances++; }
    ~Movable() {
        if(!moved_) {
            delete data_;
            dbg_destructors++;
        }
    }
    Movable& operator=(const Movable& m){
        m.move_to(this);
        return *this;
    }
    int& data() {
        if(moved_) throw std::logic_error("Already moved");
        return *data_;
    }
    const int& data() const {
        if(moved_) throw std::logic_error("Already moved");
        return *data_;
    }
    void update(int) {}
    bool moved() const { return moved_; }
private:
    void move_to(Movable* other) const {
        if(other == this) return;
        dbg_copies++;
        if(other == 0) throw std::runtime_error("NULL object");
        if(moved_) throw std::logic_error("Already moved");
        other->data_ = data_;
        moved_ = true;
    }
private:
    int* data_; 
    mutable bool moved_; 
public:
    static int dbg_destructors;
    static int dbg_copies;
    static int dbg_instances;     
};
int Movable::dbg_destructors = 0;
int Movable::dbg_copies = 0;
int Movable::dbg_instances = 0;

Movable MovableFactory(int i) {
    return Movable(new int(i));
}


//------------------------------------------------------------------------------
int main(int, char**) {
    try {
    Movable mvbl(MovableFactory(555));
    std::vector< Movable > movables;
    Movable mv(new int(2));
    Movable mv2;
    mv2 = mv;
    movables.push_back(mv2);
    assert(movables[0].data() == 2);
   }catch(const std::exception& e) {std::cout << e.what() << std::endl;}
       std::cout << "Resource destructions: " << Movable::dbg_destructors
                 << " Copies: " << Movable::dbg_copies 
                 << " Instances created: " << Movable::dbg_instances << std::endl;
//     PointerHandler<int>::type pi(new int(2));
//     assert(*pi.res() == 2);
//     PointerHandler<int>::type pi2((pi));
//     assert(pi.res() == 0);
//     assert(*pi2.res() == 2);
//     IH pi3 = foo();
//     pi3 = foo();
//     bar(move(IH(new int(5))));
//     std::vector< IH > phandlers(2);
// //     phandlers.push_back(move(pi2));
// //     assert(*phandlers.back().res() == 2);
//     phandlers[0] = pi2;
//     phandlers.push_back(move(IH(new int(123))));
//     assert(*phandlers.back().res() == 123);
// #ifndef WRAP_RESOURCE
//     assert(phandlers[0].moves() == 4);
//     PointerHandler<int>::type pi4(move(phandlers[0]));
//     assert(*pi4.res() == 2);
//     assert(pi2.res() == 0);
//     assert(pi4.moves() == 5);
//     assert(pi3.moves() == 2);
//     // foo -> temporary: 1 move
//     // temporary -> proxy
//     // proxy -> pre-existing instance = operator: +1 move
// #endif
    return 0;
}



