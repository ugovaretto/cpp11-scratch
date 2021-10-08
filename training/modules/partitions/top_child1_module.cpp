export module top_module : top_module.child_1_module;
import<iostream>;

export namespace top_module::child_1_module {
void Print() { std::cout << "Top.Child 1> Hello from top.child_1 module" << std::endl; }
}  // namespace top_module::child_1_module