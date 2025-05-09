#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <iostream>
#include <unordered_set>
#include <vector>
#include <queue>
#include <stack>
#include <unordered_map>

#ifdef _WIN32

//Stubbed functions here

#else

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <semaphore.h>

#include <sys/mman.h>
#include <fcntl.h>

#endif

//This represents the frames stored in main memory
//This should probably have been a class that used inheritance
//But it works and it's easy to use, if not a bit bloated by the end of the project
struct Main_Memory{
    int memory_size;
    std::unordered_map<std::string, int> page_references;
    int page_replacements;
    int number_of_misses;
    int delayed_replacements;
    std::string replacement_type;
    std::unordered_set<std::string> frames;
    std::queue<std::string> FIFO_set;
    std::vector<std::string> LRU_set;
    std::stack<std::string> LIFO_set;
    std::vector<std::string> MRU_set; //Could just use the LRU vector
    std::unordered_map<std::string, int> LFU_page_frequency;
    std::vector<std::string> OPT_future_tokens;

    Main_Memory(std::string replacement_type, int memory_size);
};

//A pipe for each process
//Main feeds a steady flow of input tokens from the std::cin file to each process
struct Pipes{
    int* FIFO_pipe;
    int* LRU_pipe;
    int* LIFO_pipe;
    int* MRU_pipe;
    int* LFU_pipe;
    int* OPT_pipe;
    int* BONUS_pipe;

    Pipes();
    ~Pipes();
};

//A semaphore for each process
//Keeps each process synced for reading from it's pipe so no words are overwritten before they're used
//While no particular process needs to be synced with any other, they do need to be synced with main, where the words are read in and then sent to each pipe
//Since no token is stored long term (with the exception of OPT/OPT-MOBILE) this must be synced between the parent and children
struct Semaphores{
    sem_t* PRINT_sem;
    sem_t* FIFO_sem;
    sem_t* FIFO_ready_sem;
    sem_t* LRU_sem;
    sem_t* LRU_ready_sem;
    sem_t* LIFO_sem;
    sem_t* LIFO_ready_sem;
    sem_t* MRU_sem;
    sem_t* MRU_ready_sem;
    sem_t* LFU_sem;
    sem_t* LFU_ready_sem;
    sem_t* OPT_sem;
    sem_t* OPT_ready_sem;
    sem_t* BONUS_sem;
    sem_t* BONUS_ready_sem;

    Semaphores();
    ~Semaphores();
};

///////////////////
//LEGACY CODE//
///////////////////

//Only legacy function I kept.
//Last to be discarded.
//Opted instead to use a more simple design with std::unordered_set<std::string>
//Builds a helper std::unordered_map<std::string, bool> a_before_n to check future page order of occurrences of 'n' and 'a' and 'a'
//Unneeded if we track which pages we've already seen then we can simply compare based on first occurrence of each
std::unordered_map<std::string, bool> create_a_before_n(const std::vector<std::string> &future_tokens, const std::unordered_set<std::string> &memory);

//Builds vector of tokens to pass to our sorting algos
//Didn't need, just passed them straight to each process as they were read
std::vector<std::string> build_token_list();

///////////////////
//END LEGACY CODE//
///////////////////

//Read in our input args
//Pass by reference int memory_size, int look_ahead
void read_args(char* argv[], int &memory_size, int &look_ahead);

//Read in a token from input using std::cin
std::string get_token();

//Splits token over ':' and returns true of false for bandwidth
//Takes std::string &token as input, returns true if bandwidth
//Removes the bandwidth bit from the token leaving just the page
bool split_token(std::string &token);

//Reads the page before ':' in a token
//Takes const std::string &token as input and returns std::string page
//Used in a ton of helpers
std::string read_page_in_token(const std::string &token);

//////////////////////////////////////
//BEGIN PAGE REPLACEMENT FUNCTIONS//
//////////////////////////////////////

//Page replacement function that takes const int &look_ahead, const std::string &replacement_type, const int &memory_size, sem_t* reading_sem, sem_t* ready_sem, int* pipe, sem_t* print_sem as input
//Used for each process to pass the initialization parameters for that process unique Main_Memory
//Originally used a different function for EACH page replacement type, but was able to boil it down to a big more modular design by using helpers
//This is where each process calls it's semaphores and reads in each token to then pass to it's Main_Memory via take_token()
void page_replacement(const int &look_ahead, const std::string &replacement_type, const int &memory_size, sem_t* reading_sem, sem_t* ready_sem, int* pipe, sem_t* print_sem);

//Heavy lifter in conjunction with add_to_memory
//Takes std::string &token, Main_Memory &memory, std::queue<std::string> &offline_set as input
//Passes them into memory using add_to_memory or adds them to our offline_set that is the sequence of requests waiting for bandwidth
void take_token(std::string &token, Main_Memory &memory, std::queue<std::string> &offline_set);

//Takes const std::string &page, Main_Memory &memory
//At this point the token has been stripped down to it's actual page without the remaining bandwidth bit
//Add_to_memory has no interaction based on memory, it's only called by take_token IF we have bandwidth
//Was able to make a more modular design by using more If statements than is legally allowed in the state of Texas
//Please don't tell the Governor
void add_to_memory(const std::string &page, Main_Memory &memory);

//Completely pointless helper
//Expected it to need to do more
//It doesn't
//Just makes my call overly complex
//Takes const std::string &page, std::unordered_map<std::string, int> &page_references as input
//Increments the page_references int stored in main_memory
void increment_page_references(const std::string &page, std::unordered_map<std::string, int> &page_references);

//One of my many many many print helper funcitons
//Takes const std::unordered_map<std::string, int> &page_references as input
//Compiles a string to send to our main print function to... well... print page reference data
std::string print_page_references(const std::unordered_map<std::string, int> &page_references);

//Main print function
//Takes const Main_Memory &memory, sem_t* print_sem, const std::queue<std::string> &offline_set as input
//Solution to print buffers across processes interfering with each other
//If I turn each print statement into a massive string, it can't interrupt the print statement for any other process
//Leaving me with a 'mostly' intact print statement that is only interrupted by the end of main() that autoprints in the terminal
//Could probably add a wait() to fix that, but eh...
void print_memory(const Main_Memory &memory, sem_t* print_sem, const std::queue<std::string> &offline_set);

////////////////////
//FIFO HELPERS//
////////////////////

//Prints out the requested current set (stored as queue)
//Kind of.
//Helper for print_memory
//Takes const std::queue<std::string> &set as input
//Compiles a string to send to our main print function to print set data for FIFO
std::string print_set(const std::queue<std::string> &set);

////////////////////
//END FIFO HELPERS//
////////////////////

///////////////////
//LRU HELPERS//
///////////////////

//Sorts the LRU_set
//Takes const std::string &page, std::vector<std::string> &LRU_set as input
//Reorders our LRU vector
void LRU_sort(const std::string &page, std::vector<std::string> &LRU_set);

//Removes the LRU page and adds the new one to the set
//Takes const std::string &page, std::vector<std::string> &LRU_set as input
//Kicks out a page and adds a page from our order vector
void add_page_LRU(const std::string &page, std::vector<std::string> &LRU_set);

//Prints out the requested current set (stored as vector)
//Helper for print_memory
//Also used for MRU
//Takes const std::vector<std::string> &set as input
//Compiles a string to send to our main print function to print set data for LRU and/or MRU
std::string print_vector(const std::vector<std::string> &set);

///////////////////
//END LRU HELPERS//
///////////////////

////////////////////
//LIFO HELPERS//
////////////////////

//Prints out the requested current set (stored as stack)
//Helper for print_memory
//Takes const std::stack<std::string> &LIFO_set as input
//Compiles a string to send to our main print function to print set data for LIFO
std::string print_stack(const std::stack<std::string> &LIFO_set);

////////////////////
//END LIFO HELPERS//
////////////////////

///////////////////
//MRU HELPERS//
///////////////////

//Sorts the MRU_set
//Page to the 'left' is first to be removed
//Takes const std::string &page, std::vector<std::string> &MRU_set as input
void MRU_sort(const std::string &page, std::vector<std::string> &MRU_set);

//Removes the MRU page and adds the new one to the set
//Takes bool index, const std::string &page, std::vector<std::string> &MRU_set as input
//If MRU_set full, index == 1 otherwise we add the whole vector
//This lets us change how much of the original MRU set to include.
//If it's full, we drop the furthest left page (i.e. index == 1 or true)
void add_page_MRU(bool index, const std::string &page, std::vector<std::string> &MRU_set);

///////////////////
//END MRU HELPERS//
///////////////////

///////////////////
//LFU HELPERS//
///////////////////

//Another completely pointless helper
//Expected it to need to do more
//It doesn't
//Just makes my call overly complex
//Add to the count for a page frequency
//Takes const std::string &page, std::unordered_map<std::string, int> &page_frequency as input
//Increments the page frequency
void increment_page_frequency(const std::string &page, std::unordered_map<std::string, int> &page_frequency);

//Find the LFU currently in memory
//Takes const std::unordered_map<std::string, int> &page_frequency, const std::unordered_set<std::string> &memory as input
//Returns the current LFU that we have stored in memory
std::string find_LFU(const std::unordered_map<std::string, int> &page_frequency, const std::unordered_set<std::string> &memory);

//Perform LFU replacement
//Takes const std::string &page, std::unordered_map<std::string, int> &page_frequency, std::unordered_set<std::string> &memory as input
//Performs the page replacement by removing the LFU from memory
//Originally I wrote the function to also perform the add to memory, but that's done in a single step later, so I left it out for consistency sake
void replace_LFU(const std::string &page, std::unordered_map<std::string, int> &page_frequency, std::unordered_set<std::string> &memory);

//Prints out the requested current set (stored as unordered_set)
//Helper for print_memory
//Takes const std::unordered_set<std::string> &set as input
//Compiles a string to send to our main print function to print set data for LFU
std::string print_unordered_set(const std::unordered_set<std::string> &set);

///////////////////
//END LFU HELPERS//
///////////////////

///////////////////
//OPT HELPERS//
///////////////////

//Find the OPT currently in memory
//Takes const std::vector<std::string> &future_tokens, const std::unordered_set<std::string> &memory as input
//Returns the optimal page to remove
std::string find_OPT(const std::vector<std::string> &future_tokens, const std::unordered_set<std::string> &memory);

//Add token read from pipe into our future_tokens vector
//Replace token with the token at the front of the vector to send to replacement function
//Takes std::vector<std::string> &future_tokens, std::string &token as input
//This updates our look_ahead vector by adding the token to it, and then passes the token from the front of the vector to teh page replacement function
void add_to_future_tokens(std::vector<std::string> &future_tokens, std::string &token);

//Perform OPT replacement
//Insertion performed in code body, changed to keep consistent across all replacement types
//Takes const std::string &page, const std::vector<std::string> &future_tokens, std::unordered_set<std::string> &memory as input
//const std::string &page is unnecessary in the function call, but I used it in the original method before I standardized adding to memory
void replace_OPT(const std::string &page, const std::vector<std::string> &future_tokens, std::unordered_set<std::string> &memory);

//Empties our future_token vector applying page replacement as it does
//Takes Main_Memory &main_memory, std::queue<std::string> &offline_set as input
//Once all the tokens are read from the parent, OPT and OPT-MOBILE continue processing until their future_token vectors are empty
void empty_future_tokens(Main_Memory &main_memory, std::queue<std::string> &offline_set);

//Helper for empty_future_tokens
//Takes the 'first' token in our future_tokens vector and returns it as std::string current_token
//Takes std::vector<std::string> &future_tokens as input
std::string take_front_token(std::vector<std::string> &future_tokens);

///////////////////
//END OPT HELPERS//
///////////////////

/////////////////////
//BONUS HELPERS//
/////////////////////

//Find the BONUS OPT currently in memory
//Takes const std::vector<std::string> &future_tokens, const std::unordered_set<std::string> &memory as input
//This is only called on miss as it will replace SOMETHING from memory regardless of optimum or not, though it prioritizes an optimum replacement
//By creating a std::unordered_set<std::string> marked_pages we can track each time we see a new page in the sequence of future tokens
//If we've already seen this page, we don't care about what bandwidth it will have at subsequent locations
//We only care about it's first occurrence, so we operate with that in mind by taking the furthest 'a' that has yet to be seen
//If we can't find an 'a' then we'll settle for an 'n'
//And finally, if that is not possible, we will pick a random page from memory to discard
std::string find_BONUS_OPT(const std::vector<std::string> &future_tokens, const std::unordered_set<std::string> &memory);

//Perform OPT replacement
//Takes const std::string &page, const std::vector<std::string> &future_tokens, std::unordered_set<std::string> &memory as input
//Insertion performed in code body, changed to keep consistent across all replacement types
//Also does not need const std::string &page
void replace_BONUS_OPT(const std::string &page, const std::vector<std::string> &future_tokens, std::unordered_set<std::string> &memory);

//Find n_before_a
//This is the page we want to add to memory
//Takes const std::vector<std::string> &future_tokens, const std::unordered_set<std::string> &memory as input
//Returns the first page that we see in future_tokens that we do not have in memory and that does not have bandwidth
std::string find_n_before_a(const std::vector<std::string> &future_tokens, const std::unordered_set<std::string> &memory);

//Find unnecessary page in memory
//Finds tokens in memory, but not in future_tokens
//Takes const std::string &avoid_page, const std::vector<std::string> &future_tokens, const std::unordered_set<std::string> &memory as input
//Searches our current memory to see if we are holding onto any pages that will not be needed for the forseeable future
//Returns them as a std::unordered_set<std::string>
std::unordered_set<std::string> find_unneeded_pages(const std::string &avoid_page, const std::vector<std::string> &future_tokens, const std::unordered_set<std::string> &memory);

//On a hit, checks to see if we need to add a page to memory for any future pages
//Takes const std::string &avoid_page, const std::vector<std::string> &future_tokens, const std::unordered_set<std::string> &memory as input
//If we have unneeded pages in memory, replaces one with a page that will be needed in the future
//If no unneeded pages in memory finds a page in memory that will have bandwidth the next time we see it and replaces that one
//Replace the furthest one out
std::string find_BONUS_OPT_on_hit(const std::string &avoid_page, const std::vector<std::string> &future_tokens, const std::unordered_set<std::string> &memory);

//On a hit, replaces a page with one that will not have bandwidth when it's needed
//Takes const std::string &avoid_page, Main_Memory &main as input
//Ensures we do not replace the page that we are currently using
//IF we find a page that we will need
//AND we have a valid replacement page that we can remove
//Remove that page and add the needed page for the future
void replace_BONUS_OPT_on_hit(const std::string &avoid_page, Main_Memory &main);

/////////////////////
//END BONUS HELPERS//
/////////////////////

//////////////////////////////////////
//END PAGE REPLACEMENT FUNCTIONS//
//////////////////////////////////////

///////////////////
//GENERAL HELPERS//
///////////////////

//Function to make main() look neater and easier to read
//Takes int &pid, const int &look_ahead, const int &memory_size, Pipes &message_pipes, Semaphores &replacement_type_sems as input
//Creates all of the forks we need for each page replacement type
void execute_replacement_strategies(int &pid, const int &look_ahead, const int &memory_size, Pipes &message_pipes, Semaphores &replacement_type_sems);

//Function to make main() look neater and easier to read
//Takes Pipes &message_pipes, std::string &token as input
//Performs our write() functions from parent to each child
void write_to_pipes(Pipes &message_pipes, std::string &token);

//Function to make main() look neater and easier to read
//Takes Semaphores &replacement_type_sems as input
//Executest the sem_post() command for the parent once it has written to each pipe
void post_read_semaphores(Semaphores &replacement_type_sems);

//Function to make main() look neater and easier to read
//Takes Semaphores &replacement_type_sems as input
//Executes the sem_wait() command for the parent as it waits for each child to read from it's respective pipe and release control back to parent
void wait_ready_semaphores(Semaphores &replacement_type_sems);

///////////////////
//END GENERAL HELPERS//
///////////////////

#endif
