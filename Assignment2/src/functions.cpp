#include "functions.hpp"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <unordered_set>
#include <limits>

#ifdef _WIN32

int pipe(int* pipe_file){
    std::cout << "[MOCK] Simulating pipe() in Windows\n";
    return 0;
}

void close(int pipe_file_part){
    std::cout << "[MOCK] Simulating close() in Windows\n";
}

#else

#endif

//Global Variable
//Absolute worst case scenario for a string size sent via pipe
const int MESSAGE_SIZE = 1024;

//Stores all data for a given process
Process_Data::Process_Data(std::string process_name, int deadline, int computation_time, std::vector<std::string> instructions) : process_name(process_name), deadline(deadline), computation_time(computation_time), instructions(std::move(instructions)){
    process_request_pipe = new int[2];
    resource_return_pipe = new int[2];
    master_string = {};
    global_time_tracker = 0;
    missed_deadline_flagged = static_cast<int*>(mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0));
    
    *missed_deadline_flagged = 0;

    process_owned_resources = {};
    
    banker_reply_sem = (sem_t*) mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    process_request_sem = (sem_t*) mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    resource_return_sem = (sem_t*) mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    
    sem_init(process_request_sem, 1, 0);
    sem_init(banker_reply_sem, 1, 0);
    sem_init(resource_return_sem, 1, 0);

    if(pipe(process_request_pipe) == -1){
        exit(0);
    }

    if(pipe(resource_return_pipe) == -1){
        exit(0);
    }
}

Process_Data::~Process_Data(){
    if(process_request_pipe){
        close(process_request_pipe[0]);
        close(process_request_pipe[1]);
        delete[] process_request_pipe;
    }

    if(resource_return_pipe){
        close(resource_return_pipe[0]);
        close(resource_return_pipe[1]);
        delete[] resource_return_pipe;
    }

    munmap(missed_deadline_flagged, sizeof(int));

    munmap(process_request_sem, sizeof(sem_t));
    munmap(banker_reply_sem, sizeof(sem_t));
    munmap(resource_return_sem, sizeof(sem_t));
}

//Helper function to build empty resource pool for process_data struct
std::vector<std::vector<std::string>> build_initialization_template_for_resource_pool(int num_resources){
    std::vector<std::vector<std::string>> template_vector;

    for(int resource = 0; resource < num_resources; resource++){
        std::vector<std::string> resources = {};
        template_vector.push_back(resources);
    }

    return template_vector;
}

/////////////////////
//LEGACY FUNCTIONS//
/////////////////////

//Processes an instruction string
//UNUSED//
void process_instruction(std::vector<std::string> instruction_vector){
    int instruction_size = instruction_vector.size();
    std::string command = instruction_vector[0];



}

//Turns an instruction string into a vector of strings
//UNUSED//
std::vector<std::string> vectorize_instruction(std::string instruction){
    std::stringstream instruction_stream(instruction);
    std::string instruction_piece;
    std::vector<std::string> disassembled_instruction;

    std::cout << "INSTRUCTION PIECES ARE:\n";
    while(instruction_stream >> instruction_piece){
        //Remove ';' for now, may be able to use it later, we'll see
        if(instruction_piece != ";"){
            std::cout << instruction_piece << std::endl;
            disassembled_instruction.push_back(instruction_piece);
        }
    }

    return disassembled_instruction;
}

//Add allocated resources to the master_string
//UNUSED//
void add_resources_to_master_string(const std::vector<std::vector<std::string>> &allocated_resources, std::vector<std::string> &master_string){
    for(const std::vector<std::string> &resource_group : allocated_resources){
        for(const std::string &resource : resource_group){
            //std::cout << resource << " added to master_string\n";
            master_string.push_back(resource);
        }
    }
    std::sort(master_string.begin(), master_string.end());
}

//Populate our request_queue with names
//Updates the global clock when it parses a new request
//Removes requests from the request_list once they're added to the matrix
//Adds the requests to the map for quick lookup
//UNUSED//
void add_to_requests_map_populate_request_queue(std::vector<std::string> &request_queue, std::unordered_map<std::string, std::vector<std::string>> &requests_matrix_map, std::vector<std::string> &request_list, int &global_time){
    std::vector<std::string> single_request;

    for(int request = request_list.size() - 1; request >= 0; request--){
        single_request = parse_request_message(request_list[request], global_time);
        request_queue.push_back(single_request[0]);
        requests_matrix_map[single_request[0]] = single_request;
        request_list.pop_back();
    }
}

/////////////////////
//END LEGACY FUNCTIONS//
/////////////////////

/////////////////////
//TEST FUNCTIONS//
/////////////////////

void print_process_data(Process_Data *process_info){
    std::cout << "\nProcess " << process_info->process_name << ":\n";
    std::cout << "Deadline: " << process_info->deadline << "\nComputation Time: " << process_info->computation_time << "\n";
    std::cout << "Instructions:\n";
    for(int instruction = 0; instruction < process_info->instructions.size(); instruction++){
        std::cout << process_info->instructions[instruction] << std::endl;
    }
    std::cout << "\n\n";
}

void print_resource_matrix(std::vector<std::vector<std::string>> resource_matrix){
    for(int resource_category = 0; resource_category < resource_matrix.size(); resource_category++){
        std::cout << "Resource " << resource_category + 1 << std::endl;
        for(int resource = 0; resource < resource_matrix[resource_category].size(); resource++){
            std::cout << resource_matrix[resource_category][resource];
            if(resource < resource_matrix[resource_category].size() - 1){
                std::cout << ", ";
            }
        }
        std::cout << "\n\n";
    }
}

/////////////////////
//END TEST FUNCTIONS//
/////////////////////

/////////////////////
//PARSE INPUT FUNCTIONS//
/////////////////////

//Read in the file names and save them in a std::vector<std::string> to use as needed
std::vector<std::string> file_name_read_from_function_call(char *argv[]){
    std::string file_one(argv[1]);
    std::string file_two(argv[2]);

    std::vector<std::string> file_names = {file_one, file_two};

    return file_names;
}

//Reads file by lines, stores as std::string vector
std::vector<std::string> read_file_into_vector(std::string filename){
    std::ifstream file(filename);
    std::string read_line;
    std::vector<std::string> file_contents;

    if (file.is_open()){
        while (std::getline(file, read_line)){
            file_contents.push_back(read_line);
        }

        file.close();

    } else {
        std::cerr << "Unable to open file " << filename << ". Please check file name and try again.\n";
    }

    return file_contents;    
}

//Parses the process file contents
std::vector<Process_Data*> parse_process_file_contents(std::vector<std::string> file_contents, std::vector<int> &resource_availability){
    std::vector<Process_Data*> process_data_vector;

    std::vector<std::string> integer_data = slice_vector_to_next_space(file_contents);

    resource_availability = determine_resource_availability(integer_data);
    
    std::vector<std::vector<int>> process_demand_matrix = determine_process_demand_matrix(integer_data);

    int num_resources = std::stoi(integer_data[0]);
    int num_processes = std::stoi(integer_data[1]);

    for(int process_number = 0; process_number < num_processes; process_number++){
        std::vector<std::string> process_file_section = slice_vector_to_next_space(file_contents);

        Process_Data* process_data = parse_process_data(process_file_section, num_resources);
        
        process_data_vector.push_back(process_data);
    }

    return process_data_vector;
}

//Build Process_Data map for quick lookup
std::unordered_map<std::string, Process_Data*> build_process_map(std::vector<Process_Data*> process_data_vector){
    std::unordered_map<std::string, Process_Data*> process_map;

    for(int process = 0; process < process_data_vector.size(); process++){
        std::string process_name = process_data_vector[process]->process_name;
        process_map[process_name] = process_data_vector[process];
    }

    return process_map;
}

//Build active process vector
void build_active_process_vector(const std::vector<Process_Data*> &process_data_vector, std::vector<std::string> &active_processes){
    for(const Process_Data *process : process_data_vector){
        std::string name = process->process_name;
        active_processes.push_back(name);
    }
}

//Parses the resource file contents
std::vector<std::vector<std::string>> parse_resource_file_contents(std::vector<std::string> file_contents){
    std::vector<std::vector<std::string>> resource_matrix;
    std::vector<std::string> processed_file_lines = clean_resource_file_vector(file_contents);

    for(int file_line = 0; file_line < processed_file_lines.size(); file_line++){
        std::vector<std::string> vectorized_line = vectorize(processed_file_lines[file_line]);
        resource_matrix.push_back(vectorized_line);
    }

    return resource_matrix;
}

//Slice vector to the first empty space without including the empty space in the slice
std::vector<std::string> slice_vector_to_next_space(std::vector<std::string> &file_contents){
    auto iterate_to_space = std::find(file_contents.begin(), file_contents.end(), "");

    std::vector<std::string> vector_slice(file_contents.begin(), iterate_to_space);

    if(iterate_to_space != file_contents.end()){
        file_contents.erase(file_contents.begin(), iterate_to_space + 1);
    }

    return vector_slice;
}

//Create resources availability vector
std::vector<int> determine_resource_availability(const std::vector<std::string> &file_slice){
    int num_resources = std::stoi(file_slice[0]);
    int offset = 2;
    std::vector<int> resource_availability;

    for(int resource_count = 0; resource_count < num_resources; resource_count++){
        int index = resource_count + offset;
        resource_availability.push_back(std::stoi(file_slice[index]));
    }

    return resource_availability;
}

//Parse the string into integers to add to a vector using stringstream
//Repurposed and modified from assignment1
std::vector<int> vectorize_string_to_int(std::string string_of_ints){
    std::stringstream ss(string_of_ints);

    std::vector<int> int_vector;
    int digit;

    while (ss >> digit) {
        int_vector.push_back(digit);
    }

    return int_vector;
}

//Create resource demand matrix
std::vector<std::vector<int>> determine_process_demand_matrix(const std::vector<std::string> &file_slice){
    int num_resources = std::stoi(file_slice[0]);
    int num_processes = std::stoi(file_slice[1]);
    int offset = 2 + num_resources;
    std::vector<std::vector<int>> process_demand_matrix;

    for(int process_count = 0; process_count < num_processes; process_count++){
        int index = process_count + offset;
        process_demand_matrix.push_back(vectorize_string_to_int(file_slice[index]));
    }

    return process_demand_matrix;
}

//Build our Process_Data object
Process_Data* parse_process_data(std::vector<std::string> file_slice, int num_resources){
    std::string process_name = file_slice[0];
    int deadline = std::stoi(file_slice[1]);
    int computation_time = std::stoi(file_slice[2]);
    int offset = 3;
    auto start_iterator = file_slice.begin() + offset;
    std::vector<std::string> instructions(start_iterator, file_slice.end());

    std::vector<std::vector<std::string>> empty_resource_pool = build_initialization_template_for_resource_pool(num_resources);

    Process_Data* process_data = new Process_Data(process_name, deadline, computation_time, instructions);

    process_data->process_owned_resources = empty_resource_pool;

    return process_data;
}

//Cut our resource text into individual resources
//Repurposed and modified from assignment1
std::vector<std::string> vectorize(std::string word_string){
    std::stringstream ss(word_string);
    std::string word;
    std::vector<std::string> line_vector;

    while (std::getline(ss, word, ',')){
        //check if empty space at word[0]
        if (!word.empty() && word[0] == ' '){
            word = word.substr(1);
        }
        
        line_vector.push_back(word);
    }

    return line_vector;
}

//Remove colons from resource file to leave us only the resources
std::vector<std::string> clean_resource_file_vector(std::vector<std::string> file_contents){
    int last_colon_location;
    std::string cleaned_string;
    std::vector<std::string> clean_vector;

    for(int line = 0; line < file_contents.size();line++){
        last_colon_location = file_contents[line].rfind(':');
        cleaned_string = file_contents[line].substr(last_colon_location + 1);
        clean_vector.push_back(cleaned_string);
    }

    return clean_vector;
}

/////////////////////
//END PARSE INPUT FUNCTIONS//
/////////////////////

/////////////////////
//PROCESS FUNCTIONS//
/////////////////////

//Run our process using it's instructions vector
void run_process(Process_Data *process){
    for(const std::string &instruction : process->instructions){
        std::stringstream full_instruction(instruction);
        std::string function_type;

        full_instruction >> function_type;

        if(function_type == "compute"){
            compute(instruction, process);
        } else if(function_type == "request"){
            request(instruction, process);
        } else if(function_type == "release"){
            release(instruction, process);
        } else if(function_type == "use_resources"){
            use_resources(instruction, process);
        } else if(function_type == "reduce_resources"){
            reduce_resources(instruction, process);
        } else if(function_type == "print_resources_used"){
            print_resources_used(process);
        } else if(function_type == "end."){
            end_process(instruction, process);
        } else{
            std::cout << "UNDEFINED INSTRUCTION: " << function_type << "\n\n";
            exit(0);
        }
    }
}

//Checks if deadline missed, displays notification
void process_deadline_warning(Process_Data *process){
    if(process->global_time_tracker > process->deadline && !*(process->missed_deadline_flagged)){
        std::cout << "\n\n***DEADLINE MISSED FOR " << process->process_name << "***\n\n";
        *process->missed_deadline_flagged = 1;
    }
}

//Tick global clock forward and computation time down by given time
void compute(std::string compute_request, Process_Data *process){
    std::vector<int> compute_time = parse_resource_request_return(compute_request);
    process->global_time_tracker = process->global_time_tracker + compute_time[0];
    process->computation_time = process->computation_time - compute_time[0];

    process_deadline_warning(process);
}

//Request resources from banker
void request(std::string resource_request, Process_Data *process){
    process->global_time_tracker++;
    std::string message = std::to_string(process->global_time_tracker) + ";" +  process->process_name + ";" + resource_request;
    
    write(process->process_request_pipe[1], message.c_str(), message.size() + 1);
    sem_post(process->process_request_sem);

    char buffer[MESSAGE_SIZE];
    sem_wait(process->banker_reply_sem);
    read(process->process_request_pipe[0], buffer, MESSAGE_SIZE);
    std::string reply_message(buffer);

    process->computation_time--;

    add_resources_to_process_owned_resource(reply_message, process);
    
    process_deadline_warning(process);
}

///////////////////
//Request Helpers//
///////////////////

//Adds received resources into process_resources
//Send order: global_time;resource_return x y z;resourcex0;resourcex1;resourcey0;...
void add_resources_to_process_owned_resource(std::string message, Process_Data *process){
    std::vector<std::string> return_data = partition_resource_return_message(message, process->global_time_tracker);
    std::vector<std::vector<std::string>> allocated_resources = sorted_resource_return(return_data);

    combine_resources(allocated_resources, process->process_owned_resources);
}

//Combine allocated/returned resources back into their target resource pool
void combine_resources(std::vector<std::vector<std::string>> &allocated_resources, std::vector<std::vector<std::string>> &receiving_resource_pool){
    for(int resource_group = 0; resource_group < receiving_resource_pool.size(); resource_group++){
        for(const std::string &resource : allocated_resources[resource_group]){
            receiving_resource_pool[resource_group].push_back(resource);
        }
        std::sort(receiving_resource_pool[resource_group].begin(), receiving_resource_pool[resource_group].end());
    }
}

///////////////////
//End Request Helpers//
///////////////////

//Send resources and tracked_global_time back to banker
void release(std::string release_request, Process_Data *process){
    process->global_time_tracker++;
    process->computation_time--;
    process_deadline_warning(process);

    std::string resources_being_released = build_resource_release_string(release_request, process->process_owned_resources);
    std::string message = std::to_string(process->global_time_tracker) + ";" + release_request + resources_being_released;

    write(process->resource_return_pipe[1], message.c_str(), message.size() + 1);
    sem_post(process->resource_return_sem);
}

///////////////////
//Release Helpers//
///////////////////

//Release helper to send designated resources back to banker
std::string build_resource_release_string(std::string release_request, std::vector<std::vector<std::string>> &process_owned_resources){
    std::vector<int> resource_release_amounts = parse_resource_request_return(release_request);
    std::string resources_being_released = "";

    for(int resource_group = 0; resource_group < resource_release_amounts.size(); resource_group++){
        std::queue<std::string> resources_in_group = build_resource_queue_from_group(process_owned_resources[resource_group]);
        
        for(int resource = 0; resource < resource_release_amounts[resource_group]; resource++){
            std::string resource_name = resources_in_group.front();
            resources_being_released = resources_being_released + ";" + resource_name;
            resources_in_group.pop();
        }
        
        return_remaining_resources_from_queue_to_vector(resources_in_group, process_owned_resources[resource_group]);
    }

    return resources_being_released;
}

///////////////////
//End Release Helpers//
///////////////////

//Add resources to master_string and tick clock/computation
void use_resources(std::string use_request, Process_Data *process){
    std::vector<int> time_and_uses = parse_resource_request_return(use_request);

    process->global_time_tracker += time_and_uses[0];
    process->computation_time -+ time_and_uses[0];
    process_deadline_warning(process);

    add_words_to_master_string(time_and_uses[1], process->master_string, process->process_owned_resources);

    combine_like_words(process->master_string);
    std::sort(process->master_string.begin(), process->master_string.end(), alphabetize);

}

///////////////////
//Use Helpers//
///////////////////

//Alphabetize function for std::sort to accommodate for the number words and capitalization
bool alphabetize(const std::string &first_word, const std::string &second_word){
    std::stringstream full_first_word(first_word);
    std::stringstream full_second_word(second_word);
    std::string part_one;
    std::string part_two;

    full_first_word >> part_one;
    if(is_number_word(part_one)){
        full_first_word >> part_one;
    }

    full_second_word >> part_two;
    if(is_number_word(part_two)){
        full_second_word >> part_two;
    }

    to_lower(part_one);
    to_lower(part_two);

    return part_one < part_two;
}

//Helper for alphabetize
bool is_number_word(const std::string &word){
    std::unordered_set<std::string> number_words = {"two", "three", "four", "five", "six", "seven", "eight", "nine", "ten"};
    return number_words.count(word);
}

//Helper for alphabetize
void to_lower(std::string &word){
    for(char &letter : word){
        letter = std::tolower(letter);
    }
}

//Add words from process_owned_resources to master_string y times
void add_words_to_master_string(int word_multiplier, std::vector<std::string> &master_string, const std::vector<std::vector<std::string>> &process_owned_resources){
    for(int multiplier = 0; multiplier < word_multiplier; multiplier++){
        for(const std::vector<std::string> &group : process_owned_resources){
            for(const std::string &resource : group){
                master_string.push_back(resource);
            }
        }
    }
}

//Combine same words and increment english count
//Reused from assignment1
//Modified to pbr
void combine_like_words(std::vector<std::string> &words_list){
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

    std::string last_number_word = find_number_word(words_list[words_list.size() - 1]);
    std::string last_word = detached_word(last_number_word, words_list[words_list.size() - 1]);

    if (!words_we_have_seen_set.count(last_word)){
        final_summed_words_list.push_back(words_list[words_list.size() - 1]);
    }

    words_list = final_summed_words_list;
}

//Find number word if it's attached to string
//Takes a single string comprised of a word or a number word a space and another word
//Reused from assignment1
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
//Reused from assignment1
std::string detached_word(std::string number_word, std::string original_word){
    std::string isolated_word;

    if (number_word == ""){
        return original_word;
    }

    int slice_position = number_word.size() + 1;
    int position_of_s = original_word.size() - slice_position - 1;

    isolated_word = original_word.substr(slice_position, position_of_s);

    return isolated_word;
}

//Add english numbers
//Takes two strings that are the number words from two separate words we are combining
//Reused from assignment1
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

    numerical_word_sum = first_word + second_word - 1;

    if (numerical_word_sum > 9){
        numerical_word_sum = 9;
    }

    english_sum_word = number_words[numerical_word_sum];

    return english_sum_word;
}

///////////////////
//End Use Helpers//
///////////////////

//Remove resources from master_string and tick clock/computation
void reduce_resources(std::string reduce_request, Process_Data *process){
    std::vector<int> time_and_reductions = parse_resource_request_return(reduce_request);
    process->global_time_tracker += time_and_reductions[0];
    process->computation_time -= time_and_reductions[0];
    process_deadline_warning(process);

    std::vector<std::string> resource_amounts_to_decriment = build_resource_reduction_list(time_and_reductions[1], process->process_owned_resources);
    combine_like_words(resource_amounts_to_decriment);

    reduce(resource_amounts_to_decriment, process->master_string);

    std::sort(process->master_string.begin(), process->master_string.end(), alphabetize);
}

///////////////////
//Reduce Helpers//
///////////////////

//Builds the resource list and amounts for each resource to be deducted from the master_string
std::vector<std::string> build_resource_reduction_list(int multiplier, const std::vector<std::vector<std::string>> &process_owned_resources){
    std::vector<std::string> resource_reduction_list;

    for(int multiple = 0; multiple < multiplier; multiple++){
        for(const std::vector<std::string> &group : process_owned_resources){
            for(const std::string &resource : group){
                resource_reduction_list.push_back(resource);
            }
        }
    }

    return resource_reduction_list;
}

//Perform the reduction on the master_string
void reduce(const std::vector<std::string> &resource_amounts_to_decriment, std::vector<std::string> &master_string){
    for(const std::string &resource_reduction : resource_amounts_to_decriment){
        std::stringstream full_reduction(resource_reduction);
        std::string reduction_word;
        std::string reduction_number = "";

        full_reduction >> reduction_word;
        if(is_number_word(reduction_word)){
            reduction_number = reduction_word;

            full_reduction >> reduction_word;
            reduction_word = reduction_word.substr(0, reduction_word.size() - 1);
        }

        for(std::string &resource : master_string){
            std::stringstream full_resource(resource);
            std::string resource_word;
            std::string resource_amount = "";

            full_resource >> resource_word;
            if(is_number_word(resource_word)){
                resource_amount = resource_word;

                full_resource >> resource_word;
                resource_word = resource_word.substr(0, resource_word.size() - 1);
            }

            if(reduction_word == resource_word){
                std::string remaining_resource = reduce_number_words(reduction_number, resource_amount);
                if(remaining_resource == ""){
                    resource = resource_word;
                    break;
                } else{
                    resource = remaining_resource + " " + resource_word + "s";
                    break;
                }
            }
        }
    }
}

//Similar to add_number_words logic
std::string reduce_number_words(std::string reduction, std::string resource_amount){
    std::vector<std::string> number_words = {"", "two", "three", "four", "five", "six", "seven", "eight", "nine", "ten"};
    int reduction_value;
    int resource_value;

    for(int number = 0; number < number_words.size(); number++){
        if(number_words[number] == reduction){
            reduction_value = number + 1;
        }

        if(number_words[number] == resource_amount){
            resource_value = number + 1;
        }
    }
    
    int number = std::max(0, (resource_value - reduction_value - 1));
    return number_words[number];
}

///////////////////
//End Reduce Helpers//
///////////////////

//Print out master_string and which process it belongs to
void print_resources_used(Process_Data *process){
    process->global_time_tracker++;
    process->computation_time--;
    process_deadline_warning(process);

    std::string master_string = build_master_string(process->master_string);
    std::cout << process->process_name << " " << master_string << std::endl;
}

///////////////////
//Print Helpers//
///////////////////

//Helper for print, converts the master_string vector into a single string
std::string build_master_string(const std::vector<std::string> &master_string){
    std::string final_string = "";
    for(const std::string &piece : master_string){
        if(final_string == ""){
            final_string = piece;
        } else{
            final_string = final_string + ", " + piece;
        }
    }

    return final_string;
}

///////////////////
//End Print Helpers//
///////////////////

//Send all resources back to banker if not released and exit()
void end_process(std::string end_request, Process_Data *process){
    std::string resource_amount = build_end_resource_release_string(process->process_owned_resources);

    std::string resources_to_return = build_resource_return_string(process->process_owned_resources);

    std::string current_time = std::to_string(process->global_time_tracker);

    std::string message = current_time + ";" + end_request + ";" + resource_amount + resources_to_return;

    close(process->resource_return_pipe[0]);
    close(process->resource_return_pipe[1]);

    close(process->process_request_pipe[0]);
    write(process->process_request_pipe[1], message.c_str(), message.size() + 1);
    close(process->process_request_pipe[1]);
    sem_post(process->process_request_sem);

    exit(0);
}

///////////////////
//End. Helpers//
///////////////////

//Builds the release x y z message for a return message
std::string build_end_resource_release_string(const std::vector<std::vector<std::string>> &process_owned_resources){
    std::vector<int> resources_per_group = find_amount_of_resources_in_each_group(process_owned_resources);
    std::string release_string = "release";

    for(const int &amount : resources_per_group){
        release_string = release_string + " " + std::to_string(amount);
    }

    return release_string;
}

//Finds the number of current resources in each group held by a process
std::vector<int> find_amount_of_resources_in_each_group(const std::vector<std::vector<std::string>> &process_owned_resources){
    std::vector<int> resources_in_each_group;

    for(const std::vector<std::string> &group : process_owned_resources){
        int group_count = 0;
        for(const std::string &resource : group){
            group_count++;
        }
        resources_in_each_group.push_back(group_count);
    }
    
    return resources_in_each_group;
}

//Builds resource return string when ALL resources should be returned
std::string build_resource_return_string(std::vector<std::vector<std::string>> &process_owned_resources){
    std::string resources = "";

    for(const std::vector<std::string> &resource_group : process_owned_resources){
        for(const std::string &resource : resource_group){
            resources = resources + ";" + resource;
        }
    }

    process_owned_resources.clear();
    return resources;
}

///////////////////
//End End. Helpers(Lol)//
///////////////////

/////////////////////
//End Process Functions//
/////////////////////

/////////////////////
//Banker Functions//
/////////////////////

//The banker controls the flow within the main parent branch
void banker(std::string SCHEDULER_TYPE, std::vector<std::string> process_data_file, std::vector<std::string> resource_data_file){
    int global_time = 0;

    std::vector<int> resource_group_information = {};
    std::vector<Process_Data*> process_data_vector = parse_process_file_contents(process_data_file, resource_group_information);
    std::vector<sem_t> banker_resource_semaphores = build_banker_resource_semaphores(resource_group_information);

    std::unordered_map<std::string, Process_Data*> process_map = build_process_map(process_data_vector);

    std::vector<std::vector<std::string>> banker_resources = parse_resource_file_contents(resource_data_file);
    
    std::vector<std::string> active_processes;

    build_active_process_vector(process_data_vector, active_processes);
    std::vector<std::string> all_system_processes = active_processes;

    std::unordered_map<std::string, std::vector<std::vector<int>>> allocation_map = build_allocation_map(process_map, active_processes);

    for(const std::string &name : active_processes){
        Process_Data *process = process_map[name];

        int pid = fork();

        if(pid == 0){
            run_process(process);
        }
    }
    
    while(active_processes.size() > 0){
        std::unordered_map<std::string, std::vector<std::string>> requests_map;
        
        banker_take_requests(active_processes, process_map, requests_map, global_time);

        order_requests(SCHEDULER_TYPE, process_map, active_processes);

        assess_and_execute_requests(active_processes, requests_map, process_map, banker_resources, global_time, banker_resource_semaphores, all_system_processes, allocation_map);
    }

    cleanup(process_data_vector);
    destroy_banker_semaphores(banker_resource_semaphores);
}

//Read instruction from pipe and add it to our request vector
void banker_take_requests(std::vector<std::string> active_processes, std::unordered_map<std::string, Process_Data*> process_map, std::unordered_map<std::string, std::vector<std::string>> &request_map, int &global_time){
    std::vector<std::string> request_list;

    for(const std::string &process_name : active_processes){
        std::string message = banker_take_request(process_name, process_map);
        request_map[process_name] = parse_request_message(message, global_time);
    }
}

//Reads request pipe from process
//Expected input: time;request a b c || time;end.;release a b c;resourcea;resourceb;...
std::string banker_take_request(std::string process_name, std::unordered_map<std::string, Process_Data*> process_map){
    char buffer[MESSAGE_SIZE];

    sem_wait(process_map[process_name]->process_request_sem);
    read(process_map[process_name]->process_request_pipe[0], buffer, MESSAGE_SIZE);
    std::string message(buffer);

    return message;
}

//Breaks the incoming message apart
//Expected input: time;request a b c || time;end.;return_resource a b c;resourcea;resourceb;...
std::vector<std::string> parse_request_message(std::string &request, int &global_time){
    std::vector<std::string> parts;

    int semicolon_location = request.find(';');
    std::string part(request.substr(0, semicolon_location));
    int reported_time = std::stoi(part);
    global_time = std::max(global_time, reported_time);

    request = request.substr(semicolon_location + 1);

    while(request.find(';') != std::string::npos){
        int semicolon_location = request.find(';');
        std::string part(request.substr(0, semicolon_location));
        parts.push_back(part);
        request = request.substr(semicolon_location + 1);
    }

    parts.push_back(request);

    return parts;
}

//Orders requests for a given precedence EDF or LLF
void order_requests(std::string SCHEDULER_TYPE, std::unordered_map<std::string, Process_Data*> process_map, std::vector<std::string> &active_processes){
    if(active_processes.size() <= 1){
        return;
    }
    
    if(SCHEDULER_TYPE == "LLF"){
        std::unordered_set<std::string> processes_we_have_added;
        std::vector<std::string> final_order;
        std::vector<std::string> processes_with_same_laxity;

        for(const std::string &request : active_processes){
            std::string current_process_name = request;

            for(const std::string &compared_request : active_processes){
                std::string compared_process_name = request;

                if(processes_we_have_added.count(compared_process_name) || current_process_name == compared_process_name){
                    continue;
                }

                if(processes_we_have_added.count(current_process_name)){
                    current_process_name = compared_process_name;
                    continue;
                }

                int current_process_laxity = process_map[current_process_name]->deadline - process_map[current_process_name]->computation_time;
                int compared_process_laxity = process_map[compared_process_name]->deadline - process_map[compared_process_name]->computation_time;

                if(current_process_laxity == compared_process_laxity){
                    processes_with_same_laxity.push_back(compared_process_name);
                    continue;
                }

                if(compared_process_laxity < current_process_laxity){
                    current_process_name = compared_process_name;
                    processes_with_same_laxity.clear();
                }

            }
            
            if(processes_with_same_laxity.size() > 0){
                processes_with_same_laxity.insert(processes_with_same_laxity.begin(), current_process_name);

                for(const std::string &process_name : processes_with_same_laxity){
                    current_process_name = process_name;
                    
                    for(const std::string &compare_to_name : processes_with_same_laxity){
                        if(processes_we_have_added.count(compare_to_name) || current_process_name == compare_to_name){
                            continue;
                        }
                        
                        if(processes_we_have_added.count(current_process_name)){
                            current_process_name = compare_to_name;
                            continue;
                        }

                        if(process_map[compare_to_name]->computation_time > process_map[current_process_name]->computation_time){
                            current_process_name = compare_to_name;
                        }
                    }
                    final_order.push_back(current_process_name);
                    processes_we_have_added.insert(current_process_name);
                }

                processes_with_same_laxity.clear();
                
            } else {
                if(!processes_we_have_added.count(current_process_name)){
                    final_order.push_back(current_process_name);
                    processes_we_have_added.insert(current_process_name);
                }
            }
        }

        active_processes = final_order;

        return;
    }

    std::unordered_set<std::string> processes_we_have_added;
    std::vector<std::string> final_order;
    std::vector<std::string> processes_with_same_deadline;

    for(const std::string &request : active_processes){
        std::string current_process_name = request;

        for(const std::string &compared_request : active_processes){
            std::string compared_process_name = compared_request;

            if(processes_we_have_added.count(compared_process_name) || current_process_name == compared_process_name){
                continue;
            }

            if(processes_we_have_added.count(current_process_name)){
                current_process_name = compared_process_name;
                continue;
            }

            if(process_map[current_process_name]->deadline == process_map[compared_process_name]->deadline){
                processes_with_same_deadline.push_back(compared_process_name);
                continue;
            }

            if(process_map[compared_process_name]->deadline < process_map[current_process_name]->deadline){
                current_process_name = compared_process_name;
                processes_with_same_deadline.clear();
            }

        }

        if(processes_with_same_deadline.size() > 0){
            processes_with_same_deadline.insert(processes_with_same_deadline.begin(), current_process_name);

            for(const std::string &process_name : processes_with_same_deadline){
                current_process_name = process_name;
                
                for(const std::string &compare_to_name : processes_with_same_deadline){
                    if(processes_we_have_added.count(compare_to_name) || current_process_name == compare_to_name){
                        continue;
                    }
                    
                    if(processes_we_have_added.count(current_process_name)){
                        current_process_name = compare_to_name;
                        continue;
                    }

                    if(process_map[compare_to_name]->computation_time < process_map[current_process_name]->computation_time){
                        current_process_name = compare_to_name;
                    }
                }
                final_order.push_back(current_process_name);
                processes_we_have_added.insert(current_process_name);
            }

            processes_with_same_deadline.clear();

        } else {
            if(!processes_we_have_added.count(current_process_name)){
                final_order.push_back(current_process_name);
                processes_we_have_added.insert(current_process_name);
            }
        }

        
    }

    active_processes = final_order;

}

//Split the request type off the full request string
std::string take_first_part(const std::string &request){
    std::stringstream full_request(request);
    std::string part;

    full_request >> part;

    return part;
}

//Assess requests or end. and execute or pause 
void assess_and_execute_requests(std::vector<std::string> &active_processes, std::unordered_map<std::string, std::vector<std::string>> &requests_matrix_map, std::unordered_map<std::string, Process_Data*> process_map, std::vector<std::vector<std::string>> &banker_resources, int &global_time, std::vector<sem_t> &banker_resources_sem, const std::vector<std::string> &all_system_processes, std::unordered_map<std::string, std::vector<std::vector<int>>> &allocation_map){
    std::unordered_set<std::string> processes_marked_for_removal;
    
    for(const std::string &process_name : active_processes){
        std::string request_type = take_first_part(requests_matrix_map[process_name][1]);
        
        if(request_type == "release"){
            std::vector<std::vector<std::string>> returning_resources = sorted_resource_return(requests_matrix_map[process_name], 2);
            
            execute_return(returning_resources, banker_resources, banker_resources_sem);

            //Remove resources from allocation_map
            std::vector<int> resources_to_add = parse_resource_request_return(requests_matrix_map[process_name][1]);
            reduce_allocation_map(resources_to_add, process_name, allocation_map);

            processes_marked_for_removal.insert(process_name);

            continue;
        }
        
        banker_grant_resources(process_name, requests_matrix_map[process_name][1], process_map, banker_resources, active_processes, global_time, banker_resources_sem, all_system_processes, allocation_map);
        
    }

    check_for_resource_returns(active_processes, process_map, banker_resources, global_time, banker_resources_sem, allocation_map);

    if(processes_marked_for_removal.size() > 0){
        std::vector<std::string> new_active_processes;
        for(const std::string &process_name : active_processes){
            if(processes_marked_for_removal.count(process_name)){
                continue;
            }
            new_active_processes.push_back(process_name);
        }
        active_processes = new_active_processes;
    }
}

//Parse resource request/return into an int vector to use to allocate to the banker_resource vector
//Expected input: std::string "request/resturn_resource a b ... n" where n is the number of resource groups and each variable is the number requested/returned from/to that group
//USED A LOT
std::vector<int> parse_resource_request_return(const std::string &request){
    std::stringstream full_request(request);
    std::string part;
    std::vector<int> resource_amounts;
    full_request >> part;

    while(full_request >> part){
        if(part != ";"){
            int amount = std::stoi(part);
            resource_amounts.push_back(amount);
        }
    }

    return resource_amounts;
}

//Grant resource request for a process
//Added: , const std::vector<std::string> &all_system_processes, const std::unordered_map<std::string, std::vector<std::vector<int>>> &allocation_map
void banker_grant_resources(std::string process_name, std::string request, std::unordered_map<std::string, Process_Data*> process_map, std::vector<std::vector<std::string>> &banker_resources, std::vector<std::string> &request_queue, int &global_time, std::vector<sem_t> &banker_resources_sem, const std::vector<std::string> &all_system_processes, std::unordered_map<std::string, std::vector<std::vector<int>>> &allocation_map){
    std::vector<int> resource_request_amounts = parse_resource_request_return(request);

    for(int resource_category = 0; resource_category < banker_resources.size(); resource_category++){
        if(resource_request_amounts[resource_category] > banker_resources[resource_category].size()){
            check_for_resource_returns(request_queue, process_map, banker_resources, global_time, banker_resources_sem, allocation_map);
        }
    }

    distribute_resources(request, process_name, process_map, resource_request_amounts, banker_resources, banker_resources_sem, global_time, all_system_processes, allocation_map);

}

//Check to see if any resources are ready to be returned
void check_for_resource_returns(std::vector<std::string> &request_queue, std::unordered_map<std::string, Process_Data*> process_map, std::vector<std::vector<std::string>> &banker_resources, int &global_time, std::vector<sem_t> &banker_resources_sem, std::unordered_map<std::string, std::vector<std::vector<int>>> &allocation_map){
    for(const std::string &process_name : request_queue){
        if(sem_trywait(process_map[process_name]->resource_return_sem) == 0){
            banker_return_resources(process_name, process_map, banker_resources, global_time, banker_resources_sem, allocation_map);
        }
    }
}

//Banker returns resources from process to the banker_resources
void banker_return_resources(std::string process_name, std::unordered_map<std::string, Process_Data*> process_map, std::vector<std::vector<std::string>> &banker_resources, int &global_time, std::vector<sem_t> &banker_resources_sem, std::unordered_map<std::string, std::vector<std::vector<int>>> &allocation_map){
    char buffer[MESSAGE_SIZE];
    
    read(process_map[process_name]->resource_return_pipe[0], buffer, MESSAGE_SIZE);
    std::string message(buffer);

    std::vector<std::string> return_data = partition_resource_return_message(message, global_time);

    std::vector<std::vector<std::string>> returning_resources = sorted_resource_return(return_data);

    //Reduce allocation map here
    std::vector<int> resources_to_add;

    for(int group = 0; group < returning_resources.size(); group++){
        resources_to_add.push_back(returning_resources[group].size());
    }

    reduce_allocation_map(resources_to_add, process_name, allocation_map);

    execute_return(returning_resources, banker_resources, banker_resources_sem);
}

//Cuts the return message into a vector and updates global time
//Send order: global_time;resource_return x y z;resourcex0;resourcex1;resourcey0;...
std::vector<std::string> partition_resource_return_message(std::string &message, int &global_time){
    int semicolon_location;
    std::string part;
    std::vector<std::string> partitioned_resource_message;

    semicolon_location = message.find(';');
    part = message.substr(0, semicolon_location);
    global_time = std::max(std::stoi(part), global_time);
    message = message.substr(semicolon_location + 1);

    semicolon_location = message.find(';');
    part = message.substr(0, semicolon_location);
    partitioned_resource_message.push_back(part);
    message = message.substr(semicolon_location + 1);

    while(message.find(';') != std::string::npos){
        semicolon_location = message.find(';');
        part = message.substr(0, semicolon_location);
        partitioned_resource_message.push_back(part);
        message = message.substr(semicolon_location + 1);
    }

    partitioned_resource_message.push_back(message);

    return partitioned_resource_message;
}

//Sort resources for return
std::vector<std::vector<std::string>> sorted_resource_return(std::vector<std::string> resource_info, int resource_index){
    std::vector<std::string> resource_type_group = {}; //To be sure we have SOMETHING for each group, even if it's null, to maintain order
    std::vector<std::vector<std::string>> sorted_resources;
    std::vector<int> resource_return_amount = parse_resource_request_return(resource_info[resource_index - 1]);

    for(const int &resource_group : resource_return_amount){
        for(int resource = 0; resource < resource_group; resource++){
            std::string resource_name = resource_info[resource_index++];
            resource_type_group.push_back(resource_name);
        }
        
        sorted_resources.push_back(resource_type_group);
        resource_type_group.clear();
    }

    return sorted_resources;
}

//Execute the resource return
void execute_return(std::vector<std::vector<std::string>> &returning_resources, std::vector<std::vector<std::string>> &banker_resources, std::vector<sem_t> &banker_resources_sem){
    for(int resource_group = 0; resource_group < banker_resources.size(); resource_group++){
        for(const std::string &resource : returning_resources[resource_group]){
            banker_resources[resource_group].push_back(resource);
            sem_post(&banker_resources_sem[resource_group]);
        }
        
        std::sort(banker_resources[resource_group].begin(), banker_resources[resource_group].end());
    }

    returning_resources.clear();
}

//Send resources to a process
//global_time;request a b c;resourcex0;resourcex1;...
//Added: const std::vector<std::string> &all_system_processes, const std::unordered_map<std::string, std::vector<std::vector<int>>> &allocation_map
void distribute_resources(std::string original_request, std::string process_name, std::unordered_map<std::string, Process_Data*> process_map, std::vector<int> resource_request_amounts, std::vector<std::vector<std::string>> &banker_resources, std::vector<sem_t> &banker_resources_sem, int global_time, const std::vector<std::string> &all_system_processes, std::unordered_map<std::string, std::vector<std::vector<int>>> &allocation_map){
    std::string message = std::to_string(global_time) + ";" + original_request;

    for(int resource_group = 0; resource_group < resource_request_amounts.size(); resource_group++){
        std::queue<std::string> resources_in_group = build_resource_queue_from_group(banker_resources[resource_group]);
        for(int resource = 0; resource < resource_request_amounts[resource_group]; resource++){
            sem_wait(&banker_resources_sem[resource_group]);
            message = message + ';' + resources_in_group.front();
            resources_in_group.pop();
        }
        return_remaining_resources_from_queue_to_vector(resources_in_group, banker_resources[resource_group]);
    }

    write(process_map[process_name]->process_request_pipe[1], message.c_str(), message.size() + 1);
    
    //ADD TO ALLOC_MAP HERE
    std::vector<int> resources_to_add = parse_resource_request_return(original_request);
    add_to_allocation_map_and_reduce_resources_needed(resources_to_add, process_name, allocation_map);

    //ADD PRINT HERE//
    print_state_of_system(process_name, banker_resources, all_system_processes, allocation_map, process_map);

    sem_post(process_map[process_name]->banker_reply_sem);
}

//Builds a queue for easy popping and vector manipulation
std::queue<std::string> build_resource_queue_from_group(std::vector<std::string> &resource_group){
    std::queue<std::string> resources_in_group;

    for(const std::string &resource : resource_group){
        resources_in_group.push(resource);
    }

    resource_group.clear();
    return resources_in_group;
}

//Return the remaining resources to the banker vector
void return_remaining_resources_from_queue_to_vector(std::queue<std::string> &resources_in_group, std::vector<std::string> &banker_resources_group){
    while(!resources_in_group.empty()){
        banker_resources_group.push_back(resources_in_group.front());
        resources_in_group.pop();
    }
}

//Build semaphores for each resource allowing our banker to track and gate their use
std::vector<sem_t> build_banker_resource_semaphores(std::vector<int> resource_group_information){
    int num_resource_groups = resource_group_information.size();
    std::vector<sem_t> banker_resource_semaphores;

    for(const int &count_in_group : resource_group_information){
        sem_t sem;
        sem_init(&sem, 1, count_in_group);
        banker_resource_semaphores.push_back(sem);
    }

    return banker_resource_semaphores;
}

//LAST MINUTE FUNCTIONS TO FIX AN OVERSIGHT//

void print_state_of_system(std::string process_serviced, const std::vector<std::vector<std::string>> &banker_resources, const std::vector<std::string> &system_processes, const std::unordered_map<std::string, std::vector<std::vector<int>>> &allocation_map, const std::unordered_map<std::string, Process_Data*> &process_map){
    //Available: 2 3 1
    //Allocation:
    //P1: 1 0 2
    //P2: 1 1 1
    //P3: 0 0 0
    //Need: //Will be from current_request map
    //P1: 2 2 0
    //P2: 2 2 3
    //P3: 1 1 2
    //Deadline Misses: None

    std::cout << "System state after servicing " << process_serviced << std::endl;
    std::string available_resources = "Available:";
    std::string allocation = "Allocation:\n";
    std::string need = "Need:\n";
    std::string deadline_misses = "";

    for(const std::vector<std::string> &resource_group : banker_resources){
        available_resources = available_resources + " " + std::to_string(resource_group.size());
    }

    for(const std::string &process_name : system_processes){
        allocation = allocation + process_name;
        need = need + process_name;

        for(const int &amount_per_group : allocation_map.at(process_name)[0]){
            allocation = allocation + " " + std::to_string(amount_per_group);
        }
        allocation += "\n";

        for(const int &amount_per_group : allocation_map.at(process_name)[1]){
            need = need + " " + std::to_string(amount_per_group);
        }
        need += "\n";
    }

    for(const std::string &process_name : system_processes){
        if(*(process_map.at(process_name)->missed_deadline_flagged)){
            deadline_misses = deadline_misses + process_name + "\n";
        }
    }

    std::cout << available_resources << std::endl;
    std::cout << allocation << need << std::endl;

    if(deadline_misses.length() <= 1){
        std::cout << "Missed Deadlines:\nNone\n" << std::endl;
    } else{
        std::cout << "Missed Deadlines:\n" << deadline_misses << "\n" << std::endl;
    }

}

std::unordered_map<std::string, std::vector<std::vector<int>>> build_allocation_map(std::unordered_map<std::string, Process_Data*> process_map, std::vector<std::string> active_processes){
    std::unordered_map<std::string, std::vector<std::vector<int>>> allocation_map;
    std::vector<int> resources_in_each_group;
    int num_resources = process_map[active_processes[0]]->process_owned_resources.size();

    for(int group = 0; group < num_resources; group++){
        resources_in_each_group.push_back(0);
    }
    
    for(const std::string &process_name : active_processes){
        //Create new resources_demanded vector here for each process
        std::vector<int> resources_demanded_by_each_group = resources_in_each_group;
        total_resources_needed(resources_demanded_by_each_group, process_map[process_name]);

        allocation_map[process_name] = {{}, {}};
        allocation_map[process_name][0] = resources_in_each_group;
        allocation_map[process_name][1] = resources_demanded_by_each_group;
    }

    return allocation_map;
}

void add_to_allocation_map_and_reduce_resources_needed(std::vector<int> resources_to_add, std::string process_name, std::unordered_map<std::string, std::vector<std::vector<int>>> &allocation_map){
    for(int group = 0; group < resources_to_add.size(); group++){
        allocation_map[process_name][0][group] += resources_to_add[group];
        allocation_map[process_name][1][group] -= resources_to_add[group];
    }
}

void reduce_allocation_map(std::vector<int> resources_to_add, std::string process_name, std::unordered_map<std::string, std::vector<std::vector<int>>> &allocation_map){
    for(int group = 0; group < resources_to_add.size(); group++){
        allocation_map[process_name][0][group] -= resources_to_add[group];
    }
}

void total_resources_needed(std::vector<int> &resources_demanded, Process_Data *process){
    for(const std::string &instruction : process->instructions){
        std::string current_instruction = instruction;
        current_instruction = take_first_part(current_instruction);

        if(current_instruction == "request"){
            std::vector<int> quantities_to_add = parse_resource_request_return(instruction);
            for(int group = 0; group < quantities_to_add.size(); group++){
                resources_demanded[group] += quantities_to_add[group];
            }
        }
    }
}

//END LAST MINUTE FUNCTIONS//

/////////////////////
//End Banker Functions//
/////////////////////

/////////////////////
//Execv Functions//
/////////////////////

//Borrowed structure from assignment1 implementation and modified for this assignment
//Instead of mixing up our output, will fully run the first simulation, then create a new program to run the second
void execute_LLF_with_execv(std::string input_file_one, std::string input_file_two){
    std::string file_string = "./main_LLF";
    char* file_name = const_cast<char*>(file_string.c_str());
    char* file_one = const_cast<char*>(input_file_one.c_str());
    char* file_two = const_cast<char*>(input_file_two.c_str());
    char *args[] = {file_name, file_one, file_two, NULL};
        
    execv(args[0], args);
}

/////////////////////
//End Execv Functions//
/////////////////////

/////////////////////
//Cleanup Functions//
/////////////////////

//Clean up Process_Data pointer vector
void cleanup(std::vector<Process_Data*> &my_processes){
    for(Process_Data* process : my_processes){
        delete process;
    }
}

void destroy_banker_semaphores(std::vector<sem_t> &banker_resource_semaphores){
    for(sem_t &semaphore : banker_resource_semaphores){
        sem_destroy(&semaphore);
    }
}

/////////////////////
//End Cleanup Functions//
/////////////////////
