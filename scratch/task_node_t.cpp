
task nodes shall have
- data output
- progress reporter 0 to 100 
- message reporter read message string
- status reported: executing/error

- add optional allocator for all types

task_node_t tn;
tn.action = make_action_wrapper(tn.promise, fun, param1, param2,...);
execute:

 task.promise.set_value(task.binder(task.action))

connect:
  target_task.in[channel].push_back(source_task.out)

create wrappers for futures and promises to be stored into
data_t types: this allows to have task functions executed in separate threads
as needed and synchronize access through data_t get method

propagate update flag: if flag set in node task propagate it to next tasks

consider pull operations: start execution from last task and cause previous
tasks to execute until a non-dirty task is found

consider using strings and hash maps for task channels

std::unordered_map< string, std::vector< std::reference_wrapper< const data_t > >;

wrap everything with an execution_engine_t holding a reference to active tasks source_task

consider adding history to data_t: add method push which adds an object into
an internal history array


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