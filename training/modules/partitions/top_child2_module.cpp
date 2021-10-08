module top_module : top_module.child_2_module;
import<iostream>;

namespace top_module::child_2_module {
void Print() {
    std::cout << "Top.Child 2> Hello from hidden top.child_2 module" << std::endl;
}
}  // namespace top_module::child_2_module