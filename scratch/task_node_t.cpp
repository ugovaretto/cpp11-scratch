
task nodes shall have
- data output
- progress reporter 0 to 100 
- message reporter read message string
- status reported: executing/error

- add optional allocator for all types

task_node_t tn;
tn.action = make_action_wrapper(tn.promise, fun, param1, param2,...);

use wrapper<X> -> see herb sutter's c++ concurrency
execute(task_node_t& tn, int sid, executor_t& e) {
    if(!tn.executed || !try_acquire(tn.lock)) {
        acquire(tn.lock)
        if(tn.sid == sid) {
            tn.action();
            tn.executed = true;
        } else {
            tn.promise = tn.create_promise();
            tn.future = tn.promise.get_future();
            e(execute, tn.sid, executor);
        }
    }
    for(auto& n; tn.next_nodes)
        execute(n, sid, e);
}