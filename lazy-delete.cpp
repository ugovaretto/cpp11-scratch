//Author: Ugo Varetto
//Lazy delete:
//first call to delete invokes destructor and
//replaces instance with "DeletedType" instance
//second call to delete actually frees allocated memory
//useful in cases where you have multiple references to an
//object and want to implement a garbage-collection step
//to remove all the references pointing to empty objects

#include <cassert>
#include <iostream>

// derive from Deleted/Empty/Deletable Type
struct DeletedType {
    virtual bool Empty() const { return true; }
    virtual ~DeletedType() { std::cout << "~DeletedType\n";}
};

struct Type : DeletedType {
    virtual bool Empty() const { return false; }
    void operator delete(void* p) {
       std::cout << "Type::operator delete\n"; 
       new (p) DeletedType;      
    }
    ~Type() { std::cout << "~Type\n"; }
};

int main(int, char**) {
    DeletedType* t = new Type;
    DeletedType& r = *t;
    assert(not r.Empty());
    //replace instance with a DeletedType instance
    delete t;
    assert(r.Empty());
    //physically free memory
    delete t;
    return 0;
}