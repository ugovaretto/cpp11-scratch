#include <iostream>
#include <memory> //unique_ptr etc.
#include <cassert>
#include <stdexcept>

db.insert(key1/)

IMPORTANT: HAVE GRAPH NODE HOLD REFERENCES TO :
- INPUT DATA SOURCES
- INPUT NODES
- OUTPUT SOURCE
- OUTPUT NODE

typedef const std::type_info* type_t;

#if CHECK_TYPES
#ifndef CUSTOM_TYPE_CHECKER
template < typename T >
void check_type(type_t t) {
    if(&typeid(T) != t) {
        throw(std::bad_cast("Invalid cast"));
    }
}
#endif
#else
template < typename T >
void check_type(type_t) {}
#endif


struct data_t {
    struct i_data_t {
        virtual i_data_t* copy() const = 0;
        template < typename T >
        const T& get() const {
            return static_cast< const data_impl_t< T >& >(*this).get();
        }
    };
    template < typename T >
    struct data_impl_t : i_data_t {
        i_data_t* copy() const {
            return new(data_impl_t< T >(*this));
        }
        const T& get() const {
            return data_;
        }
        T data_;
    };    
    template < typename T >
    data_t(const T& d) : type_(&typeid(T)), data_(new data_impl_t< T >(d)) {}
    data_t(const data_t& d) : data_(d.data_.copy()) {}
    data_t(data_t&&) = default;
    template< typename T >
    const T& get<T>() const {
        check_type< T >(type_);
        return data_->get< T >();
    }    
    std::unique_ptr< i_data_t > data_;
    type_t type_;
};

using const_data_array_t = 
    std::vector< std::const_reference_wrapper< data_t > >;

struct task_node_t {  
    action_t a;
    const_task_node_array_t in;
    task_node_array_t out; 
    data_t out;
    binder_t call;
    bool enabled;   
};

struct b {
    operator(action_t& a, array_data_in_t& ad, data_t& out) {
        const tuple_t< int, float >& t = to_tuple_ref(ad);
        
    }
};

execute(task_node_t& tn, executor& e) {
    future = e(call, a, tn.in, tn.out);
    for(auto& next; tn.out)
        if(next.enabled)
            execute(next, e);
}

bind(binder_t ) {
    call(a, in, out);
    for(auto& next; out)
        if(next.enabled)
            execute(next);
}

connect(const task_node_t& in, task_node_t out, pos) {
    out.in[pos] = std::cref(in);
}

template < typename T >
task_node_t make_task_node(T t) {
    return task_node_t();
}

void execute() {
   execute(tn.action, )
}

//------------------------------------------------------------------------------
int f(int a, int b) {
    return a + b;
}

int gen_a() {
    return 2;
}

int gen_b() {
    return 3;
}

void print(int i) {
    std::cout << i << std::endl;
}

//------------------------------------------------------------------------------
void test_task_graph() {
    task_node_t t0  = make_task_node();
    task_node_t t11 = make_task_node(gen_a);
    task_node_t t12 = make_task_node(gen_b);
    task_node_t t2  = make_task_node(f);
    taks_node_t t3  = make_task_node(print);

    connect(t11, t2);
    connect(t12, t2);
    connect(t2,  t3);

    execute(2, t0); //execute with two threads starting at t0
}

int main(int, char**) {
    test_task_graph();
    return 0;
}


