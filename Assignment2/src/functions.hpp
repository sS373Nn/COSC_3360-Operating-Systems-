#ifndef FUNCTIONS_H
#define FUNCTIONS_H

//Shared includes here
#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <queue>

#ifdef _WIN32

//Stub shit here
int pipe(int* pipe_file);

void close(int pipe_file_part);

#else

//Linux includes here
#include <unistd.h>
#include <stdio.h>
#include <semaphore.h>

#include <sys/mman.h>
#include <fcntl.h>

#endif

//Stores all data for a given process
//Should have rolled into a class but by the time I realized that it would have been more work to refactor than to just finish out using it as a struct
struct Process_Data{
    std::string process_name;
    int deadline;
    int computation_time;
    int global_time_tracker;
    std::vector<std::string> instructions;
    int* missed_deadline_flagged;

    std::vector<std::vector<std::string>> process_owned_resources;
    
    int* process_request_pipe;
    int* resource_return_pipe;
    std::vector<std::string> master_string;
    sem_t* process_request_sem;
    sem_t* banker_reply_sem;
    sem_t* resource_return_sem;

    Process_Data(std::string process_name, int deadline, int computation_time, std::vector<std::string> instructions);
    ~Process_Data();
};

//Helper function to build empty resource pool for process_data struct
//Ran into issues building an empty 2D vector and my solution was to simply initialize it to the correct size based on the number of resource groups but leaving each one empty
//Allows me to track which resource belongs where for allocation and return to bank without having to id each one by name and create a map or some other data struct to return them
//Used in parse_process_data function where we create our Process_Data pointers
std::vector<std::vector<std::string>> build_initialization_template_for_resource_pool(int num_resources);

/////////////////////
//LEGACY FUNCTIONS//
/////////////////////

//Legacy code
//Processes an instruction string
//Don't know remember this was for originally, I never finished it
//I think it got lost in the shuffle of a refactor somewhere along the way
//I suspect it was a 'planning function' that I wrote the basic code for and expected to need later
void process_instruction(std::vector<std::string> instruction_vector);

//Legacy code
//Turns an instruction string into a vector of strings
//Used for early testing and data parsing
std::vector<std::string> vectorize_instruction(std::string instruction);

//Legacy code
//Add allocated resources to the master_string
//Originally thought that request was supposed to add resources to master_string
//Removed once I realized I was mistaken
void add_resources_to_master_string(const std::vector<std::vector<std::string>> &allocated_resources, std::vector<std::string> &master_string);

//Legacy code
//Create request_matrix from raw requests NOPE
//PBR Allows us to add to our request matrix if an iteration hasn't completed ALL requests
//Instead we create our map and a vector of process names to use as lookup and then order
//PBR still works tho
//Also updates the global clock when it parses a new request
//Removes requests from the request_list once they're added to the matrix
//Added a map for quick lookup with the request order vector
void add_to_requests_map_populate_request_queue(std::vector<std::string> &request_queue, std::unordered_map<std::string, std::vector<std::string>> &requests_matrix_map, std::vector<std::string> &request_list, int &global_time);

/////////////////////
//END LEGACY FUNCTIONS//
/////////////////////

/////////////////////
//TEST FUNCTIONS//
/////////////////////

//Legacy code and testing
//Prints out all the info in one of the Process_Data structs
void print_process_data(Process_Data *process_info);

//Legacy code and testing
//Prints out the contents of a resource matrix read in from the file
//This was one of my first test functions as in this project I became much more accustomed using pass by reference iterators for my loops
//And I have A LOT of them
void print_resource_matrix(std::vector<std::vector<std::string>> resource_matrix);

/////////////////////
//END TEST FUNCTIONS//
/////////////////////

//Read in the file names and save them in a std::vector<std::string> to use as needed
//Reads and stores the file names from the command line as std::vector<std::string> for future access and use
std::vector<std::string> file_name_read_from_function_call(char *argv[]);

//Reads in file given a std::string filename and stores it as a vector for further use
//This is my file reading function
//Given the std::string filename it parses contents into std::vector<std::string> to pass to the banker where they will be broken down further
std::vector<std::string> read_file_into_vector(std::string filename);

/////////////////////
//Parse Input Files//
/////////////////////

//Parses the std::vector<std::string> file_contents of the first input file
//Returns a std::vector<Process_Data*> that contains each Process_Data object for our fork()s
std::vector<Process_Data*> parse_process_file_contents(std::vector<std::string> file_contents, std::vector<int> &resource_availability);

//Build Process_Data map for quick lookup
//This builds a std::unordered_map<std::string, Process_Data*> from our std::vector<Process_Data*> process_data_vector
//Prevents us from having to iterate through a vector to find a Process_Data object
//Instead we can simply use the std::string process_name as a key meaning we don't have to constantly reorder our whole vector of Process_Data* pointers
//Instead we can keep a std::vector<std::string> process_names and sort that into whatever order we'd like, leaving our main data structure static and easy to reference
//Here I used std::vector<std::string> active_processes which was a list of the processes that had not yet processed the end. instruction
std::unordered_map<std::string, Process_Data*> build_process_map(std::vector<Process_Data*> process_data_vector);

//Build active process vector
//This function built that std::vector<std::string> active_processes
//Used to track and dictate process order by the banker
void build_active_process_vector(const std::vector<Process_Data*> &process_data_vector, std::vector<std::string> &active_processes);

//Parses the parts of the resource input file
//Parses the std::vector<std::string> file_contents of the second input file
//Similar to the parse_process_file_contents function, this function splits the resources from the resource file into a std::vector<std::vector<std::string>> banker_resources
std::vector<std::vector<std::string>> parse_resource_file_contents(std::vector<std::string> file_contents);

//Slices apart file_contents vector into smaller sections over each "" for each section in the file
//Takes std::vector<std::string> file_contents passed by reference as it needs to modify the original data struct
//Returns std::vector<std::string> vector_slice which is the file_contents vector from [0] -> the first ""
//This function does some relatively heavy lifting for it's size
//It's how we differentiate between the initial resource data and each process in the process data file
//In between each chunk of relevant data we have a std::string ""
//This iterates through our std::vector<std::string> file_contents and returns the location of that piece
//It cuts off the chunk of data preceding the "" and then we use that differently for different parts
//I.E. initially we can find the number of resources and processes and quantities for each
//The following num_processes chunks are data specifically related to each process that we may then parse into each individual Process_Data struct
std::vector<std::string> slice_vector_to_next_space(std::vector<std::string> &file_contents);

//Create resources availability vector
//Takes const std::vector<std::string> &file_slice which is the first section sliced off of our file_contents from our first input file
//Returns a std::vector<int> resource_availability that is the number of each resource available
//We use this info to also build our banker semaphores later
std::vector<int> determine_resource_availability(const std::vector<std::string> &file_slice);

//Parse the string into integers to add to a vector using stringstream
//Reused from first assignment so nothing new or fancy
//Takes a std::string string_of_ints and turns it into a std::vector<int> int_vector
std::vector<int> vectorize_string_to_int(std::string string_of_ints);

//Create resource demand matrix but it's technically unused
//We create the process_demand_matrix but never assign it to any process as per the professor's instruction, no process will ask for more than it should
//So I've left it here to show that we CAN create and use it, but we don't as it's technically out of the scope of this assignment
//Takes a const std::vector<std::string> &file_slice which is the first section sliced off of our file_contents from our first input file
//Returns a std::vector<std::vector<int>> process_data_vector that is the maximum demand of each process
std::vector<std::vector<int>> determine_process_demand_matrix(const std::vector<std::string> &file_slice);

//Build our Process_Data object
//Takes a std::vector<std::string> file_slice and int num_resources
//That is the slice of data relevant to the process and the number of resource groups in the second input file (though it's determined in the first file)
Process_Data* parse_process_data(std::vector<std::string> file_slice, int num_resources);

//Cut our resource text into individual resources
//Repurposed and modified from assignment1 so nothing crazy here either
//Takes a std::string word_string and returns a std::vector<std::string> line_vector
std::vector<std::string> vectorize(std::string word_string);

//Remove colons from resource file pieces to leave us only the resources
//Considered modifying to include the R1, etc and type but decided against it as we only need resources themselves given our storage method
//Takes a std::vector<std::string> file_contents that is a single resource group built from the second input file
//Returns a std::vector<std::string> clean_vector that is the resource words without their ';' attached
//Built this early in the dev process, lots of room for improvement, would probably pass and iterate by reference now if I were to rebuild it
std::vector<std::string> clean_resource_file_vector(std::vector<std::string> file_contents);

/////////////////////
//End Parse Input//
/////////////////////

/////////////////////
//Process Functions//
/////////////////////

//Run our process using it's instructions vector
//One of the last functions I built
//Iterates over a Process_Data internal list of instructions we built in our parse_process_data function
//Calls each function based on the instruction given
//Exits when end. instruction passed
void run_process(Process_Data *process);

//Checks if deadline missed, displays notification
//Called in any function that costs time or updates time received from banker
//Takes a Process_Data *process and compares deadline to global_time_tracker
//Once a deadline is missed, prints a warning message and sets bool missed_deadline_flagged to true
void process_deadline_warning(Process_Data *process);

//Tick global_time_tracker forward and computation_time down by compute_time
//Takse std::string compute_request and Process_Data *process as input
void compute(std::string compute_request, Process_Data *process);

//Request resources from banker and tick global_time_tracker forward and computation_time down by one
//Takes std::string resource_request and Process_Data *process
//Builds a standardized message to send to the banker through the process_request_pipe
//Banker waits for each process to pass a request as per the intstructions any process must allow a different process to post a request before it may request again
//Here is where one of the process gates occurs as the process must wait for the banker's reply via a specified banker_reply_sem semaphore
//Only once the banker has approved and allocated resources for this process will it sem_post() and allow the process to take those resources and proceed
//And only once the resources have been granted does the computation_time for the process decrease
//It then adds the resources to it's process_owned_resources
void request(std::string resource_request, Process_Data *process);

///////////////////
//Request Helpers//
///////////////////

//Helper to add resources from the banker into the process_resource pool and master_string
//Takes std::string message and Process_Data *process as input
//Uses helpers built for the banker to return resources to it's banker_resources pool to allocate resources to the process_owned_resources pool
//Adds received resources into process_resources
//Send order: global_time;request_resource x y z;resourcex0;resourcex1;resourcey0;...
void add_resources_to_process_owned_resource(std::string message, Process_Data *process);

//Combine allocated/returned resources back into their target resource pool
//Takes std::vector<std::vector<std::string>> &allocated_resources and std::vector<std::vector<std::string>> &receiving_resources_pool
//Combines the resources from the first 2D vector into the second with the appropriate resource group in mind
//Assumes all resources are unique
void combine_resources(std::vector<std::vector<std::string>> &allocated_resources, std::vector<std::vector<std::string>> &receiving_resource_pool);

///////////////////
//End Request Helpers//
///////////////////

//Tick global_time_tracker forward and computation_time down by one and send resources and tracked_global_time back to banker
//Expected output: time;release x y z;resourcex;resourcey;...
//Takes std::string release_request and Process_Data *process as input
//Writes to resource_return_pipe and sem_post(resource_return_sem)
void release(std::string release_request, Process_Data *process);

///////////////////
//Release Helpers//
///////////////////

//Builds a std::string of the resources to be returned to the banker
//Takes std::string release_request and std::vector<std::vector<std::string>> &process_owned_resources as input
//Returns a std::string resources_being_released
//Super similar to my End. function, but different enough that I'd need to redesign it and we're on a time crunch now
//So we're gonna have to break DRY today and Venkat will be angry
//He won't know why, but he'll be angry
std::string build_resource_release_string(std::string release_request, std::vector<std::vector<std::string>> &process_owned_resources);

///////////////////
//End Release Helpers//
///////////////////

//Add resources to the master_string tick global_time_tracker forward and computation_time down by given time
//Takes std::string use_request and Process_Data *process as input
//Helpers used here were borrowed and mutated from assignment1
//Improved my alphabetizing method which I found to be buggy
void use_resources(std::string use_request, Process_Data *process);

///////////////////
//Use Helpers//
///////////////////

//Alphabetize function for std::sort to accommodate for the number words and capitalization
//Helper function to sort each word in the master string in alphabetic order WITHOUT considering it's number word
//Takes const std::string &first_word and const std::string &second_word as input
//Using stringstream checks to see if they have attached number words
//Compares the two non-number words alphabetically
bool alphabetize(const std::string &first_word, const std::string &second_word);

//Helper for alphabetize
//Takes const std::string &word as input
//Function name says it all
bool is_number_word(const std::string &word);

//Helper for alphabetize
//Takes std::string &word as input
//Pass by reference allows us to call it easily as an internal helper for alphabetize
//Self explanatory function name, used so we don't compare 'A' to 'a' etc
//DOES NOT modify the original resource word, just the internal stringstream copy
void to_lower(std::string &word);

//Add words from process_owned_resources to master_string y times
//Takes int word_multiplier, std::vector<std::string> &master_string and const std::vector<std::vector<std::string>> &process_owned_resources as input
//Adds every word currently stored in process_owned_resources to the master_string word_multiplier times
//Does NOT combine them
void add_words_to_master_string(int word_multiplier, std::vector<std::string> &master_string, const std::vector<std::vector<std::string>> &process_owned_resources);

//Reused from assignment1
//Modified to pbr
//Combine same words and increment english count
//THIS combines them but still has limitations as can't handle numbers that sum > 10
void combine_like_words(std::vector<std::string> &words_list);

//Reused from assignment1
//Find number word if it's attached to string
//Takes a single string comprised of a word or a number word a space and another word
std::string find_number_word(std::string word_to_check);

//Reused from assignment1
//Detach a word from it's number
std::string detached_word(std::string number_word, std::string original_word);

//Reused from assignment1
//Add english numbers
//Takes two strings that are the number words from two separate words we are combining
std::string add_number_words(std::string word_one, std::string word_two);

///////////////////
//End Use Helpers//
///////////////////

//Remove resources from the master_string tick global_time_tracker forward and computation_time down by given time
//Takes std::string reduce_request and Process_Data *process as input
//Uses SOME helpers from use, but had to make some minor changes
//Could have probably redesigned to be more modular and work with fewer functions, but here we are
void reduce_resources(std::string reduce_request, Process_Data *process);

///////////////////
//Reduce Helpers//
///////////////////

//Builds the resource list and amounts for each resource to be deducted from the master_string
//Takes int multiplier and const std::vector<std::vector<std::string>> &process_owned_resources as input
//Instead of adding to the master_string, it creates it's own vector of resources to remove with each resource from the process_owned_resources included multiplier times
//These are all combined into single resource names with number words using combine_like_words in reduce_resource function for comparison against the master_string
std::vector<std::string> build_resource_reduction_list(int multiplier, const std::vector<std::vector<std::string>> &process_owned_resources);

//Perform the reduction on the master_string
//Takes const std::vector<std::string> &resource_amounts_to_decriment and std::vector<std::string &master_string as input
//Using our helper reduce_number_words, we iterate over resource_amounts_to_decriment, reducing the matching resource in master_string
void reduce(const std::vector<std::string> &resource_amounts_to_decriment, std::vector<std::string> &master_string);

//Similar to add_number_words logic
//Takes std::string reduction and std::string resource_amount as input
//reduction is the number word to subtract from the number word resource_amount
//Almost the inverse of add_number_words with the minimum output being "" or 1
std::string reduce_number_words(std::string reduction, std::string resource_amount);

///////////////////
//End Reduce Helpers//
///////////////////

//Print out the master_string and the process it belongs too
//Increment global_time_tracker forward and computation_time down
//Takes a Process_Data *process as input
void print_resources_used(Process_Data *process);

///////////////////
//Print Helpers//
///////////////////

//Helper to convert our stored std::vector<std::string> master_string into a printable string
//Easier to manipulate and add to a stored vector holding the individual resource entries as opposed to a static string that would have to be parsed and reparsed for every use or reduce command
std::string build_master_string(const std::vector<std::string> &master_string);

///////////////////
//End Print Helpers//
///////////////////

//Send all resources back to banker if not released and exit() the process
//Takes std::string end_request and Process_Data *process as input
//IF there are any resources left in the std::vector<std::vector<std::string>> process_owned_resources
//Compile them into a final message to send back into process_request_pipe
//This signals the banker that the process has completed and to check add any remaining resources back to the std::vector<std::vector<std::string>> banker_resources pool
//Also allows the banker to remove this process from the std::vector<std::string> active_processes so that this process is no longer in the resource queue
//Does NOT tick global_time_tracker or computation_time
void end_process(std::string end_request, Process_Data *process);

///////////////////
//End. Helpers//
///////////////////

//Builds the release x y z message for a return message
//Takes const std::vector<std::vector<std::string>> &process_owned_resources as input
//Compiles them into a std::string release_string of all resource quantities in each group, mirroring the format of the release x y z and request a b c instruction
//This allows the banker to parse into which resource group that each resource in the message should be stored
std::string build_end_resource_release_string(const std::vector<std::vector<std::string>> &process_owned_resources);

//Finds the number of current resources in each group held by a process
//Takes const std::vector<std::vector<std::string>> &process_owned_resources as input
//Returns a std::vector<int> resources_in_each_group that correlates to the current number of resources in each group in the process_owned_resources
//Used to build the std::string release_string
std::vector<int> find_amount_of_resources_in_each_group(const std::vector<std::vector<std::string>> &process_owned_resources);

//Builds resource return string when ALL resources should be returned
//Similar to the release function, but I built this one first and was too lazy/rushed to modify it and refactor all the code needed to make it work for both
//Would probably be relatively easy if I just passed in the std::vector<int> into it to dictate what gets returned from the process_owned_resources
//Takes std::vector<std::vector<std::string>> &process_owned_resources as input
//Returns a std::string resources of all resources compiled into a single string separated by ';'
std::string build_resource_return_string(std::vector<std::vector<std::string>> &process_owned_resources);

///////////////////
//End End. Helpers(lol)//
///////////////////

/////////////////////
//End Process Functions//
/////////////////////

/////////////////////
//Banker Functions//
/////////////////////

//The banker controls the flow within the main parent process
//Admittedly, the banker here likely has a bit more control than the simple banker algorithm
//But our banker uses the banker algorithm to control the flow of it's processes
//Takes as input std::string SCHEDULER_TYPE, std::vector<std::string> process_data_file and std::vector<std::string> resource_data_file
//Banker tracks global_time across all processes, synchronizing them with the data sent and received from pipes
//Banker does NOT increment time, only control the flow of the processes and sending and receiving time updates between them
//Banker parses input file data and creates the std::vector<Process_Data*> process_data_vector, std::vector<sem_t> banker_resource_semaphores, std::unordered_map<std::string, Process_Data*> process_map, std::vector<std::vector<std::string>> banker_resources and std::vector<std::string> active_processes
//Banker iterates through each Process_Data process and creates a fork() for it to run
//While there remain processes in std::vector<std::string> active_processes the banker:
//Takes requests which it stores in it's std::unordered_map<std::string, std::vector<std::string>> requests_map
//Orders those requests with order_requests, either using EDF or LLF depending on the SHEDULER_TYPE
//Assesses and executes those reqeusts with assess_and_execute_active_requests
//Once a process has sent the end. request to the banker, it is removed from std::vector<std::string> active_processes
//Upon removal of all active_processes banker calls:
//cleanup(process_data_vector) to safely destroy and deallocate memory from it's pointers
//destroy_banker_semaphores(banker_resource_semaphores) to properly clean up memory for it's semaphores allocated to each resource group
void banker(std::string SCHEDULER_TYPE, std::vector<std::string> process_data_file, std::vector<std::string> resource_data_file);

//Compiles requests read by banker_take_request into requests vector and updates global_time
//Takes std::vector<std::string> active_processes, std::unordered_map<std::string, Process_Data*> process_map, std::unordered_map<std::string, std::vector<std::string>> &request_map and int &global_time as input
//Breaks message into a std::vector<std::string> to store in the requests_map using the process_name as the key
//This saves us from using a vector and trying to track which process' instruction is stored at which location, etc
void banker_take_requests(std::vector<std::string> active_processes, std::unordered_map<std::string, Process_Data*> process_map, std::unordered_map<std::string, std::vector<std::string>> &request_map, int &global_time);

//Takes a single request from a process as a helper to banker_take_requests
//Takes std::string process_name, std::unordered_map<std::string and Process_Data*> process_map as input
//Calls sem_wait(process_request_sem) as a means to gate banker access to request pipe before the process has posted it's request
//However, after access banker does NOT call sem_post() as this leaves the control for this pipe with the process itself
//Expected input: time;request a b c || time;end.;return_resource a b c;resourcea;resourceb;...
std::string banker_take_request(std::string process_name, std::unordered_map<std::string, Process_Data*> process_map);

//Break request into name, request and update global time
//Takes std::string &request and int &global_time as input
//Returns std::vector<std::string> parts that are the message read for the pipe broken into parts over the ';'
//Input into the pipes is standardized so we know exactly what to expect and where in the message
//This is then added to our std::unordered_map<std::string, std::vector<std::string>> &request_map
std::vector<std::string> parse_request_message(std::string &request, int &global_time);

//Orders requests for a given precedence
//Takes DRY out back and does an Old Yeller...
//A TON of room for improvement on this function
//Could probably break it into two main helper functions primary_order and secondary_order and then apply each function over the given sorting type
//But here we are
//Venkat would probably fail me retroactively if he ever saw this function
//Takes std::string SCHEDULER_TYPE, std::unordered_map<std::string, Process_Data*> process_map and std::vector<std::string> &request_queue as input
//Returns the reordered std::vector<std::string> active_processes
//For the given sorting type, loops over the current active_processes by reference iterator
//Sets std::string current_process_name to request
//Inner loop then iterates through the entire std::vector<std::string> active_processes as well
//Sets std::string compared_process_name = compared_request
//If we've added compared_request to our std::unordered_set<std::string> processes_we_have_added || current_process_name == compared_process_name skip it
//If we've added current_process_name to std::unordered_set<std::string> processes_we_have_added then we set current_process_name = compared_process_name then skip this inner loop
//This is to ensure we loop over EVERY process in our active_processes, without this, the outer loop is shortened prematurely
//For LLF we calculate laxity of current_process_name and compared_process_name then compare them
//If they're equal we add the compared_process_name to our std::vector<std::string> processes_with_same_laxity
//This tracks every process that has the same laxity so that we can compare and order them in relation to each other IF that laxity is the lowest of our remaining processes
//If compared_laxity is < current_laxity we set current_process_name = compared_process_name AND clear the std::vector<std::string> processes_with_same_laxity as we these (if they're stored here) no longer have the lowest laxity that we've seen
//Once all the inner loop has completed we will be left with either a process with the least laxity or a std::vector<std::string> processes_with_same_laxity
//If there are any processes in that vector, we insert the currently held current_process_name into the vector
//Repeat the above steps with nearly identical logic on the std::vector<std::string> processes_with_same_laxity
//Except now we don't compare ties
//Each std::string current_process_name is added to std::vector<std::string> final_order upon completion of a full inner loop
//Once the outer loop is completed, we have ordered our processes based on LLF (or EDF) into std::vector<std::string> final_order
//Set active_processes = final_order
//If necessary, I can provide a better line-by-line explanation inside the function implementation, but I think the logic and process should be relatively straight forward to follow
void order_requests(std::string SCHEDULER_TYPE, std::unordered_map<std::string, Process_Data*> process_map, std::vector<std::string> &request_queue);

//Split the request type off the full request string
//Helper function for assess_and_execute_requests to parse either request or end.
//(Though end. is denoted by "release" as that's how I built the end_request function to compile it's message)
//Takes const std::string &request as input
//Returns the first part of the request
//I.E. "request 1 0 5" returns "request"
std::string take_first_part(const std::string &request);

//Assess requests and execute or pause
//Takes std::vector<std::string> &active_processes, std::unordered_map<std::string, std::vector<std::string>> &requests_matrix_map, std::unordered_map<std::string, Process_Data*> process_map, std::vector<std::vector<std::string>> &banker_resources, int &global_time and std::vector<sem_t> &banker_resources_sem as input
//Iterates over const std::string &process_name in active_requests
//Reads and assess the requests sent by the active_processes stored in std::unordered_map<std::string, std::vector<std::string>> &requests_matrix_map
//If a process sends an end. request, it is marked for removal by adding it to std::unordered_set<std::string> processes_marked_for_removal
//The resources sent with the request are returned to the banker and the banker_resource_semaphores are decremented per resource returned
//If the request is not an end request, then the banker checks resource availability and grants resources as they become available
//Then checks to see if there are any more resource returns sent through the resource_return_pipe by any actual release instructions internal to processes
//Finally if processes_marked_for_removal.size() > 0 we find the process that needs to be removed and remove it from active_processes
void assess_and_execute_requests(std::vector<std::string> &active_processes, std::unordered_map<std::string, std::vector<std::string>> &requests_matrix_map, std::unordered_map<std::string, Process_Data*> process_map, std::vector<std::vector<std::string>> &banker_resources, int &global_time, std::vector<sem_t> &banker_resources_sem, const std::vector<std::string> &all_system_processes, std::unordered_map<std::string, std::vector<std::vector<int>>> &allocation_map);

//Parse resource request/return
//This little helper was used A LOT especially in process specific functions
//Takes const std::string &request as input
//Where request == "request/release a b c..." and converts a b c to their corresponding ints
//Returns std::vector<int> that is the number of attached resources for each resource group
//This may then be used to build our std::vector<std::vector<std::string>> insert_2D_resource_vector_here OR simply parse the int associate with a compute x instruction etc
std::vector<int> parse_resource_request_return(const std::string &request);

//Grant resource request for a process
//Takes std::string process_name, std::string request, std::unordered_map<std::string, Process_Data*> process_map, std::vector<std::vector<std::string>> &banker_resources, std::vector<std::string> &request_queue, int &global_time and std::vector<sem_t> &banker_resources_sem
//Assesses whether or not there are enough resources on hand to meet a request then once there are, allocates them to the reqeusting process
//If there aren't then checks for returns from all processes using sem_trywait so as not to freeze on a process that has no resources to return
//Then calls distribute_resources to perform the actual resource allocation via pipe
void banker_grant_resources(std::string process_name, std::string request, std::unordered_map<std::string, Process_Data*> process_map, std::vector<std::vector<std::string>> &banker_resources, std::vector<std::string> &request_queue, int &global_time, std::vector<sem_t> &banker_resources_sem, const std::vector<std::string> &all_system_processes, std::unordered_map<std::string, std::vector<std::vector<int>>> &allocation_map);

//Check to see if any resources are ready to be returned
//Takes std::vector<std::string> &request_queue, std::unordered_map<std::string, Process_Data*> process_map, std::vector<std::vector<std::string>> &banker_resources, int &global_time and std::vector<sem_t> &banker_resources_sem as input
//Iterates through every active process and calls sem_trywait(resource_return_sem)
//If a process has added resources to their resource return pipe, it will signal and the banker will return those resources to allocate them to another process
//If not, the process will continue to run as sem_trywait() does not halt process operation on a failure, simply moves on without attempting to access the pipe
void check_for_resource_returns(std::vector<std::string> &request_queue, std::unordered_map<std::string, Process_Data*> process_map, std::vector<std::vector<std::string>> &banker_resources, int &global_time, std::vector<sem_t> &banker_resources_sem, std::unordered_map<std::string, std::vector<std::vector<int>>> &allocation_map);

//Banker returns resources from process to the banker_resources
//Takes std::string process_name, std::unordered_map<std::string, Process_Data*> process_map, std::vector<std::vector<std::string>> &banker_resources, int &global_time and std::vector<sem_t> &banker_resources_sem as input
//Once returned resources have been identified, this reads them from resource_return_pipe for the given process_name
//It parses the data with other helper functions partition_resource_return_message
//And creates the 2D resource vector std::vector<std::vector<std::string>> returning_resources with sorted_resource_return()
//Then calls execute_return to merge the returned resources back into the banker_resources pool
void banker_return_resources(std::string process_name, std::unordered_map<std::string, Process_Data*> process_map, std::vector<std::vector<std::string>> &banker_resources, int &global_time, std::vector<sem_t> &banker_resources_sem, std::unordered_map<std::string, std::vector<std::vector<int>>> &allocation_map);

//Cuts the return message into a vector and updates global time
//Takes std::string &message and int &global_time as input
//Send order: global_time;resource_return x y z;resourcex0;resourcex1;resourcey0;...
//Updates the global_time sent from the process
//Returns std::vector<std::string> partitioned_resource_message cutting over ';'
std::vector<std::string> partition_resource_return_message(std::string &message, int &global_time);

//Sort resources for return
//Resources should be placed in return in order of group type NOT alphabetical
//As long as order is preserved, we don't need to track name -> type when replacing
//Takes std::vector<std::string> resource_info and int resource_index as input
//Modify the resource_index based on whether it's an end. request or release as the end request has an extra part associated in the message string that needs to be accounted for
//Returns std::vector<std::vector<std::string>> sorted_resources 2D vector to be merged with the 2D banker_resources vector
std::vector<std::vector<std::string>> sorted_resource_return(std::vector<std::string> resource_info, int resource_index = 1);

//Execute the resource return
//Takes std::vector<std::vector<std::string>> &returning_resources, std::vector<std::vector<std::string>> &banker_resources and std::vector<sem_t> &banker_resources_sem as input
//Performs the actual merger of received resources and the banker_resources
//Calls sem_post(banker_resources_sem) for each resource in each resource group that is returned
//These resource semaphores are how we gate the wait for resource availability from the banker
void execute_return(std::vector<std::vector<std::string>> &returning_resources, std::vector<std::vector<std::string>> &banker_resources, std::vector<sem_t> &banker_resources_sem);

//Send resources to a process
//global_time;resourcex0;resourcex1;...
//Takes std::string original_request, std::string process_name, std::unordered_map<std::string, Process_Data*> process_map, std::vector<int> resource_request_amounts, std::vector<std::vector<std::string>> &banker_resources, std::vector<sem_t> &banker_resources_sem and int global_time as input
//Compiles the resource message that the banker sends through the process_request_pipe
//Sends the first a b c... resources from group x y z...
//Uses a std::queue<std::string> resources_in_group to avoid expensive vector manipulation and allows us to easily allocate resources_in_group.front()
//Then resources_in_group.pop() to track what resources are removed and retained by the banker
//Each group is allocated to the queue and then deallocated back to the banker via helper functions
//Worth considering changing the banker's resource structure to be a std::vector<std::queue> for ease and expediency of use
//Not sure if that's a valid std::vector declaration though
//Finally function performs write(process_request_pipe)
//And sem_post(banker_reply_sem) to allow the process to access the reply pipe and continue with it's operation
void distribute_resources(std::string original_request, std::string process_name, std::unordered_map<std::string, Process_Data*> process_map, std::vector<int> resource_request_amounts, std::vector<std::vector<std::string>> &banker_resources, std::vector<sem_t> &banker_resources_sem, int global_time, const std::vector<std::string> &all_system_processes, std::unordered_map<std::string, std::vector<std::vector<int>>> &allocation_map);

//Builds a queue for easy popping and vector manipulation
//'Transfers' data from vector into queue
//Helper function mentioned abvoe
//Takes std::vector<std::string> &resource_group as input
//Returns a std::queue<std::string> resrouces_in_group
std::queue<std::string> build_resource_queue_from_group(std::vector<std::string> &resource_group);

//Return the remaining resources to the banker vector
//Second part of the helper functions mentioned above to convert the queue back into the banker's 2D vector
//Takes std::queue<std::string> &resources_in_group and std::vector<std::string> &banker_resources_group as input
void return_remaining_resources_from_queue_to_vector(std::queue<std::string> &resources_in_group, std::vector<std::string> &banker_resources_group);



//Build semaphores for each resource allowing our banker to track and gate their use
//Include only the size of each resource in the group info vector, not the num_groups
//That can be determined with just resource_info.size()
//Takes std::vector<int> resource_group_information as input
std::vector<sem_t> build_banker_resource_semaphores(std::vector<int> resource_group_information);


//LAST MINUTE FUNCTIONS TO FIX AN OVERSIGHT//

//Prints the system state upon resources allocation
void print_state_of_system(std::string process_serviced, const std::vector<std::vector<std::string>> &banker_resources, const std::vector<std::string> &system_processes, const std::unordered_map<std::string, std::vector<std::vector<int>>> &allocation_map, const std::unordered_map<std::string, Process_Data*> &process_map);

//Allocation map tracks the various parts in the state_of_system print statement
//Had to update and slightly modify some of my data structures, but this addon is fairly lightweight and the functions should be easy to parse as they're not too complex
//I use a map to store what's needed and what's been allocated in a 2D vector that only tracks counts, not resource names
std::unordered_map<std::string, std::vector<std::vector<int>>> build_allocation_map(std::unordered_map<std::string, Process_Data*> process_map, std::vector<std::string> active_processes);

//Adds to the process allocation tracker variables and reduces the resources needed for any process that receives them
void add_to_allocation_map_and_reduce_resources_needed(std::vector<int> resources_to_add, std::string process_name, std::unordered_map<std::string, std::vector<std::vector<int>>> &allocation_map);

//Removes allocation numbers when a process returns it's resources to the banker
//More accurately, the banker removes them once it receives the resources
void reduce_allocation_map(std::vector<int> resources_to_add, std::string process_name, std::unordered_map<std::string, std::vector<std::vector<int>>> &allocation_map);

//Helper function that reads through each process and tallies the total number of resources requested through it's instruction set
void total_resources_needed(std::vector<int> &resources_demanded, Process_Data *process);


//END LAST MINUTE FUNCTIONS//

/////////////////////
//End Banker Functions//
/////////////////////

/////////////////////
//Execv Functions//
/////////////////////

//Borrowed structure from assignment1 implementation and modified for this assignment
//Instead of mixing up our output, will fully run the first simulation, then create a new program to run the second
void execute_LLF_with_execv(std::string input_file_one, std::string input_file_two);

/////////////////////
//End Execv Functions//
/////////////////////

/////////////////////
//Cleanup Functions//
/////////////////////

//Probably unnecessarily complicated solution to double deleting my pipe ints
void cleanup(std::vector<Process_Data*> &my_processes);

//Clean up the semaphores created for the banker to gate resources
void destroy_banker_semaphores(std::vector<sem_t> &banker_resource_semaphores);

/////////////////////
//End Cleanup Functions//
/////////////////////

#endif
