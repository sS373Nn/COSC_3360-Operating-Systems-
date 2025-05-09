#include "functions.hpp"

int main(int argc, char* argv[]){
    
    int number_of_processes;
    std::vector<std::vector<int>> user_matrix = build_user_matrix(number_of_processes);
    ////

    //Part One//
    //parent_process_part_one(number_of_processes, user_matrix);

    //Part Two//
    //Initialize our semaphore
    sem_t sem;
    sem_init(&sem, 1, 1);

    //Create outputPipe, pass pipe and sem into main_process
    int* output_pipe = new int[2];

    if (pipe(output_pipe) == -1){
        std::cout << "Output pipe creation failed\n";
        exit(0);
    }

    parent_process(number_of_processes, user_matrix, sem, output_pipe);

    //Destroy our semaphore
    //Sloppy, but if it works...
    for (int process = 0; process < number_of_processes; process++){
        wait(NULL);
    }
    sem_destroy(&sem);

    //Destroy output pipe
    if (output_pipe) {
        close(output_pipe[0]);
        close(output_pipe[1]);
        delete[] output_pipe;
    }

    return 0;
}
