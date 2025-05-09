#include "functions.hpp"

#include <string>
#include <utility>
#include <sstream>
#include <cmath>
#include <array>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <unordered_set>

#ifdef _WIN32

int fork(){
    std::cout << "[MOCK] Simulating fork() in Windows\n";
    return 0;
}

int pipe(int* pipe_file){
    std::cout << "[MOCK] Simulating pipe() in Windows\n";
    return 0;
}

void close(int pipe_file_part){
    std::cout << "[MOCK] Simulating close() in Windows\n";
}

void write(int pipe_file_part, const char serialized_vector[], int size){
    std::cout << "[MOCK] Simulating write() in Windows\n";
}

void read(int pipe_file_part, char buffer[], int size){
    std::cout << "[MOCK] Simulating read() in Windows\n";
}

#else

#endif

PipeManager::PipeManager(int number_of_pipes){
    for (int pipe_location = 0; pipe_location < number_of_pipes; pipe_location++){
        int* pipe_file_desc = new int[2];
        
        if (pipe(pipe_file_desc) == -1){
            exit(0);
        }

        pipes.push_back(pipe_file_desc);
    }
}

PipeManager::~PipeManager(){
    for (int* pipe_file_desc : pipes){
        if (pipe_file_desc) {
            close(pipe_file_desc[0]);
            close(pipe_file_desc[1]);
            delete[] pipe_file_desc;
        }
    }
}

//TEST FUNCTIONS AND LEGACY CODE//

//Input Processing//

void print_command_line_args(int args, char** parts){

    for (int i = 0; i < args; i++){
        std::cout << "Argument " << i << ": " << parts[i] << std::endl;
    }

}

std::string read_matrix(int args, char** parts){
    if (args == 1){
        std::cout << "Invalid entry, not enough arguments passed." << std::endl;
        return "";
    }

    return parts[1];
}

void show_matrix_of_integers(std::vector<int> matrix){
    int k = sqrt(matrix.size());
    if (k * k == matrix.size()){
        for (int row = 0; row < k; row++){
            for (int col = 0; col < k; col++){
                std::cout << matrix[row * k + col] << " ";
            }
            std::cout << std::endl;
        }
    }

    else {
        std::cout << "Improper matrix size." << std::endl;
    }

    return;
}

std::vector<std::string> read_file_into_vector(std::string file_name){
    std::ifstream file(file_name);
    std::vector<std::string> file_contents;

    if (!file){
        std::cerr << "Error: Could not open file '" << file_name << "'" << std::endl;
        return file_contents;
    }

    std::string line;
    while (std::getline(file, line)){
        file_contents.push_back(line);
        std::cout << line << " added to vector" << std::endl;
    }

    file.close();
    return file_contents;
}

std::vector<std::vector<int>> create_matrix_from_file_contents(std::vector<std::string> file_contents){
    std::vector<std::vector<int>> matrix;

    for (size_t i = 0; i < file_contents.size(); i++){
        matrix.push_back(vectorize_string_to_int(file_contents[i]));
    }

    //Modify matrix_vector function to work with file input examples

    return matrix;
}

//Read stdin vector line from user
//REMOVE//
//Unused
std::string read_matrix_line(){
    std::string matrix_line;

    std::cout << "Enter matrix line:\n";
    std::getline(std::cin, matrix_line);

    std::cout << "You entered:\n" << matrix_line << std::endl;

    return matrix_line;
}

//Read stdin string from user
//REMOVE//
//Unused
std::string read_string(){
    std::string input_string;
    std::cout << "Please enter a word to process:\n";
    std::cin >> input_string;
    std::cout << "You wrote: " << input_string << std::endl;

    std::transform(input_string.begin(), input_string.end(), input_string.begin(), ::tolower);

    return input_string;
}

//Close write ends after use
//REMOVE// Added close statement to read/write functions
void close_write_ends_after_use(std::vector<int> used_pipe_locations, std::vector<int*> pipes){
    for (int index = 0; index < used_pipe_locations.size(); index++){
        close(pipes[used_pipe_locations[index]][1]);
    }
}

//Close read ends after use
//REMOVE// Added close statement to read/write functions
void close_read_ends_after_use(std::vector<int> used_pipe_locations, std::vector<int*> pipes){
    for (int index = 0; index < used_pipe_locations.size(); index++){
        close(pipes[used_pipe_locations[index]][0]);
    }
}

///////////////////////
////END LEGACY CODE////
///////////////////////

//Main Process Part I//
void parent_process_part_one(int number_of_processes, std::vector<std::vector<int>> process_matrix){
    int process_id;

    std::cout << "Part One Output:\n";

    for (int process = 0; process < number_of_processes; process++){
        process_id = fork();
        
        if (process_id == -1){
            std::cout << "Error creating process using fork." << std::endl;
        }

        if (process_id == 0){
            std::cout << "I am a process with " << count_incoming_pipes_for_process(process, process_matrix) << " incoming and " << count_outgoing_pipes_for_process(process_matrix[process]) << " outgoing pipes." << std::endl;
            exit(0); //Works properly in Linux, exits early in Windows
        }
    }

    for (int processes = 0; processes < number_of_processes; processes++){
        wait(NULL); //Uncomment for Linux
    }
}

//Main Process Part II//
void parent_process(int number_of_processes, std::vector<std::vector<int>> process_matrix, sem_t &semaphore, int* output_pipe){
    int process_id;
    std::string child_word;
    std::vector<bool> input_process_tracking_vector = input_process_tracker(process_matrix);
    int input_process_count = count_input_processes(input_process_tracking_vector);
    std::vector<std::string> words_list = create_words_list(input_process_count);
    int number_of_pipes_needed = count_pipes_to_create(process_matrix);
    PipeManager pipeManager(number_of_pipes_needed);
    std::vector<int> incoming_pipe_index;
    std::vector<int> outgoing_pipe_index;

    for (int process = 0; process < number_of_processes; process++){
        if (input_process_tracking_vector[process]){
            child_word = grab_word(words_list);
        }

        process_id = fork();
        
        if (process_id == -1){
            std::cout << "Error creating process using fork." << std::endl;
        }

        if (process_id == 0){
            int incoming_pipes = count_incoming_pipes_for_process(process, process_matrix);
            int outgoing_pipes = count_outgoing_pipes_for_process(process_matrix[process]);
            std::vector<std::string> process_words_list;

            if (input_process_tracking_vector[process]){
                process_words_list.push_back(child_word);
            }

            if (incoming_pipes > 0){
                close(output_pipe[0]);
                incoming_pipe_index = incoming_pipe_index_vector(process, process_matrix);
                
                close_unused_read_ends(incoming_pipe_index, pipeManager.pipes);
                
                //Read from open pipes
                process_words_list = read_from_incoming_pipes(incoming_pipe_index, pipeManager.pipes);
            } else {
                //Close all incoming pipes
                close(output_pipe[0]);
                for (int incoming_pipe = 0; incoming_pipe < number_of_pipes_needed; incoming_pipe++){
                    close(pipeManager.pipes[incoming_pipe][0]);
                }
            }
    
            if (outgoing_pipes > 0){
                close(output_pipe[1]);
                outgoing_pipe_index = outgoing_pipe_index_vector(process, process_matrix);
                
                //Close unused outgoing pipes
                close_unused_write_ends(outgoing_pipe_index, pipeManager.pipes);
                
                //Write to open pipes
                write_to_outgoing_pipes(outgoing_pipe_index, pipeManager.pipes, process_words_list);
            } else {
                //This is an output process
                //Dupe output pipe, close all pipes
                int output_pipe_write = dup(output_pipe[1]);
                close(output_pipe[1]);
                
                //Close all outgoing pipes
                for (int outgoing_pipe = 0; outgoing_pipe < number_of_pipes_needed; outgoing_pipe++){
                    close(pipeManager.pipes[outgoing_pipe][1]);
                }

                //write to output pipe
                std::vector<char> serialized_process_words_list = super_serialize(process_words_list);

                sem_wait(&semaphore);
                write(output_pipe_write, serialized_process_words_list.data(), serialized_process_words_list.size());
                close(output_pipe_write);
                sem_post(&semaphore);
            }
            
            exit(0); //Works properly in Linux, exits early in Windows
        }
    }

    //Call execv() here I think
    //Run a wait() function first?
    for (int wait_for_end = 0; wait_for_end < number_of_processes; wait_for_end++){
        wait(NULL);
    }
    //Delete sem (Handled in main())

    close(output_pipe[1]);

    process_id = fork();

    if (process_id == 0){
        //Build execv() input args
        std::string output_as_string = std::to_string(output_pipe[0]);
        std::string file_string = "./output_processor";
        char* output_pipe_read_end = const_cast<char*>(output_as_string.c_str());
        char* file_name = const_cast<char*>(file_string.c_str());
        char *args[] = {file_name, output_pipe_read_end, NULL};
        
        //Call execv()
        execv(args[0], args);

        //Unnecessary but it makes me feel better
        exit(0);
    }

    close(output_pipe[0]);
}
////

////Main Process Helper Functions////

std::vector<int> vectorize_string_to_int(std::string string_of_ints){
    std::stringstream ss(string_of_ints);

    std::vector<int> matrix;
    int digit;

    while (ss >> digit) {
        matrix.push_back(digit);
    }

    return matrix;
}

//Count incoming pipes
int count_incoming_pipes_for_process(int process_number, std::vector<std::vector<int>> matrix){
    int number_of_processes = matrix.size();
    int incoming_processes = 0;

    for (int i = 0; i < number_of_processes; i++){
        incoming_processes += matrix[i][process_number];
    }

    return incoming_processes;
}

//Count outgoing pipes
int count_outgoing_pipes_for_process(std::vector<int> process_row){
    int sum = 0;

    for (int intersection = 0; intersection < process_row.size(); intersection++){
        sum += process_row[intersection];
    }

    return sum;
}

//Count Input Processes
int count_input_processes(std::vector<bool> input_process_vector){
    int size = input_process_vector.size();
    int input_process_count = 0;

    for (int process = 0; process < size; process++){
        if (input_process_vector[process]){
            input_process_count++;
        }
    }

    return input_process_count;
}

//Check if process is input process
bool is_input_process(int process_number, std::vector<std::vector<int>> process_matrix){
    int size = process_matrix.size();
    int incoming_vertices = 0;

    for (int row = 0; row < size; row++){
        incoming_vertices += process_matrix[row][process_number];
    }

    return incoming_vertices == 0;
}

//Create bool vector to track input processes
std::vector<bool> input_process_tracker(std::vector<std::vector<int>> process_matrix){
    int size = process_matrix.size();
    std::vector<bool> input_process_vector;

    for (int row = 0; row < size; row++){
        input_process_vector.push_back(is_input_process(row, process_matrix));
    }

    return input_process_vector;
}

//Build process matrix from user input
std::vector<std::vector<int>> build_user_matrix(int &number_of_processes){
    std::vector<std::string> matrix_as_string_vector;
    std::vector<std::vector<int>> process_matrix;
    std::string accept_matrix;
    int matrix_size;
    
    while (accept_matrix != "y"){
        std::vector<std::string> input_matrix_string;
        accept_matrix = "";
        matrix_size = 0;
        
        std::cout << "How many processes will your matrix have?\n";
        std::cin >> matrix_size;
        std::cin.ignore();
        std::cout << "Input the whole matrix WITH spaces between digits AND newline char included\n";
        std::string matrix_line;

        for (int input = 0; input < matrix_size; input++){
            matrix_line = "";
            std::getline(std::cin, matrix_line);
            input_matrix_string.push_back(matrix_line);
        }

        std::cout << "Your matrix is:\n";
        for (int row = 0; row < input_matrix_string.size(); row++){
            std::cout << input_matrix_string[row] << std::endl;
        }
        std::cout << "Do you want to use this matrix?\ny/n\n";
        std::cin >> accept_matrix;
        std::cin.ignore();

        number_of_processes = matrix_size;
        matrix_as_string_vector = input_matrix_string;
    }

    for (int row = 0; row < matrix_as_string_vector.size(); row++){
        std::vector<int> new_row =vectorize_string_to_int(matrix_as_string_vector[row]);

        process_matrix.push_back(new_row);
    }

    return process_matrix;
}

//Create word_list vector from user input
std::vector<std::string> create_words_list(int number_of_input_processes){
    std::vector<std::string> words_list;

    std::cout << "Input one word per line WITH the newline character per input process\n";

    for (int word_order = 0; word_order < number_of_input_processes; word_order++){
        std::string word;
        std::getline(std::cin, word);
        words_list.push_back(word);
    }

    return words_list;
}

//Take word for the child process from words_list vector
std::string grab_word(std::vector<std::string>& vector_of_words){
    if (vector_of_words.empty()){
        return "";
    }

    std::string childs_word = vector_of_words[0];
    vector_of_words.erase(vector_of_words.begin());

    return childs_word;
}

//Turn vector into string to send through pipes
std::string serialize(std::vector<std::string> word_list){
    std::string serialized_vector = "";

    if (word_list.size() == 0){
        return serialized_vector;
    }

    for (int word = 0; word < word_list.size(); word++){
        if (word == 0){
            serialized_vector = word_list[word];
        }

        else{
            serialized_vector = serialized_vector + ", " + word_list[word];
        }
    }

    return serialized_vector;
}

//Super-serialize a vector for output pipe reading
std::vector<char> super_serialize(std::vector<std::string> word_list){
    std::vector<char> super_serialized_vector;

    if (word_list.size() == 0){
        return super_serialized_vector;
    }

    for (int word = 0; word < word_list.size(); word++){
        if (word == 0){
            for (int letter = 0; letter < word_list[word].size(); letter++){
                super_serialized_vector.push_back(word_list[word][letter]);
            }
        }

        else{
            super_serialized_vector.push_back(',');
            super_serialized_vector.push_back(' ');
            for (int letter = 0; letter < word_list[word].size(); letter++){
                super_serialized_vector.push_back(word_list[word][letter]);
            }
        }
    }

    //This is so our vectorize() function can parse the word back into strings properly
    super_serialized_vector.push_back(',');
    super_serialized_vector.push_back(' ');
    
    return super_serialized_vector;
}

//Turn serialized string back into vector inside a process
void vectorize(std::string word_string, std::vector<std::string> &word_list){
    std::stringstream ss(word_string);
    std::string word;

    while (std::getline(ss, word, ',')){
        //check if empty space at word[0]
        if (!word.empty() && word[0] == ' '){
            word = word.substr(1);
        }
        
        word_list.push_back(word);
    }
}

//Count number of pipes for our matrix
int count_pipes_to_create(std::vector<std::vector<int>> process_matrix){
    int size = process_matrix.size();
    int pipes_to_create = 0;

    for (int row = 0; row < size; row++){
        pipes_to_create += count_outgoing_pipes_for_process(process_matrix[row]);
    }
    
    return pipes_to_create;
}

//Find incoming pipe for row
std::vector<int> incoming_pipe_index_vector(int process_number, std::vector<std::vector<int>> process_matrix){
    std::vector<int> pipe_locations_in_pipe_vector;
    int index_offset; //Sum of pipes before this row, keep in mind pipe 1 is index 0

    //Check column for precedence
    for (int matrix_row = 0; matrix_row < process_matrix.size(); matrix_row++){
        index_offset = 0;

        //If there is a pipe connecting this row to this column
        if (process_matrix[matrix_row][process_number]){
            //If we're past the first row
            if (matrix_row > 0){
                for (int previous_rows = matrix_row - 1; previous_rows >= 0; previous_rows--){
                    //Sums all pipes in rows before current row
                    index_offset += count_outgoing_pipes_for_process(process_matrix[previous_rows]);
                }
            }
            
            for (int column = 0; column < process_number; column++){
                //Adds all preceding pipes in row up to but not including the current pipe edge
                index_offset += process_matrix[matrix_row][column];
            }
            //Add this to vector.
            pipe_locations_in_pipe_vector.push_back(index_offset);
        }
    }
    
    return pipe_locations_in_pipe_vector;
}

//Find all outgoing pipes for a given row in our pipe_file vector
std::vector<int> outgoing_pipe_index_vector(int process_number, std::vector<std::vector<int>> process_matrix){
    std::vector<int> pipe_locations_in_pipe_file_desc_vector;
    int offset = 0;

    //Find offset
    if (process_number > 0){
        for (int previous_rows = 0; previous_rows < process_number; previous_rows++){
            offset += count_outgoing_pipes_for_process(process_matrix[previous_rows]);
        }
    }

    for (int intersection = 0; intersection < process_matrix.size(); intersection++){
        if (process_matrix[process_number][intersection]){
            pipe_locations_in_pipe_file_desc_vector.push_back(offset);
        }

        offset += process_matrix[process_number][intersection];
    }

    return pipe_locations_in_pipe_file_desc_vector;    
}

//Close unused write end of pipes in the pipes vector
void close_unused_write_ends(std::vector<int> used_pipe_locations, std::vector<int*> pipes){
    int location_marker = 0; //Could use front feed and vector reduction, but I don't want to destroy my vector
    int pipe_location = used_pipe_locations[location_marker];

    for (int index = 0; index < pipes.size(); index++){
        if (index == pipe_location){
            location_marker++;
            pipe_location = used_pipe_locations[location_marker];
        } else{
            close(pipes[index][1]);
        }
    }
}

//Close unused read end of pipes in the pipes vector
void close_unused_read_ends(std::vector<int> used_pipe_locations, std::vector<int*> pipes){
    int location_marker = 0;
    int pipe_location = used_pipe_locations[location_marker];

    for (int index = 0; index < pipes.size(); index++){
        if (index == pipe_location){
            location_marker++;
            pipe_location = used_pipe_locations[location_marker];
        } else{
            close(pipes[index][0]);
        }
    }
}

//Write to all outgoing pipes for a given process
void write_to_outgoing_pipes(std::vector<int> outgoing_pipe_locations, std::vector<int*> pipes, std::vector<std::string> word_list){
    std::string serialized_vector = serialize(word_list);
    for (int index = 0; index < outgoing_pipe_locations.size(); index++){
        int pipe_index = outgoing_pipe_locations[index];

        write(pipes[pipe_index][1], serialized_vector.c_str(), serialized_vector.size() + 1);
        close(pipes[pipe_index][1]);
    }
}

//Read from all incoming pipes and return a vector of strings for a given process
//Takes a std::vector<int> that is the process' incoming pipe location in our pipe vector and a std::vector<int*> of pipes
std::vector<std::string> read_from_incoming_pipes(std::vector<int> index_pipe_locations, std::vector<int*> pipes){
    //Need to cut over ',' and add english number words
    std::vector<std::string> word_list;
    int number_of_pipes_to_read = index_pipe_locations.size();
    char buffer[100]; //Maybe move buffer into for loop to ensure we don't send the wrong data to the wrong pipe

    for (int index = 0; index < number_of_pipes_to_read; index++){
        int pipe_index = index_pipe_locations[index];

        read(pipes[pipe_index][0], buffer, 100);
        close(pipes[pipe_index][0]);

        //Cleans word by cutting off at '\0' char
        std::string incoming_word(buffer);
        //Split words over ',' char add to vector
        vectorize(incoming_word, word_list);
    }
    //Add steps to process and combine like words
    std::sort(word_list.begin(), word_list.end());
    combine_like_words(word_list);
    
    return word_list;
}

//Add english numbers
//Takes two strings that are the number words from two separate words we are combining
std::string add_number_words(std::string word_one, std::string word_two){
    std::string english_sum_word;
    std::vector<std::string> number_words = {"", "two", "three", "four", "five", "six", "seven", "eight", "nine", "ten"}; //There won't be a "one"
    int first_word = 0;
    int second_word = 0;
    int numerical_word_sum = 0;

    for (int word_index = 0; word_index < number_words.size(); word_index++){
        if (word_one == number_words[word_index]){
            first_word = word_index + 1;
        }
        if (word_two == number_words[word_index]){
            second_word = word_index + 1;
        }
    }

    //Add
    numerical_word_sum = first_word + second_word - 1;
    //Can only have a max value of "ten"
    if (numerical_word_sum > 9){
        numerical_word_sum = 9;
    }

    //Convert to english
    english_sum_word = number_words[numerical_word_sum];

    return english_sum_word;
}

//Find number word if it's attached to string
//Takes a single string comprised of a word or a number word a space and another word
std::string find_number_word(std::string word_to_check){
    std::string number_word = "";
    std::vector<std::string> number_words = {"one", "two", "three", "four", "five", "six", "seven", "eight", "nine", "ten"}; //"one" is unnecessary, leaving it for now because it works
    
    //Can add a catch all to check for a space to be sure that a number isn't inserted into a fruit, but that seems unlikely
    for (int word_index = 0; word_index < number_words.size(); word_index++){
        if (word_to_check.find(number_words[word_index]) != std::string::npos){
            number_word = number_words[word_index];
        }
    }

    return number_word;
}

//Detach a word from it's number
std::string detached_word(std::string number_word, std::string original_word){
    std::string isolated_word;

    if (number_word == ""){
        return original_word;
    }

    //Cut off the number and space and plural 's'
    int slice_position = number_word.size() + 1;
    int position_of_s = original_word.size() - slice_position - 1;

    isolated_word = original_word.substr(slice_position, position_of_s);

    return isolated_word;
}

//Combine same words and increment english count
std::vector<std::string> combine_like_words(std::vector<std::string> words_list){
    //Find all occurrences of first word in vector
    //Add them, then remove them
    //Repeat until all pieces have been combined

    //Add word to this vector to be sure we don't double add words
    std::unordered_set<std::string> words_we_have_seen_set;
    std::vector<std::string> final_summed_words_list;
    int size = words_list.size();

    for (int word_index = 0; word_index < size - 1; word_index++){
        std::string current_number_word = find_number_word(words_list[word_index]);
        std::string current_word = detached_word(current_number_word, words_list[word_index]);

        if (words_we_have_seen_set.count(current_word)){
            continue;
        }

        for (int check_index = word_index + 1; check_index < words_list.size(); check_index++){
            std::string second_number_word = find_number_word(words_list[check_index]);
            std::string second_word = detached_word(second_number_word, words_list[check_index]);
            if (current_word == second_word){
                current_number_word = add_number_words(current_number_word, second_number_word);
            }
        }

        std::string summed_word;
        if (current_number_word == ""){
            summed_word = current_word;
        } else {
            summed_word = current_number_word + " " + current_word + "s";
        }
        words_we_have_seen_set.insert(current_word);
        final_summed_words_list.push_back(summed_word);
    }

    //Check last word in list
    std::string last_number_word = find_number_word(words_list[words_list.size() - 1]);
    std::string last_word = detached_word(last_number_word, words_list[words_list.size() - 1]);

    if (!words_we_have_seen_set.count(last_word)){
        final_summed_words_list.push_back(words_list[words_list.size() - 1]);
    }

    return final_summed_words_list;
}

//Format output
std::string format_final_output(std::vector<std::string> output_words_list){
    int size = output_words_list.size();
    std::string output_string = "";

    if (output_words_list[0] == ""){
        std::cout << "WHY IS THERE AN EMPTY STRING HERE?!?!?!!\n";
    }

    for (int word = 0; word < size; word++){
        if (word == 0){
            output_string = output_words_list[word];
            continue;
        } else if (word > 0 && word < size - 1){
            output_string = output_string + ", " + output_words_list[word];
        } else {
            output_string = output_string + ", " + output_words_list[word] + ".";
        }
    }

    return output_string;
}

//Clean up our words list vector to remove any straggling spaces from the way we added it to the pipes
void clean_words_vector(std::vector<std::string> &words_list){
    int size = words_list.size();
    std::vector<std::string> new_words_list;

    for (int word = 0; word < size; word++){
        if (words_list[word] == ""){ //Add other checks if needed
            continue;
        }
        new_words_list.push_back(words_list[word]);
    }

    words_list = new_words_list;
}
