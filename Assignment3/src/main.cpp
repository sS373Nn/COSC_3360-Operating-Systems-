#include "functions.hpp"

int main(int argc, char* argv[]){

    int memory_size;
    int look_ahead;

    int pid;
    Pipes message_pipes;
    Semaphores replacement_type_sems;

    read_args(argv, memory_size, look_ahead);
    
    execute_replacement_strategies(pid, look_ahead, memory_size, message_pipes, replacement_type_sems);

    std::string first_token = get_token();

    write_to_pipes(message_pipes, first_token);
    post_read_semaphores(replacement_type_sems);

    while(first_token != ""){
        first_token = get_token();
        wait_ready_semaphores(replacement_type_sems);
        write_to_pipes(message_pipes, first_token);
        post_read_semaphores(replacement_type_sems);
    }

    return 0;
}
