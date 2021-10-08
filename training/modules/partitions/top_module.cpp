export module top_module;
import<iostream>;
export import : top_module.child_1_module;
import : top_module.child_2_module;

export namespace top_module {
void Print() {
    std::cout << "Top> Hello from top module" << std::endl;
    std::cout << "Top> calling child_2_module..." << std::endl;
    child_2_module::Print();
}
}  // namespace top_module