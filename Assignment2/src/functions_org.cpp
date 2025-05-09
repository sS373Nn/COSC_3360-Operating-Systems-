#include "functions.hpp"

//Shared includes here

#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <unordered_set>
#include <limits> //Can remove

#ifdef _WIN32

//Stub shit here
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
//Use new sem_t so as to be able to use the semaphores in the data struct across processes without COW interfering with usage
Process_Data::Process_Data(std::string process_name, int deadline, int computation_time, std::vector<std::string> instructions) : process_name(process_name), deadline(deadline), computation_time(computation_time), instructions(std::move(instructions)){
    process_request_pipe = new int[2];
    resource_return_pipe = new int[2];
    master_string = {};
    global_time_tracker = 0;
    missed_deadline_flagged = false;

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

//Read in the file names and save them in a std::vector<std::string> to use as needed
std::vector<std::string> file_name_read_from_function_call(char *argv[]){
    //Expect exactly argc == 3
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
            //std::cout << "[" << file_contents.back() << "]" << std::endl;
        }

        file.close();

    } else {
        std::cerr << "Unable to open file " << filename << ". Please check file name and try again.\n";
    }

    return file_contents;    
}

//TEST AGAIN, MODIFIED//
//Parses the process file contents
//Use functions specific to each part
std::vector<Process_Data*> parse_process_file_contents(std::vector<std::string> file_contents, std::vector<int> &resource_availability){
    std::vector<Process_Data*> process_data_vector;

    //std::cout << "Began parsing file contents\n";

    std::vector<std::string> integer_data = slice_vector_to_next_space(file_contents);
    //std::cout << "Created our integer_data\n";

    resource_availability = determine_resource_availability(integer_data);
    //std::cout << "Determined our resource availability\n";
    
    std::vector<std::vector<int>> process_demand_matrix = determine_process_demand_matrix(integer_data);
    //std::cout << "Created our process_demand_matrix\n";


    int num_resources = std::stoi(integer_data[0]);
    int num_processes = std::stoi(integer_data[1]);

    //std::cout << "Num_resources: " << num_resources << "\nNum_processes: " << num_processes << std::endl;

    for(int process_number = 0; process_number < num_processes; process_number++){
        std::vector<std::string> process_file_section = slice_vector_to_next_space(file_contents);
        //std::cout << "Sliced vector\n";

        Process_Data* process_data = parse_process_data(process_file_section, num_resources);
        //std::cout << "Built a process_data\n";
        
        process_data_vector.push_back(process_data);
        //std::cout << "Added that process_data to our vector\n";
    }

    //std::cout << "Completed our Process_Data vector\n";

    return process_data_vector;
}

//Build Process_Data map for quick lookup
//May fully convert depending on uses for the old vector
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
//Renamed variables to fit this project
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

    //std::cout << "Set all initial variables successfully for process parsing\n";
    //std::cout << "Instruction set for " << process_name << std::endl;
    //for(const std::string &instruction : instructions){
    //    std::cout << instruction << std::endl;
    //}

    std::vector<std::vector<std::string>> empty_resource_pool = build_initialization_template_for_resource_pool(num_resources);

    //std::cout << "Built our empty pool\n";

    Process_Data* process_data = new Process_Data(process_name, deadline, computation_time, instructions);
    //std::cout << "Built our new struct\n";

    process_data->process_owned_resources = empty_resource_pool;
    //std::cout << "Set it's resource pool to be empty\n";

    return process_data;

}

//Cut our resource text into individual resources
//Repurposed and modified from assignment1
//Could modify to be more versatile and work for processing instructions as well
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
//May modify to include the R1, etc and type
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
//Process Functions//
/////////////////////

//Run our process using it's instructions vector
void run_process(Process_Data *process){
    for(const std::string &instruction : process->instructions){
        std::stringstream full_instruction(instruction);
        std::string function_type;

        full_instruction >> function_type;

        //std::cout << "INSTRUCTION FOR PROCESS " << process->process_name << " " << function_type << "\n\n";

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
    if(process->global_time_tracker > process->deadline && !process->missed_deadline_flagged){
        std::cout << "\n\n***DEADLINE MISSED FOR " << process->process_name << "***\n\n";
        process->missed_deadline_flagged = true;
    }
}

//Tick clock forward by compute_time
//This would modify the Process_Data deadline clock and computation time
void compute(std::string compute_request, Process_Data *process){
    std::vector<int> compute_time = parse_resource_request_return(compute_request);
    process->global_time_tracker = process->global_time_tracker + compute_time[0];
    process->computation_time = process->computation_time - compute_time[0];

    //Add check for missing deadline
    process_deadline_warning(process);
}

//Request resources from banker and tick clock
//This would modify the Process_Data master_string
//Here is where our initial request message is ordered
//Maybe change to be more consistent with other message structures...
//We'll see...
//Changed order, now we can send request_resources and end. instructions through this pipe
void request(std::string resource_request, Process_Data *process){
    process->global_time_tracker++;
    std::string message = std::to_string(process->global_time_tracker) + ";" +  process->process_name + ";" + resource_request;
    
    write(process->process_request_pipe[1], message.c_str(), message.size() + 1);
    //std::cout << message << " sent.\n";
    sem_post(process->process_request_sem);
    //std::cout << "Post succeeded for " << process->process_name << std::endl;

    char buffer[MESSAGE_SIZE];
    sem_wait(process->banker_reply_sem);
    //std::cout << "Reading Reply\n";
    read(process->process_request_pipe[0], buffer, MESSAGE_SIZE);
    std::string reply_message(buffer);

    //Resources received, now we can compute
    process->computation_time--;

    //std::cout << reply_message << " read from banker\n\n";

    //Process reply
    //Update resources
    //Update process clock
    add_resources_to_process_owned_resource(reply_message, process);

    //std::cout << "Completed adding to " << process->process_name << "\n\n";
    
    //Add check for missing deadline
    process_deadline_warning(process);
}

///////////////////
//Request Helpers//
///////////////////

//Send order: global_time;resource_return x y z;resourcex0;resourcex1;resourcey0;...
//Functions built for banker work here too
void add_resources_to_process_owned_resource(std::string message, Process_Data *process){
    //Splits message into vector and updates time for the process
    std::vector<std::string> return_data = partition_resource_return_message(message, process->global_time_tracker);
    
    //Sort resources for allocation into process
    std::vector<std::vector<std::string>> allocated_resources = sorted_resource_return(return_data);

    //I think we need to remove this
    //add_resources_to_master_string(allocated_resources, process->master_string);

    //Combine the two vectors
    combine_resources(allocated_resources, process->process_owned_resources);

    //std::cout << "Resources added into process_resources!\n";
    //for(const std::vector<std::string> &group : process->process_owned_resources){
    //    for(const std::string &resource : group){
    //        //std::cout << resource << " ";
    //    }
    //    //std::cout << std::endl;
    //}
    //std::cout << "\n\n";

}

//Combine allocated/returned resources back into their target resource pool
void combine_resources(std::vector<std::vector<std::string>> &allocated_resources, std::vector<std::vector<std::string>> &receiving_resource_pool){
    //std::cout << "Combining resources from banker into resource pool\n";
    
    for(int resource_group = 0; resource_group < receiving_resource_pool.size(); resource_group++){
        for(const std::string &resource : allocated_resources[resource_group]){
            receiving_resource_pool[resource_group].push_back(resource);
        }
        //Probably unnecessary, but OCD
        //allocated_resources[resource_group].clear();
        //Alphabetize each group for later return in order
        std::sort(receiving_resource_pool[resource_group].begin(), receiving_resource_pool[resource_group].end());
    }
    //Also probably unnecessary...
    //allocated_resources.clear();
}

//Add allocated resources to the master_string
//REMOVE//
void add_resources_to_master_string(const std::vector<std::vector<std::string>> &allocated_resources, std::vector<std::string> &master_string){
    //std::cout << "Adding resources to master_string\n";

    //if(allocated_resources.size() < 1){
    //    //std::cout << "EMPTY ALLOCATED RESOURCE MATRIX\n";
    //}

    for(const std::vector<std::string> &resource_group : allocated_resources){
        for(const std::string &resource : resource_group){
            //std::cout << resource << " added to master_string\n";
            master_string.push_back(resource);
        }
    }

    //Alphabetize master_string after adding the resources
    std::sort(master_string.begin(), master_string.end());
}

///////////////////
//End Request Helpers//
///////////////////

//Send resources back to banker and tick clock
//This would modify the Process_Data master_string
//No it wouldn't, just the stored resources
void release(std::string release_request, Process_Data *process){
    //std::cout << "Releasing [" << release_request << "] for " << process->process_name << "\n\n"; 

    //Increment time tracking variables and check for a miss
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

//Super similar to my End. function, but different enough that I'd need to redesign it and we're on a time crunch now
//So we're gonna have to break DRY today and Venkat will be angry
//He won't know why, but he'll be angry
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

//Add resources to master_string and tick clock
//This would modify the Process_Data clock and master_string
void use_resources(std::string use_request, Process_Data *process){
    //std::cout << "Called use_resources\n\n";
    std::vector<int> time_and_uses = parse_resource_request_return(use_request);
    //std::cout << "Successfully used parse_resource_request\n\n";
    process->global_time_tracker += time_and_uses[0];
    process->computation_time -+ time_and_uses[0];
    process_deadline_warning(process);

    //Add to master_string by multiple of time_and_uses[1]
    add_words_to_master_string(time_and_uses[1], process->master_string, process->process_owned_resources);

    //Combine all occurrence into words with attached english numbers
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

    //Alphabetize master_string after adding the resources
    //std::sort(master_string.begin(), master_string.end());    
}

//Reused from assignment1
//Modified to pbr
//Combine same words and increment english count
void combine_like_words(std::vector<std::string> &words_list){
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

    words_list = final_summed_words_list;
    //return final_summed_words_list;
}

//Reused from assignment1
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

//Reused from assignment1
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

//Reused from assignment1
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

///////////////////
//End Use Helpers//
///////////////////

//Remove resources from master_string and tick clock
//This would modify the Process_Data clock and master_string
void reduce_resources(std::string reduce_request, Process_Data *process){
    std::vector<int> time_and_reductions = parse_resource_request_return(reduce_request);
    process->global_time_tracker += time_and_reductions[0];
    process->computation_time -= time_and_reductions[0];
    process_deadline_warning(process);

    //Create vector of words to reduce the master_string by
    std::vector<std::string> resource_amounts_to_decriment = build_resource_reduction_list(time_and_reductions[1], process->process_owned_resources);
    combine_like_words(resource_amounts_to_decriment);

    //Perform reduction on master_string
    reduce(resource_amounts_to_decriment, process->master_string);
    //Unnecessary, but let's just add this to be certain
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
void reduce(std::vector<std::string> resource_amounts_to_decriment, std::vector<std::string> &master_string){

    //For each piece to reduce
    for(const std::string &resource_reduction : resource_amounts_to_decriment){
        std::stringstream full_reduction(resource_reduction);
        std::string reduction_word;
        std::string reduction_number = "";

        full_reduction >> reduction_word;
        if(is_number_word(reduction_word)){
            reduction_number = reduction_word;

            //If has number, then plural so remove 's'
            full_reduction >> reduction_word;
            reduction_word = reduction_word.substr(0, reduction_word.size() - 1);
        }

        //For each resource in our master string
        for(std::string &resource : master_string){
            std::stringstream full_resource(resource);
            std::string resource_word;
            std::string resource_amount = "";

            full_resource >> resource_word;
            if(is_number_word(resource_word)){
                resource_amount = resource_word;

                //If has number, remove plural 's'
                full_resource >> resource_word;
                resource_word = resource_word.substr(0, resource_word.size() - 1);
            }

            if(reduction_word == resource_word){
                //Compare number words
                std::string remaining_resource = reduce_number_words(reduction_number, resource_amount);
                if(remaining_resource == ""){
                    //Replace word in master_string and break to next iteration
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
    
    //Can return a min of "" which == 1
    int number = std::max(0, (resource_value - reduction_value - 1));
    return number_words[number];
}

///////////////////
//End Reduce Helpers//
///////////////////

//Print out master_string and which process it belongs to
//Tick clock?
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

//Changed end to end_process to make sure we aren't overloading any function called 'end'
//Send resources back to banker if not released
//Create a final master_string for this process?
//Tick clock? Probably not as process is over?
//Should end be a process at all or simply a notification?

void end_process(std::string end_request, Process_Data *process){
    //std::cout << "\n\nEnd. " << process->process_name << "\n\n";
    
    std::string resource_amount = build_end_resource_release_string(process->process_owned_resources);
    //std::cout << "Built resource_amount " << resource_amount << " for " << process->process_name << "\n\n";

    std::string resources_to_return = build_resource_return_string(process->process_owned_resources);
    //std::cout << "Built resources_to_return " << resources_to_return << " for " << process->process_name << "\n\n";
    
    std::string current_time = std::to_string(process->global_time_tracker);
    //std::cout << "Built current_time " << current_time << " for " << process->process_name << "\n\n";

    std::string message = current_time + ";" + end_request + ";" + resource_amount + resources_to_return;
    //std::cout << "Built message " << message << " for " << process->process_name << "\n\n";

    close(process->resource_return_pipe[0]);
    close(process->resource_return_pipe[1]);

    close(process->process_request_pipe[0]);
    write(process->process_request_pipe[1], message.c_str(), message.size() + 1);
    close(process->process_request_pipe[1]);
    sem_post(process->process_request_sem);

    //After we've closed our pipes and posted the message, end the process
    //Could maybe move this into the main loop of the process?
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
            //Including the ";" is intentional as then we can just add it to the end of our return message as is
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
//Can build the map inside the banker instead of passing in a variable

//When we initialize, create a std::vector<std::string> processes that is the list of active processes in our program per the file input
//While processes, loop until each returns the end. instruction
//Once the last process has sent end. we may end our banker process and close the program
//This assumes that all processes are fully executeable
void banker(std::string SCHEDULER_TYPE, std::vector<std::string> process_data_file, std::vector<std::string> resource_data_file){
    //Initialize all banker variables from file data input
    //Allows modular use and for banker to be run twice in the same main()
    
    int global_time = 0;

    std::vector<int> resource_group_information = {};
    std::vector<Process_Data*> process_data_vector = parse_process_file_contents(process_data_file, resource_group_information);
    std::vector<sem_t> banker_resource_semaphores = build_banker_resource_semaphores(resource_group_information);

    std::unordered_map<std::string, Process_Data*> process_map = build_process_map(process_data_vector);

    std::vector<std::vector<std::string>> banker_resources = parse_resource_file_contents(resource_data_file);
    
    std::vector<std::string> active_processes;
    build_active_process_vector(process_data_vector, active_processes);
    
    //End initialization//

    //Begin Process Creation//

    for(const std::string &name : active_processes){
        Process_Data *process = process_map[name];

        int pid = fork();

        if(pid == 0){
            //std::cout << "Child process name: " << process->process_name << "\n\n";
            //std::string message = "request_resources 1 1 1";
            //std::cout << message << "\n\n";
            //request(message, process);

            //use_resources("use_resources 1 2", process);
            //std::cout << "\n\nPRINTING USED RESOURCES\n";
            //print_resources_used(process);

            //message = "release 0 0 1";
            //release(message, process);

            //use_resources("use_resources 1 2", process);
            //std::cout << "\n\nPRINTING USED RESOURCES\n";
            //print_resources_used(process);

            //reduce_resources("reduce_resource 1 3", process);
            //std::cout << "\n\nPRINTING REDUCED RESOURCES\n";
            //print_resources_used(process);

            //message = "end.";
            //end_process(message, process);
            //exit(0);

            run_process(process);
        }
    }

    //End Process Creation//
    
    while(active_processes.size() > 0){
        
        //std::vector<std::string> request_queue; //Request process names in order from highest to lowest precedence// Replace with active_processes?
        std::unordered_map<std::string, std::vector<std::string>> requests_map;
        
        //This gives us our raw request vector
        //Could maybe use a map here as well with active_processes being the key?
        //Pass by reference may also be a good idea for future iterations
        //std::cout << "Taking requests\n\n";
        banker_take_requests(active_processes, process_map, requests_map, global_time);


        //std::cout << "INITIAL REQUEST LIST IN BANKER:\n\n";
        //for(int i = 0; i < intial_request_list.size(); i++){
        //    std::cout << intial_request_list[i] << std::endl;
        //}


        //Check if any requests are end.
        //If they are, return any resources sent with them
        //Return resources
        //Remove that process from our queue


        //Update the global time and recompile these into std::vector<std::vector<std::string>>
        //Don't need a requests_matrix, just use a map and a list of WHO is requesting
        //Can remove this function, replaced wiht banker_take_requests
        //add_to_requests_map_populate_request_queue(request_queue, requests_matrix_map, intial_request_list, global_time);


        //std::cout << "REQUESTS IN BANKER MAP\n\n";
        //for(const std::string &process_name : request_queue){
        //    for(int j = 0; j < requests_matrix_map[process_name].size(); j++){
        //        std::cout << requests_matrix_map[process_name][j] << " ";
        //    }
        //    std::cout << std::endl;
        //}

        //std::cout << "Ordering request\n\n";
        order_requests(SCHEDULER_TYPE, process_map, active_processes);

        //std::cout << "\n\nREQUEST QUEUE ORDER:\n";
        //for(const std::string &process_name : active_processes){
        //    std::cout << process_name << std::endl;
        //}

        //assess_requests(request_queue, request_queue);

        //NEXT STEP IN BANKER LOGIC APPLICATION
        //Attempt to grant requests while requests pending
        //Remove any completed processes
        //std::cout << "Assessing and executing\n\n";
        assess_and_execute_requests(active_processes, requests_map, process_map, banker_resources, global_time, banker_resource_semaphores);
    }

    //CLEANUP//
    cleanup(process_data_vector);
    destroy_banker_semaphores(banker_resource_semaphores);
}

//Read instruction from pipe and add it to our request vector
//Use our active_processes vector instead
//Order will change, and once it's gone it's gone, so there's nothing to iterate over
void banker_take_requests(std::vector<std::string> active_processes, std::unordered_map<std::string, Process_Data*> process_map, std::unordered_map<std::string, std::vector<std::string>> &request_map, int &global_time){
    std::vector<std::string> request_list;

    //Can add an if(active_processes) check later if needed

    for(const std::string &process_name : active_processes){
        std::string message = banker_take_request(process_name, process_map);
        request_map[process_name] = parse_request_message(message, global_time);
        //request_list.push_back(banker_take_request(process_name, process_map));
    }

    //TESTING MESSAGES
    
    //std::cout << "REQUESTS:\n";
    //for(const std::string &process_name : active_processes){
    //    for(const std::string &message_piece : request_map[process_name]){
    //        std::cout << message_piece << " ";
    //    }
    //    std::cout << std::endl;
        //parse_request(request_list[i], test_time);
    //}
}

//Don't need to attach the name in the message, we have that to talk to the process
//Expected input: time;request a b c || time;end.;return_resource a b c;resourcea;resourceb;...
std::string banker_take_request(std::string process_name, std::unordered_map<std::string, Process_Data*> process_map){
    char buffer[MESSAGE_SIZE];
    //std::cout << "Waiting to read request from process " << process_name << std::endl;
    sem_wait(process_map[process_name]->process_request_sem);
    //std::cout << "Reading request\n";
    read(process_map[process_name]->process_request_pipe[0], buffer, MESSAGE_SIZE);
    std::string message(buffer);
    //std::cout << "Read " << message << " from process.\n\n";

    return message;
}

//Break request into name, request, time
//Pass request by reference?
//Ugh, lazy me hard coded the length
//Should maybe standardize the message to be time first?
//For requests from the process this may not be necessary, but it makes the overall logic a bit more consistent, I think
//So we always expect to read the same data from a pipe
//Though the request from the pipe could maybe be considered a special case, it may be better to plan for more modularity
//I.E. no lazy ass hard coding like I did here
//Expected input: time;name;request;extra_data...
//This allows us to send end. through the initial request pipe as well as any leftover resources that haven't been returned through normal means yet
//Don't need name
//Expected input: time;request a b c || time;end.;return_resource a b c;resourcea;resourceb;...
std::vector<std::string> parse_request_message(std::string &request, int &global_time){
    std::vector<std::string> parts;

    //Reordered, now we take the time first
    int semicolon_location = request.find(';');
    std::string part(request.substr(0, semicolon_location));
    int reported_time = std::stoi(part);
    global_time = std::max(global_time, reported_time);

    request = request.substr(semicolon_location + 1);

    //std::cout << "Request parts:\n";
    while(request.find(';') != std::string::npos){
        int semicolon_location = request.find(';');
        std::string part(request.substr(0, semicolon_location));
        parts.push_back(part);
        request = request.substr(semicolon_location + 1);

        //std::cout << part << std::endl;
    }

    //Adds the last bit after the last ';'
    parts.push_back(request);

    //std::cout << "Reported time: " << reported_time << "\n\n";

    return parts;
}

//Populate our request_queue with names
//Updates the global clock when it parses a new request
//Removes requests from the request_list once they're added to the matrix
//Adds the requests to the map for quick lookup
void add_to_requests_map_populate_request_queue(std::vector<std::string> &request_queue, std::unordered_map<std::string, std::vector<std::string>> &requests_matrix_map, std::vector<std::string> &request_list, int &global_time){
    std::vector<std::string> single_request;

    for(int request = request_list.size() - 1; request >= 0; request--){
        single_request = parse_request_message(request_list[request], global_time);
        request_queue.push_back(single_request[0]);
        requests_matrix_map[single_request[0]] = single_request;
        request_list.pop_back();
    }
}

//Orders requests for a given precedence
//PBR request_queue to order it for precedence
//Takes DRY out back and does an Old Yeller...
//Will fix time permitting
void order_requests(std::string SCHEDULER_TYPE, std::unordered_map<std::string, Process_Data*> process_map, std::vector<std::string> &active_processes){
    if(active_processes.size() <= 1){
        return;
    }
    
    if(SCHEDULER_TYPE == "LLF"){
        //Placeholder
        std::unordered_set<std::string> processes_we_have_added;
        std::vector<std::string> final_order;
        std::vector<std::string> processes_with_same_laxity;

        for(const std::string &request : active_processes){
            std::string current_process_name = request;

            for(const std::string &compared_request : active_processes){
                std::string compared_process_name = request;

                //If current process is in processes_we_have_added OR the same as our current process skip it
                if(processes_we_have_added.count(compared_process_name) || current_process_name == compared_process_name){
                    continue;
                }

                //If we've added it, find the first non-added name and replace it
                //Then continue iterating through the rest of the process names
                if(processes_we_have_added.count(current_process_name)){
                    current_process_name = compared_process_name;
                    continue;
                }

                //Compute psuedo-laxity
                //ACTUAL laxity == deadline - global_time - computation_time
                //This is because I leave deadline static and tick tracked global time up as a comparison metric
                //So ACTUAL deadline == deadline - global_time
                //Which is why our laxity is only psuedo-laxity
                //BUT that's ok becuase it's a consistent factor across all processes so we're still actually sorting by laxity
                //We re-compute current_laxity each time here as current_process may change inside the loop
                int current_process_laxity = process_map[current_process_name]->deadline - process_map[current_process_name]->computation_time;
                int compared_process_laxity = process_map[compared_process_name]->deadline - process_map[compared_process_name]->computation_time;

                //If they have the same laxity, add the new process name to a vector for later use
                if(current_process_laxity == compared_process_laxity){
                    processes_with_same_laxity.push_back(compared_process_name);
                    //std::cout << "Adding " << compared_process_name << " to tie vector\n";
                    continue;
                }

                //If the new laxity is less than the old laxity, replace it and remove any processes that were added to the equal_vector before this loop
                if(compared_process_laxity < current_process_laxity){
                    current_process_name = compared_process_name;
                    //std::cout << "Clearing tie vector\nSize before clear: " << processes_with_same_deadline.size() << std::endl;
                    processes_with_same_laxity.clear();
                    //std::cout << "Size after clear: " << processes_with_same_deadline.size() << std::endl;
                }

            }
            /////////
            //Copy pasta//
            /////////

            //If we have ties
            //Just had to change the vector name and the comparison sign for computation time
            if(processes_with_same_laxity.size() > 0){
                //std::cout << "Size of processes_with_same_deadline: " << processes_with_same_deadline.size() << std::endl;
                processes_with_same_laxity.insert(processes_with_same_laxity.begin(), current_process_name);

                for(const std::string &process_name : processes_with_same_laxity){
                    current_process_name = process_name;
                    
                    for(const std::string &compare_to_name : processes_with_same_laxity){
                        //If we've added it or it's the same as the initial process skip it
                        if(processes_we_have_added.count(compare_to_name) || current_process_name == compare_to_name){
                            continue;
                        }
                        
                        //If we've added it, replace it with one not yet added
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

                //Clear the vector once it's been used so we don't keep adding to it
                //std::cout << "Emptying tie vector\n";
                processes_with_same_laxity.clear();
                //std::cout << "Size of tie vector: " << processes_with_same_deadline.size() << std::endl;
                
            } else {
                //Final check to make sure we don't double add the last request name
                if(!processes_we_have_added.count(current_process_name)){
                    final_order.push_back(current_process_name);
                    processes_we_have_added.insert(current_process_name);
                }
            }

            /////////
            //End//
            /////////
        }
        return;
    }

    std::unordered_set<std::string> processes_we_have_added;
    std::vector<std::string> final_order;
    std::vector<std::string> processes_with_same_deadline;

    //Change to nested for loop instead of while loop
    for(const std::string &request : active_processes){
        std::string current_process_name = request;

        for(const std::string &compared_request : active_processes){
            std::string compared_process_name = compared_request;

            //If current process is in processes_we_have_added OR the same as our current process skip it
            if(processes_we_have_added.count(compared_process_name) || current_process_name == compared_process_name){
                continue;
            }

            //If we've added it, find the first non-added name and replace it
            //Then continue iterating through the rest of the process names
            if(processes_we_have_added.count(current_process_name)){
                current_process_name = compared_process_name;
                continue;
            }

            //If they have the same deadline, add the new process name to a vector for later use
            if(process_map[current_process_name]->deadline == process_map[compared_process_name]->deadline){
                processes_with_same_deadline.push_back(compared_process_name);
                //std::cout << "Adding " << compared_process_name << " to tie vector\n";
                continue;
            }

            //If the new deadline is sooner than the old deadline, replace it and remove any processes that were added to the equal_vector before this loop
            if(process_map[compared_process_name]->deadline < process_map[current_process_name]->deadline){
                current_process_name = compared_process_name;
                //std::cout << "Clearing tie vector\nSize before clear: " << processes_with_same_deadline.size() << std::endl;
                processes_with_same_deadline.clear();
                //std::cout << "Size after clear: " << processes_with_same_deadline.size() << std::endl;
            }

        }

        //If we have ties
        if(processes_with_same_deadline.size() > 0){
            //std::cout << "Size of processes_with_same_deadline: " << processes_with_same_deadline.size() << std::endl;
            processes_with_same_deadline.insert(processes_with_same_deadline.begin(), current_process_name);

            for(const std::string &process_name : processes_with_same_deadline){
                current_process_name = process_name;
                
                for(const std::string &compare_to_name : processes_with_same_deadline){
                    //If we've added it or it's the same as the initial process skip it
                    if(processes_we_have_added.count(compare_to_name) || current_process_name == compare_to_name){
                        continue;
                    }
                    
                    //If we've added it, replace it with one not yet added
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

            //Clear the vector once it's been used so we don't keep adding to it
            //std::cout << "Emptying tie vector\n";
            processes_with_same_deadline.clear();
            //std::cout << "Size of tie vector: " << processes_with_same_deadline.size() << std::endl;
            
        } else {
            //Final check to make sure we don't double add the last request name
            if(!processes_we_have_added.count(current_process_name)){
                final_order.push_back(current_process_name);
                processes_we_have_added.insert(current_process_name);
            }
        }

        
    }

    //Set our request_queue to our newly ordered vector
    //Could use std::move() if I wanna be fancy
    active_processes = final_order;

}

//Split the request type off the full request string
std::string take_first_part(const std::string &request){
    std::stringstream full_request(request);
    std::string part;

    full_request >> part;

    return part;
}

//Assess requests and execute or pause 
//We will only be recieving requests through that pipe

void assess_and_execute_requests(std::vector<std::string> &active_processes, std::unordered_map<std::string, std::vector<std::string>> &requests_matrix_map, std::unordered_map<std::string, Process_Data*> process_map, std::vector<std::vector<std::string>> &banker_resources, int &global_time, std::vector<sem_t> &banker_resources_sem){
    std::unordered_set<std::string> processes_marked_for_removal;
    //std::cout << "Beginning assesment\n\n";

    for(const std::string &process_name : active_processes){
        //request_type will always be request_resources x y z
        //Can send end. through request pipe
        std::string request_type = take_first_part(requests_matrix_map[process_name][1]);
        //std::cout << "Taking first part of reqeust [" << request_type << "] for " << process_name << "\n\n";

        //End. will have the 'release' instruction attached to it
        if(request_type == "release"){
            //Return all resources
            //Resources should be vectorized in our request map
            //{end.}, {release x y z}, {resource x}, {resource y},...
            //std::cout << process_name << " is returning it's remaining resources\n";
            //Add a check to see if they're empty?
            std::vector<std::vector<std::string>> returning_resources = sorted_resource_return(requests_matrix_map[process_name], 2);
            //If returning_resources empty, don't execure return
            execute_return(returning_resources, banker_resources, banker_resources_sem);

            //Remove process_name from active_processes
            //Mark process_name for removal
            //Remove at end of loop
            //Upon termination, the process_name should be removed from our active_processes vector
            processes_marked_for_removal.insert(process_name);
            continue;
        }
        
        //std::cout << "Granting resources to " << process_name << std::endl;
        banker_grant_resources(process_name, requests_matrix_map[process_name][1], process_map, banker_resources, active_processes, global_time, banker_resources_sem);
        
    }

    //Create a loop somewhere that assess how many processes are still operating
    //Track active processes
    //Remove them once end. is returned through resource_returns
    //While active_processes(check_for_returns)
    //std::cout << "Checking for any resource returns after allocations completed\n\n";
    check_for_resource_returns(active_processes, process_map, banker_resources, global_time, banker_resources_sem);

    //Check for returns before reducint active_processes vector JUST in case the end is read before the returns are read
    //Remove marked names here
    if(processes_marked_for_removal.size() > 0){
        //std::cout << "We have processes marked for removal!\n\n";
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
std::vector<int> parse_resource_request_return(const std::string &request){
    std::stringstream full_request(request);
    std::string part;
    std::vector<int> resource_amounts;
    full_request >> part;

    //std::cout << "part to be discarded: " << part << "\n\n";
    while(full_request >> part){
        if(part != ";"){
            //std::cout << "Part to convert to int: " << part << "\n\n";
            int amount = std::stoi(part);
            resource_amounts.push_back(amount);
        }
    }

    return resource_amounts;
}

//Grant resource request for a process
void banker_grant_resources(std::string process_name, std::string request, std::unordered_map<std::string, Process_Data*> process_map, std::vector<std::vector<std::string>> &banker_resources, std::vector<std::string> &request_queue, int &global_time, std::vector<sem_t> &banker_resources_sem){
    std::vector<int> resource_request_amounts = parse_resource_request_return(request);

    //Verify quantity on hand
    for(int resource_category = 0; resource_category < banker_resources.size(); resource_category++){
        //If not enough on hand, then we check for returns
        //Should include check to be sure the process isn't requesting more than it's max, but that is outside the scope of this assignment
        if(resource_request_amounts[resource_category] > banker_resources[resource_category].size()){
            check_for_resource_returns(request_queue, process_map, banker_resources, global_time, banker_resources_sem);
        }
    }


    //Resource semaphore?
    //One per type with init == num_resources avail for that group
    //IF not enough check for returns
    //process_returns();

    //If still not enough...?

    //If enough, send resources
    distribute_resources(request, process_name, process_map, resource_request_amounts, banker_resources, banker_resources_sem, global_time);

}

//UNTESTED//
//Check to see if any resources are ready to be returned
void check_for_resource_returns(std::vector<std::string> &request_queue, std::unordered_map<std::string, Process_Data*> process_map, std::vector<std::vector<std::string>> &banker_resources, int &global_time, std::vector<sem_t> &banker_resources_sem){
    //Loop through all active processes looking for resources ready to return
    //Be careful, if I delete a process in this loop it will break
    for(const std::string &process_name : request_queue){
        if(sem_trywait(process_map[process_name]->resource_return_sem) == 0){
            banker_return_resources(process_name, process_map, banker_resources, global_time, banker_resources_sem);
        }
    }
}

//UNTESTED//
//Banker returns resources from process to the banker_resources
//Might need to be moved out of the request queue...
//Banker may only control resource allocation, not process return allocation?
void banker_return_resources(std::string process_name, std::unordered_map<std::string, Process_Data*> process_map, std::vector<std::vector<std::string>> &banker_resources, int &global_time, std::vector<sem_t> &banker_resources_sem){
    //Read from resource return pipe
    //sem_wait already called
    //Could build a read() function for this...
    char buffer[MESSAGE_SIZE];
    //std::cout << "Return available from " << process_name << "\n";
    read(process_map[process_name]->resource_return_pipe[0], buffer, MESSAGE_SIZE);
    std::string message(buffer);

    //For now assume this only processes 'release' requests followed by the string of resources
    //We can attach the process name to the return just to keep it consistent and reuse other functions
    //Parse the message
    //DISREGARD process_name, resource_return x y z, resource x1, resources x2, resource y1, ..., global_time
    //DONE Alright... build a damn function to parse this, my original function needs to be updated anyway
    //Send order: global_time;resource_return x y z;resourcex0;resourcex1;resourcey0;...
    std::vector<std::string> return_data = partition_resource_return_message(message, global_time);

    //Pass in the first part of return_data as that is the request
    //This gives us a vector of the number of each resource returned to the bank
    //THIS IS NOW DONE INSIDE sorted_resource_return to build the resource 2D vector

    //Sort resources for return
    std::vector<std::vector<std::string>> returning_resources = sorted_resource_return(return_data);

    //Execute return
    execute_return(returning_resources, banker_resources, banker_resources_sem);
}

//Cuts the return message into a vector and updates global time
//Also use for sending resources to process
//Standardize form to stated below
std::vector<std::string> partition_resource_return_message(std::string &message, int &global_time){
    //Send order: global_time;resource_return x y z;resourcex0;resourcex1;resourcey0;...
    int semicolon_location;
    std::string part;
    std::vector<std::string> partitioned_resource_message;

    //Can turn this into a function to just cut off the first part of a string up to ';'
    //Update time
    semicolon_location = message.find(';');
    part = message.substr(0, semicolon_location);
    global_time = std::max(std::stoi(part), global_time);
    message = message.substr(semicolon_location + 1);

    //Grab return command
    //Can combine this into the main loop below
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

    //Adds the last bit of the message left over after the last ';' is found
    //Alternatively we could end the message with ';' and then avoid this
    //But that could do funky things for our subst cut for the last cut...
    //Ig we could build a function and just have a check..........
    partitioned_resource_message.push_back(message);

    return partitioned_resource_message;
}

//Sort resources for return
//Resources should be placed in return in order of group type NOT alphabetical
//As long as order is preserved, we don't need to track name -> type when replacing
//Our index changes based on if this is a release or end. instruction
std::vector<std::vector<std::string>> sorted_resource_return(std::vector<std::string> resource_info, int resource_index){
    std::vector<std::string> resource_type_group = {}; //To be sure we have SOMETHING for each group, even if it's null, to maintain order
    std::vector<std::vector<std::string>> sorted_resources;
    std::vector<int> resource_return_amount = parse_resource_request_return(resource_info[resource_index - 1]);

    for(const int &resource_group : resource_return_amount){
        for(int resource = 0; resource < resource_group; resource++){
            //Location in resource_info is linear and starts at 1
            std::string resource_name = resource_info[resource_index++];
            resource_type_group.push_back(resource_name);
        }
        //This *should* still give us an empty vector if no resources are added for a group
        sorted_resources.push_back(resource_type_group);
        resource_type_group.clear();
    }

    return sorted_resources;
}

//UNTESTED//
//Execute the resource return
//Will need to decide how and where we declare the semaphores
void execute_return(std::vector<std::vector<std::string>> &returning_resources, std::vector<std::vector<std::string>> &banker_resources, std::vector<sem_t> &banker_resources_sem){
    for(int resource_group = 0; resource_group < banker_resources.size(); resource_group++){
        for(const std::string &resource : returning_resources[resource_group]){
            banker_resources[resource_group].push_back(resource);
            sem_post(&banker_resources_sem[resource_group]);
        }
        //Re-alphebetize the resource list
        std::sort(banker_resources[resource_group].begin(), banker_resources[resource_group].end());
    }

    //Probably unnecessary, but just to be sure we don't interfere with other returns
    returning_resources.clear();
    //std::cout << "Resources returned to banker!\n";
}

//Send resources to a process
//global_time;resourcex0;resourcex1;...
//No need to send the request amounts as these are already on hand for the process
//May change that later
void distribute_resources(std::string original_request, std::string process_name, std::unordered_map<std::string, Process_Data*> process_map, std::vector<int> resource_request_amounts, std::vector<std::vector<std::string>> &banker_resources, std::vector<sem_t> &banker_resources_sem, int global_time){
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

    //Can separate this into a separate function to make the writing distinct from the creation of the string, but I don't think that's necessary

    //May be time to reevaluate MESSAGE_SIZE variable
    write(process_map[process_name]->process_request_pipe[1], message.c_str(), message.size() + 1);
    sem_post(process_map[process_name]->banker_reply_sem);
}

//Builds a queue for easy popping and vector manipulation
//'Transfers' data from vector into queue
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

//UNTESTED//
//Build semaphores for each resource allowing our banker to track and gate their use
//Include only the size of each resource in the group info vector, not the num_groups
//That can be determined with just resource_info.size()
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
        
    //Call execv()
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
