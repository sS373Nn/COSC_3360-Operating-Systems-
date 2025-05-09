#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <iostream>
#include <vector>
#include <algorithm>

#ifdef _WIN32

//Mocked these functions for early troubleshooting.
//Irrelevant to process running in Linux.

int fork();

int pipe(int* pipe_file);

void close(int pipe_file_part);

void write(int pipe_file_part, const char serialized_vector[], int size);

void read(int pipe_file_part, char buffer[], int size);

#else

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <semaphore.h>

#endif

//Solution to being able to create a vector of int* and manage their memory
//Input is the number of pipes needed for our matrix with the exception of the final output pipe
class PipeManager{
    public:
        PipeManager(int number_of_pipes);
        ~PipeManager();

        std::vector<int*> pipes;
};

//Test functions and Legacy Code//
//Verify we can read in the command line properly (Psuedo canary test)//
//Only used for early testing to print out the args and testing execv() received the correct args
void print_command_line_args(int args, char** parts);
////

//Reading matrix from command line functions//
//Read matrix into a string from command line
//Legacy input reading
//No longer used
std::string read_matrix(int args, char** parts);

//Display vector as a 2D matrix
//Takes a matrix of integers and prints it out in an k x k matrix
//Legacy code to print matrix
void show_matrix_of_integers(std::vector<int> matrix);

//Process file contents into vector
//Takes a file name as a string, opens the file, outputs contents into string
//Legacy code to read in files
std::vector<std::string> read_file_into_vector(std::string file_name);

//Process vector into matrix
//Takes std::vector<std::string> from read_file_into_vector
//Legacy code to create matrix from string vector pulled from file contents
std::vector<std::vector<int>> create_matrix_from_file_contents(std::vector<std::string> file_contents);

//READING INPUT//
//Helper function to read stdin vector line from user for build_user_matrix
//Legacy code
//MOVE TO LEGACY CODE//
std::string read_matrix_line();

//Read stdin string from user
//Helper function for create_words_list
//Legacy code
//MOVE TO LEGACY//
std::string read_string();

//Close write ends after use
//Legacy code
//Instead we close them in the write_to_outgoing_pipes function
//MOVE TO LEGACY//
void close_write_ends_after_use(std::vector<int> used_pipe_locations, std::vector<int*> pipes);

//Close read ends after use
//Legacy code
//Instead we close them in the read_from_incoming_pipes function
//MOVE TO LEGACY//
void close_read_ends_after_use(std::vector<int> used_pipe_locations, std::vector<int*> pipes);

////END LEGACY CODE////

////MAIN FUNCTIONS////

//Main Process Part I//
//Lightweight main_process to output pipe count from each process fork
//Takes number of processes and process matrix entered by the user
//Prints solution to part 1 to the console
void parent_process_part_one(int number_of_processes, std::vector<std::vector<int>> process_matrix);
////

//Main Process Part II//
//Heavy lifting process for part 2, contains our main process, pipe(), fork(), dup() and execv() calls
//Takes number of processes and matrix entered by the user, also takes a semaphore and output pipe
//The output pipe will be passed to execv()
//execv() will call output_processor program for final output to console
//Many many many many helper functions called in main_process
//Parent process loops number_of_processes times, where number of processes is the number of processes entered by the user for the process matrix
//At the beginning of the loop, the parent process will call grab_word if the process is an input process to then pass that word to it's child process
//Upon initialization, each child will count the incoming and outgoing pipes it has and create it's own std::vector<std::string> process_words_list
//If the child is an input process, it will accept it's input word from parent
//If the child has incoming pipes it will first close the output_pipe read end then create it's incoming pipe index vector
//The child will then close the unused incoming pipe read ends
//Then the child will read from and close the pipes routed to it and add to it's process_words_list
//If the child has no incoming pipes, it will close the output_pipe read end as well as all other incoming pipes in the std::vector<int*> pipes vector
//If the child has outgoing pipes, the child will first close the output_pipe write end then create it's index vector of outgoing pipe locations
//The child will then close all unused outgoing (write) pipe ends
//The child will then write to and close all outgoing pipe ends
//If the child has no outgoing pipe ends then it is an output process and will instead write to the output_pipe
//The child calls dup() on output_pipe[1] write end and closes the output_pipe[1] end
//The child then closes all outgoing pipes in the std::vector<int*> pipes vector
//The child then calls super_serialize to convert the final output into a std::vector<char> to be passed into the output_pipe
//The child calls sem_wait() to block out the output_pipe and writes to the dup() output pipe, closes it and calls sem_post() to release the semaphore
//The child then calls exit() to end it's process
//Upon completion of the process loop, the parent process calls wait() to ensure all processes have successfully written to the output_pipe
//The parent then closes the output_pipe read end and calls fork() to create the output process with which to call execv()
//The final child process then creates the args to pass to execv() which calls output_processor
//Finally, the parent process closes the write end of the output_pipe
void parent_process(int number_of_processes, std::vector<std::vector<int>> process_matrix, sem_t &semaphore, int* output_pipe);

////END MAIN FUNCTIONS////

//MAIN PROCESS HELPER FUNCTIONS//

//Parse the matrix into integers in a vector using stringstream
//Takes a list of integers as a string, separates them over spaces with stringstream
//Originally part of legacy code for reading from file
//Repurposed to helper function for build_user_matrix
//MOVE OUT OF LEGACY SECTION//
std::vector<int> vectorize_string_to_int(std::string string_of_ints);

//Count incoming pipes for a process
//Used to count the pipes routed to a process
//Used in main_process_part_one and for various helpers in main_process to identify pipe routes
//Takes an the process number as an int and the std::vector<std::vector<int>> process matrix
int count_incoming_pipes_for_process(int process_number, std::vector<std::vector<int>> matrix);

//Count outgoing pipes for a process
//Used to count the pipes routed from a process
//Used in main_process_part_one and for various helpers in main_process to identify pipe routes
//Takes a single std::vector<int> process/row from the process matrix
int count_outgoing_pipes_for_process(std::vector<int> process_row);

//Count Input Processes
//Counts the number of processes in the matrix that are input processes
//Takes a std::vector<bool> that tracks which process is an input, counts it to use to determine how many input words to expect
//Probably a better way to track, but it works
int count_input_processes(std::vector<bool> input_process_vector);

//Check if process is input process
//Helper function for building input_process_tracker
//Takes the process/row number as an int and std::vector<std::vector<int>> process matrix
bool is_input_process(int process_number, std::vector<std::vector<int>> process_matrix);

//Create bool vector to track input processes
//Uses is_input_process to build a bool vector to track input processes
//Takes std::vector<std::vector<int>> process matrix to pass to is_input_process
std::vector<bool> input_process_tracker(std::vector<std::vector<int>> process_matrix);

//Build process matrix from user input
//Builds process matrix from user input and stores the number of processes
//Outputs std::vector<std::vector<int> process_matrix and sets the number_of_processes variable
//LOTS of helper functions
std::vector<std::vector<int>> build_user_matrix(int &number_of_processes);

//Create word_list vector from user input
//Builds std::vector<string> words list from user input
//Can accept all words at once, one per line
//Or can accept one word at a time
//Takes int number_of_input_processes to determine the number of words for the matrix
std::vector<std::string> create_words_list(int number_of_input_processes);

//Take word for the child process from words_list vector
//Takes the std::vector<std::string> vector_of_words built from user input
//Grabs one to pass to an input process fork()
std::string grab_word(std::vector<std::string>& vector_of_words);

//Turn vector into string to send through pipes
//Returns a single std::string from a std::vector<std::string> of multiple strings
//Passes this string to write() for pipe as a .c_str() for our inter-process pipes
std::string serialize(std::vector<std::string> word_list);

//Super-serialize a vector for output pipe reading
//I'm super serial guys! Manbearpig is real!
//Had to modify serialize to read multiple entries from a single pipe after we've added to the pipe with dup()
//Returns a std::vector<char> from a std::vector<std::string> words_list
//Then use to pass .data() to dup() pipe[1] to prevent a null character at the end of the string
//May have been a better solution, but this worked
std::vector<char> super_serialize(std::vector<std::string> word_list);

//Turn serialized string back into vector inside a process
//Used to recompile strings of one or more words pulled from pipes back into the process words list vector to be sorted and combined by combine_like_words function
//Takes the std::string word_string read from the pipe and adds it to the std::vector<std::string> word_list that we pass by reference
void vectorize(std::string word_string, std::vector<std::string> &word_list);

//Count number of pipes for our matrix
//Counts the number of pipes in user matrix to build our std::vector<int*> pipes in PipeManager class
//Takes std::vector<std::vector<int>> process_matrix entered by the user and returns an int number_of_pipes
int count_pipes_to_create(std::vector<std::vector<int>> process_matrix);

//Find all incoming pipes for a given row in our pipe_file vector
//Pipe tracking function used to build a vector of index numbers for a given process/row that correlates to the incoming pipe index in the PipeManager pipes vector
//Takes an int process_number that is the process or row and a std::vector<std::vector<int>> process_matrix entered by the user and returns a std::vector<int> pipe_index 
std::vector<int> incoming_pipe_index_vector(int process_number, std::vector<std::vector<int>> process_matrix);

//Find all outgoing pipes for a given row in our pipe_file vector
//Pipe tracking function used to build a vector of index numbers for a given process/row that correlates to the outgoing pipe index in the PipeManager pipes vector
//Takes an int process_number that is the process or row and a std::vector<std::vector<int>> process_matrix entered by the user and returns a std::vector<int> pipe_index 
//TL;DR Same as previous function but for outgoing pipes, logic different, input same
std::vector<int> outgoing_pipe_index_vector(int process_number, std::vector<std::vector<int>> process_matrix);

//Close unused write end of pipes in the pipes vector
//Closes unused write ends of pipes before we write to them in functions that have outgoing edges
//Takes std::vector<int> used_pipe_locations vector from outgoing_pipe_index_vector function and std::vector<int*> pipes from PipeManager
//Performs the close() operation on unused outgoing (write end) pipes
void close_unused_write_ends(std::vector<int> used_pipe_locations, std::vector<int*> pipes);

//Close unused read end of pipes in the pipes vector
//Closes unused read ends of pipes before we read from them in functions that have incoming edges
//Takes std::vector<int> used_pipe_locations vector from incoming_pipe_index_vector function and std::vector<int*> pipes from PipeManager
//Performs the close() operation on unused incoming (read end) pipes
//TL;DR same as above function  but for read end of pipes
//Could probably improve modularity by including the pipe operation in the call i.e. passing a 1 or 0 into the function to determine read or close
//May update time permitting
void close_unused_read_ends(std::vector<int> used_pipe_locations, std::vector<int*> pipes);

//Write serialized vector to all outgoing pipes then close those pipes
//Writes to all outgoing pipes from a process
//Takes std::vector<int> pipe_locations std::vector<int*> pipes from PipeManager and std::vector<std::string> word_list from the current process
//Calls the write function for each outgoing pipe for this process
void write_to_outgoing_pipes(std::vector<int> outgoing_pipe_locations, std::vector<int*> pipes, std::vector<std::string> word_list);

//Read from all incoming pipes and return a vector of strings then close those pipes
//Reads from all incoming pipes from a process
//Takes std::vector<int> pipe_locations std::vector<int*> pipes from PipeManager
//Calls the read function for each incoming pipe for this process and returns a std::vector<std::string> words_list of words read from the pipes
//Calls vectorize() and combine_like_words to combine and sort words from different pipes
std::vector<std::string> read_from_incoming_pipes(std::vector<int> index_pipe_locations, std::vector<int*> pipes);

//Add english numbers
//Helper function for combine_like_words function
//Takes two strings that are english number words between one and ten inclusive and adds them together.
//Outputs the result as another english number word
//Only has a maximum output of "ten" for this assignment.
//Anything higher will simply be output as "ten"
std::string add_number_words(std::string word_one, std::string word_two);

//Find number word if it's attached to string
//Helper function for combine_like_words
//Takes a single string comprised of a word or a number word a space and another word and returns the number word in english or "" if none attached
//Takes a std::string word_to_check as input and returns a std::string number_word to be used wiht detached_word and combine_like_words functions
std::string find_number_word(std::string word_to_check);

//Detach a word from it's number
//Helper function for combine_like_words function
//Identifies and isolates original noun from it's number word and plural 's' then returns that word
//Takes std::string number_word from find_number_word function and std::string original_word which is the whole original word with number, noun and space
//Returns the original noun as a std::string word with no number word or plural 's'
std::string detached_word(std::string number_word, std::string original_word);

//Combine same words and increment english count
//Main word combination function for words read from pipes
//Handles parsing and adding english number words with helper functions outlined above
//DOES NOT SORT but could probably include that functionality here instead of in main_process, will adjust time permitting
//Takes std::vector<std::string> words_list from a given processs as input returns a combined std::vector<std::string> of that words_list
//Originally done with pass by reference, removed during trouble shooting, may return as it is a cleaner method and was not the issue, again, time permitting
std::vector<std::string> combine_like_words(std::vector<std::string> words_list);

////output_processor functions////

//Format output for final display
//This displays the final output for part 2
//Takes std::vector<std::string> words_list read from main.cpp final output pipe and returns a std::string of the words properly combined with commas and a period
//Could call the print statement at the end of the function instead of returning a string, but I don't think it really matters
std::string format_final_output(std::vector<std::string> output_words_list);

//Clean up our words list vector to remove any straggling spaces from the way we added it to the pipes
//Removes empty strings from the words_list vector read into output_processor
//Currently works as intended, need to test further edge cases with more output processes as it may add empty strings with english words
//Takes std::vector<std::string> words_list passed by reference, cleans and updates it
void clean_words_vector(std::vector<std::string> &words_list);

#endif
