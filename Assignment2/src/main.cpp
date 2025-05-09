#include "functions.hpp"

int main(int argc, char* argv[]){

    std::vector<std::string> file_names = file_name_read_from_function_call(argv);

    std::vector<std::string> process_info = read_file_into_vector(file_names[0]);
    std::vector<std::string> resource_info = read_file_into_vector(file_names[1]);
    
    std::cout << "EDF:\n\n";

    banker("EDF", process_info, resource_info);

    std::cout << std::endl;
    
    execute_LLF_with_execv(file_names[0], file_names[1]);

    return 0;
}
