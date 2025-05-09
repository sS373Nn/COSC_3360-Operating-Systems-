#include "functions.hpp"

int main(int argc, char* argv[]){
    
    int number_of_processes;
    std::vector<std::vector<int>> user_matrix = build_user_matrix(number_of_processes);
    ////

    //Part One//
    parent_process_part_one(number_of_processes, user_matrix);

    return 0;
}
