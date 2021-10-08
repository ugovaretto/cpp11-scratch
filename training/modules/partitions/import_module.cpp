import top_module;
import <iostream>;

int main(int argc, const char** argv) {
    std::cout <<  std::endl;
    std::cout << "Main> Calling top_module from Main..." << std::endl;
    top_module::Print();
    std::cout << "Main> Calling child 1 module from Main..." << std::endl;
    top_module::child_1_module::Print();
    std::cout <<  std::endl;
    return 0;
}