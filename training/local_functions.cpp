// Local functions & local context
// Author Ugo Varetto
//
// Shows how to use a local struct with static methods to create local
// C functions, not supported by the C++ standard otherwise.
// GCC and Clang do have their own very different custom extensions for 
// creating local functions in a non portable way in C code.
// The method shown here is standard compliant, but makes use of thread_local
// instead of static.
//
// C++ 2003
// "\1. A class can be defined within a function definition; such a class is
// called a local class. The name of a local class is local to its enclosing
// scope. The local class is in the scope of the enclosing scope, and has the
// same access to names outside the function as does the enclosing function.
// Declarations in a local class can use only type names, static variables,
// extern variables and functions, and enumerators from the enclosing scope."
//
// thread_local results in the same behaviour as static but it's per-thread
//

#include <iostream>
#include <vector>

using namespace std;

//------------------------------------------------------------------------------
// Simple stateful function: at each invocation a new value is returned.
using Counter = int (*)();

Counter GenCounter(int start) {
    thread_local int val = start;
    struct __ {
        static int _() { return val++; }
    };
    return __::_;
}

//------------------------------------------------------------------------------
// Assigning a counter to multiple variable results each variable calling the
// same function, it is therefore required to use unique IDs to indentify
// different counters
template <int ID = 0>
Counter GenCounterUnique(int start) {
    thread_local int val = start;
    struct __ {
        static int _() { return val++; }
    };
    return __::_;
}

//------------------------------------------------------------------------------
// Same as above but with an automatic compile-time ID generator
#define NextID __COUNTER__

// Update counter value with custom callback
using NextCount = int (*)(int);

template <int ID = 0>
Counter GenCounterIndirect(int start, NextCount next) {
    thread_local int val = start;
    thread_local auto f = next;
    struct __ {
        static int _() {
            val = f(val);
            return val;
        }
    };
    return __::_;
}

//------------------------------------------------------------------------------
// Generic function to create generators.
// 1. Create initial context instance and store function to apply to context
// 2. At each invocation return value from passed value and stored context
//    and optionally updated context
template <int ID, typename Ctx, typename RetT, typename T>
RetT (*MakeGen(RetT (*f)(Ctx&, T), const Ctx& initial = Ctx()))(T) {
    thread_local Ctx ctx = initial;
    thread_local RetT (*F)(Ctx&, T) = f;
    ctx = initial;
    struct __ {
        static RetT _(T v) { return F(ctx, v); }
    };
    return __::_;
}

//------------------------------------------------------------------------------
// Generate counter and invoke twice
void RunCounter1() {
    auto count = GenCounter(1);
    cout << count() << " " << count() << endl;
}

// Generate countes and assign to two variable, it's the same counter instance;
// when a new counter is generated it will change the value of the previous
// one
void RunCounter2() {
    auto count1 = GenCounter(1);
    auto count2 = GenCounter(4);
    cout << count1() << " " << count2() << endl;
}

// Generate unique counters
void RunCounterUnique() {
    auto count1 = GenCounterUnique<1>(1);
    auto count2 = GenCounterUnique<2>(4);
    cout << count1() << " " << count2() << endl;
}

// 
void RunCounterIndirect() {
    auto count = GenCounterIndirect(2, [](int v) { return 2 * v; });
    cout << count() << " " << count() << " " << count() << endl;
}

//------------------------------------------------------------------------------
struct Ray {
    double dir[3] = {0., 0., -1.};
    double p[3] = {0., 0., 0.};
};
ostream& operator<<(ostream& os, const Ray& r) {
    os << "position: " << r.p[0] << ' ' << r.p[1] << " " << r.p[2] << " "
       << "direction: " << r.dir[0] << ' ' << r.dir[1] << ' ' << r.dir[2];
    return os;
}

Ray Advance(Ray& r, double step) {
    r.p[0] += r.dir[0] * step;
    r.p[1] += r.dir[1] * step;
    r.p[2] += r.dir[2] * step;
    return r;
}

// Created generators updating position of Ray instance with different step
// sizes 
void RunMakeGen() {
    auto rayMarch = MakeGen<1>(Advance);
    auto rayMarch2 = MakeGen<2>(Advance);
    for (int i = 0; i != 3; ++i) {
        cout << rayMarch(1) << endl;
        cout << rayMarch2(0.2) << endl;
        cout << endl;
    }
}
//------------------------------------------------------------------------------
// OOP by storing instance inside function and interacting through struct
// with function pointers.
struct Vec3D {
    double xyz[3] = {0., 0., 0.};
};

struct ISect {
    bool isect = false;
    Vec3D pos;
    Vec3D norm;
    Vec3D texCoord;
};

// "Base" class just a container of C function pointers accessing local
// data in functions.
struct Object {
    ISect (*Intersect)(const Ray& ray) = nullptr;
    void (*CheckedApply)(void*,const std::type_info&) = nullptr; // throw
    void* (*CheckedGet)(const std::type_info&) = nullptr; // throw
    template <typename T>
    void Apply(void (*f)(T*)) {
      CheckedApply(reinterpret_cast<void*>(f), typeid(T));
    }
    template <typename T>
    T& Get() {
      T& data = *reinterpret_cast<T*>(CheckedGet(typeid(T)));
      return data;
    }
};

// Object Type ID
enum ObjectType {
    EMPTY,
    SPHERE,
    PLANE
};

// Sphere
struct Sphere {
    ObjectType type = SPHERE;  // check type at run-time
    double radius = 1.;
    Vec3D pos;
};

// Plane
struct Plane {
    ObjectType type = PLANE;  // check type at run-time
    Vec3D normal = {0.,1.,0.};
    Vec3D pos;
};

// Create Object containing C function pointers to access the Sphere
// instance stored within the function
template <int ID>
Object MakeSphere(double radius, const Vec3D& position) {
    thread_local Sphere s = {SPHERE, radius, position};
    using ApplyFun = void (*)(Sphere*);
    struct __ {
        static ISect Intersect_(const Ray& ray) { return ISect(); }
        static void Apply_(void* f, const std::type_info& ti) {
            if(!f || ti != typeid(Sphere)) throw std::bad_cast();
            auto fun = reinterpret_cast<ApplyFun>(f);
            fun(&s);
        }
        static void* Get_(const std::type_info& ti) {
            if(ti != typeid(Sphere)) throw std::bad_cast();
            return &s;
        }
    };
   return {.Intersect = __::Intersect_, .CheckedApply = __::Apply_,
           .CheckedGet = __::Get_};
};

// Create Object containing C function pointers to access the Plane
// instance stored within the function
template <int ID>
Object MakePlane(const Vec3D& norm, const Vec3D& position) {
    thread_local Plane p = {PLANE, norm, position};
    using ApplyFun = void (*)(Plane*);
    struct __ {
        static ISect Intersect_(const Ray& ray) { return ISect(); }
        static void Apply_(void* f, const std::type_info& ti) {
            if(!f || ti != typeid(Sphere)) throw std::bad_cast();
            auto fun = reinterpret_cast<ApplyFun>(f);
            fun(&p);
        }
        static void* Get_(const std::type_info& ti) {
            if(ti != typeid(Plane)) throw std::bad_cast();
            return &p;
        }
    };
   return {.Intersect = __::Intersect_, .CheckedApply = __::Apply_,
           .CheckedGet = __::Get_};
};

// Store a Sphere and Plane inside a vector, update Sphere and print
void RunMakeObjects() {
  vector<Object> objects;
  objects.push_back(MakeSphere<1>(1., {{0.,0.,0.}}));
  objects.push_back(MakePlane<1>({{0.,1.,0.}}, {{0.,0.,0.}}));
  Sphere& s = objects[0].Get<Sphere>();
  cout << s.pos.xyz[0] << " " << s.pos.xyz[1] << " " << s.pos.xyz[2] << endl;
  void (*f)(Sphere*) = [](Sphere* s) {s->pos = {{1.,1.,1.}};};
  objects[0].Apply(f); // passing a lambda in line is detected as lambda
                       // passing an lvalue converts it to C function pointer
  s = objects[0].Get<Sphere>();
  cout << s.pos.xyz[0] << " " << s.pos.xyz[1] << " " << s.pos.xyz[2] << endl;

}
//------------------------------------------------------------------------------
// Use C++ lambda instead of struct + static method in case a single function
// pointer is to be returned
int (*TestLambda(int init))() {
    thread_local int c = init;
    return [] { return c++; };
}

// Test invocation of generator with lambda
void RunTestLambda() {
    auto c = TestLambda(10);
    cout << c() << " " << c() << " " << c() << endl;
}

//------------------------------------------------------------------------------
int main(int, char**) {
    RunCounter1();
    RunCounter2();
    RunCounterUnique();
    RunCounterIndirect();
    RunMakeGen();
    RunTestLambda();
    RunMakeObjects();
    return 0;
}