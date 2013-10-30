#include <iostream>
#include <memory> //unique_ptr etc.

struct task_node_t {  
    template < typename A >
    task_node_t(const A& a) : action_(new A(a)) {}   
    template < typename RetT, typename...Args >
    RetT do(Ret& ret, Args...args) {
        return action(args...);
    }
    struct i_action {
        void 
    };
    const type_info*     
};

template < typename T >
task_node_t make_task_node(T t) {
    return task_node_t();
}

void execute(int nt, task_node_t& tn) {
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


