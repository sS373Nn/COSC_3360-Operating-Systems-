#include "functions.hpp"

#include <iostream>
#include <string>

#ifdef _WIN32

//Stubbed functions here

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
//I don't suspect we'll have page sizes that are 8 digits
const int MESSAGE_SIZE = 10;

/////////////////////
//BEGIN STRUCTS//
/////////////////////

//This represents the frames stored in main memory
Main_Memory::Main_Memory(std::string replacement_type, int memory_size) : replacement_type(replacement_type), memory_size(memory_size){
    page_replacements = 0;
    number_of_misses = 0;
    delayed_replacements = 0;
}

Pipes::Pipes(){
    FIFO_pipe = new int[2];
    LRU_pipe = new int[2];
    LIFO_pipe = new int[2];
    MRU_pipe = new int[2];
    LFU_pipe = new int[2];
    OPT_pipe = new int[2];
    BONUS_pipe = new int[2];

    if(pipe(FIFO_pipe) == -1){
        exit(0);
    }

    if(pipe(LRU_pipe) == -1){
        exit(0);
    }

    if(pipe(LIFO_pipe) == -1){
        exit(0);
    }

    if(pipe(MRU_pipe) == -1){
        exit(0);
    }

    if(pipe(LFU_pipe) == -1){
        exit(0);
    }

    if(pipe(OPT_pipe) == -1){
        exit(0);
    }

    if(pipe(BONUS_pipe) == -1){
        exit(0);
    }
};

Pipes::~Pipes(){
    if(FIFO_pipe){
        close(FIFO_pipe[0]);
        close(FIFO_pipe[1]);
        delete[] FIFO_pipe;
    }

    if(LRU_pipe){
        close(LRU_pipe[0]);
        close(LRU_pipe[1]);
        delete[] LRU_pipe;
    }

    if(LIFO_pipe){
        close(LIFO_pipe[0]);
        close(LIFO_pipe[1]);
        delete[] LIFO_pipe;
    }

    if(MRU_pipe){
        close(MRU_pipe[0]);
        close(MRU_pipe[1]);
        delete[] MRU_pipe;
    }

    if(LFU_pipe){
        close(LFU_pipe[0]);
        close(LFU_pipe[1]);
        delete[] LFU_pipe;
    }

    if(OPT_pipe){
        close(OPT_pipe[0]);
        close(OPT_pipe[1]);
        delete[] OPT_pipe;
    }

    if(BONUS_pipe){
        close(BONUS_pipe[0]);
        close(BONUS_pipe[1]);
        delete[] BONUS_pipe;
    }
};

//A semaphore for each process
Semaphores::Semaphores(){
    PRINT_sem = (sem_t*) mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    FIFO_sem = (sem_t*) mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    FIFO_ready_sem = (sem_t*) mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    LRU_sem = (sem_t*) mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    LRU_ready_sem = (sem_t*) mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    LIFO_sem = (sem_t*) mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    LIFO_ready_sem = (sem_t*) mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    MRU_sem = (sem_t*) mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    MRU_ready_sem = (sem_t*) mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    LFU_sem = (sem_t*) mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    LFU_ready_sem = (sem_t*) mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    OPT_sem = (sem_t*) mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    OPT_ready_sem = (sem_t*) mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    BONUS_sem = (sem_t*) mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    BONUS_ready_sem = (sem_t*) mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    
    sem_init(PRINT_sem, 1, 1);
    sem_init(FIFO_sem, 1, 0);
    sem_init(FIFO_ready_sem, 1, 0);
    sem_init(LRU_sem, 1, 0);
    sem_init(LRU_ready_sem, 1, 0);
    sem_init(LIFO_sem, 1, 0);
    sem_init(LIFO_ready_sem, 1, 0);
    sem_init(MRU_sem, 1, 0);
    sem_init(MRU_ready_sem, 1, 0);
    sem_init(LFU_sem, 1, 0);
    sem_init(LFU_ready_sem, 1, 0);
    sem_init(OPT_sem, 1, 0);
    sem_init(OPT_ready_sem, 1, 0);
    sem_init(BONUS_sem, 1, 0);
    sem_init(BONUS_ready_sem, 1, 0);
};

Semaphores::~Semaphores(){
    munmap(PRINT_sem, sizeof(sem_t));
    munmap(FIFO_sem, sizeof(sem_t));
    munmap(FIFO_ready_sem, sizeof(sem_t));
    munmap(LRU_sem, sizeof(sem_t));
    munmap(LRU_ready_sem, sizeof(sem_t));
    munmap(LIFO_sem, sizeof(sem_t));
    munmap(LIFO_ready_sem, sizeof(sem_t));
    munmap(MRU_sem, sizeof(sem_t));
    munmap(MRU_ready_sem, sizeof(sem_t));
    munmap(LFU_sem, sizeof(sem_t));
    munmap(LFU_ready_sem, sizeof(sem_t));
    munmap(OPT_sem, sizeof(sem_t));
    munmap(OPT_ready_sem, sizeof(sem_t));
    munmap(BONUS_sem, sizeof(sem_t));
    munmap(BONUS_ready_sem, sizeof(sem_t));
};

/////////////////////
//END STRUCTS//
/////////////////////

///////////////////
//LEGACY CODE//
///////////////////

//Builds a helper std::unordered_map<std::string, bool> a_before_n to check future page order of occurrences of 'n' and 'a'
//Unneeded if we track which pages we've already seen then we can simply compare based on first occurrence of each
std::unordered_map<std::string, bool> create_a_before_n(const std::vector<std::string> &future_tokens, const std::unordered_set<std::string> &memory){
    std::unordered_map<std::string, bool> a_before_n;

    for(const std::string &memory_page : memory){
        for(const std::string &token : future_tokens){
            std::string current_page = read_page_in_token(token);
            
            if(memory_page == current_page){
                if(a_before_n.count(memory_page)){
                    //Seen it, skip it
                    continue;
                } 
                
                if(token[token.size() - 1] == 'a'){
                    //Haven't seen it, first sighting is 'a', can discard
                    a_before_n[memory_page] = true;
                } else{
                    //Haven't seen it, first sighting is 'n', cannot discard
                    a_before_n[memory_page] = false;
                }
            }
        }
    }

    return a_before_n;
}

///////////////////
//END LEGACY CODE//
///////////////////

//Read in our input args
void read_args(char* argv[], int &memory_size, int &look_ahead){
    memory_size = std::stoi(argv[1]);
    look_ahead = std::stoi(argv[2]);
}

//Read in a token from input
std::string get_token(){
    std::string token;

    std::cin >> token;

    return token;
}

//Builds vector of tokens to pass to our sorting algos
std::vector<std::string> build_token_list(){
    std::string token = get_token();
    std::vector<std::string> token_list;

    while(token != ""){
        token_list.push_back(token);
        token = get_token();
    }

    return token_list;
}

//Splits token over ':' and returns true of false for bandwidth
bool split_token(std::string &token){
    int colon_location = token.find(':');
    
    if(colon_location == std::string::npos){
        return true;
    }

    char flag = token[colon_location + 1];
    token = token.substr(0, colon_location);

    return flag == 'a';
}

//Splits token over ':' and returns true of false for bandwidth
std::string read_page_in_token(const std::string &token){
    int colon_location = token.find(':');
    
    if(colon_location == std::string::npos){
        return token;
    }

    return token.substr(0, colon_location);
}

//////////////////////////////////////
//BEGIN PAGE REPLACEMENT FUNCTIONS//
//////////////////////////////////////

void page_replacement(const int &look_ahead, const std::string &replacement_type, const int &memory_size, sem_t* reading_sem, sem_t* ready_sem, int* pipe, sem_t* print_sem){
    Main_Memory main_memory(replacement_type, memory_size);
    char buffer[MESSAGE_SIZE];
    std::queue<std::string> offline_set;
    std::string message;

    if(main_memory.replacement_type == "OPT" || main_memory.replacement_type == "BONUS"){
        for(int token_intake = 0; token_intake < look_ahead; token_intake++){
            sem_wait(reading_sem);
            read(pipe[0], buffer, MESSAGE_SIZE);
            sem_post(ready_sem);

            message = buffer;
            main_memory.OPT_future_tokens.push_back(message);
            //add_to_future_tokens(look_ahead, main_memory.OPT_future_tokens, message);
        }
    }

    sem_wait(reading_sem);
    read(pipe[0], buffer, MESSAGE_SIZE);
    sem_post(ready_sem);

    message = buffer;

    //if(main_memory.replacement_type == "MRU"){
    //    std::cout << "Received " << message << std::endl;
    //}

    if(main_memory.replacement_type == "OPT" || main_memory.replacement_type == "BONUS"){
        add_to_future_tokens(main_memory.OPT_future_tokens, message);
    }

    take_token(message, main_memory, offline_set);
    //Call print inside take_token
    //print_set(FIFO_set);

    //std::cout << "We read: " << message << " from pipe." << std::endl;

    while(message != ""){
        //std::cout << "FIFO waiting for read sem.\n";
        sem_wait(reading_sem);
        //std::cout << "FIFO Read sem posted, reading from pipe.\n";
        read(pipe[0], buffer, MESSAGE_SIZE);
        //std::cout << "FIFO Read from pipe.\n";
        sem_post(ready_sem);
        //std::cout << "FIFO Ready to read sem posted.\n";

        message = buffer;

        //if(main_memory.replacement_type == "MRU"){
        //    std::cout << "Received " << message << std::endl;
        //}

        //std::cout << "We read: " << message << " from pipe." << std::endl;
        if(message != ""){
            if(main_memory.replacement_type == "OPT" || main_memory.replacement_type == "BONUS"){
                add_to_future_tokens(main_memory.OPT_future_tokens, message);
            }
            take_token(message, main_memory, offline_set);
            //Call print inside take_token
            //print_set(FIFO_set);
        }
    }

    if(main_memory.replacement_type == "OPT" || main_memory.replacement_type == "BONUS"){
        empty_future_tokens(main_memory, offline_set);
    }

    print_memory(main_memory, print_sem, offline_set);

    exit(0);
}

void take_token(std::string &token, Main_Memory &memory, std::queue<std::string> &offline_set){
    //std::cout << "\n\nTOKEN TAKEN FOR " << memory.replacement_type << std::endl;
    //print_memory(memory);
    //std::cout << "\n" << std::endl;
    increment_page_references(read_page_in_token(token), memory.page_references);
    
    //Do we have service?
    //If yes:
    if(split_token(token)){
        //std::cout << "WE HAVE SERVICE!\n";
        //Do we have any pending pages?
        //If yes:
        if(offline_set.size() > 0){
            //std::cout << "WE HAVE STORED OFFLINE PAGE REQUESTS!\n";
            int set_size = offline_set.size();
            for(int page = 0; page < set_size; page++){
                //std::cout << "Checking " << offline_set.front();
                add_to_memory(offline_set.front(), memory);
                offline_set.pop();
                //print_memory(memory);
            }
        }

        //std::cout << "Trying to add to memory.\n";
        add_to_memory(token, memory);
        //add_to_memory(token[0], memory);

        return;
    }
    
    //If no:
    //Is the page in memory already?
    //std::cout << "NO SERVICE!!!\n";
    if(memory.frames.count(token)){
    //if(memory.frames.count(token[0])){
        //If yes:
        //Skip it, we can access it
        //std::cout << "But that's ok, we have this stored already.\n";
        //REORDER FOR LRU HERE
        if(memory.replacement_type == "LRU"){
            LRU_sort(token, memory.LRU_set);
        } else if(memory.replacement_type == "MRU"){
            MRU_sort(token, memory.MRU_set);
        } else if(memory.replacement_type == "LFU"){
            increment_page_frequency(token, memory.LFU_page_frequency);
        }

        return;
    }

    //If no:
    //Add page to offline_set
    //std::cout << "Adding to offline set!\nDelayed replacement incremented for " << memory.replacement_type << std::endl;
    offline_set.push(token);
    memory.delayed_replacements++;
}

void add_to_memory(const std::string &page, Main_Memory &memory){
    //Is page in memory?
    //If yes:
    if (memory.frames.count(page)){
        //increment_page_references(page, memory.page_references);
        //Discard, page may be accessed
        //std::cout << "Page " << page << " in memory, can be accessed.\n";
        //If not at end of LRU_set reorder
        if(memory.replacement_type == "LRU" && page != memory.LRU_set[memory.LRU_set.size() - 1]){
            //std::cout << "Set before reorder:\n";
            //print_memory(memory);
            LRU_sort(page, memory.LRU_set);
            //std::cout << "Set after reorder:\n";
            //print_memory(memory);
        } else if(memory.replacement_type == "MRU" && page != memory.MRU_set[0]){
            //std::cout << "Set before reorder:\n";
            //print_memory(memory);
            MRU_sort(page, memory.MRU_set);
            //std::cout << "Set after reorder:\n";
            //print_memory(memory);
        } else if(memory.replacement_type == "LFU"){
            increment_page_frequency(page, memory.LFU_page_frequency);
        } else if(memory.replacement_type == "BONUS"){
            //Check to see if we can preempt misses with our current bandwidth access
            replace_BONUS_OPT_on_hit(page, memory);
        }
        
        //Can return inside the if statements instead...
        return;
    }

    //Is memory full?
    //If yes:
    if(memory.frames.size() == memory.memory_size){
        //std::cout << "MISS FOR " << memory.replacement_type << std::endl;
        memory.number_of_misses++;
        //std::cout << "Memory full!\n";
        //std::cout << "PAGE REPLACEMENT FOR " << memory.replacement_type << std::endl;
        memory.page_replacements++;
        //increment_page_references(page, memory.page_references);

        //Find first entry added to FIFO_set
        if(memory.replacement_type == "LRU"){
            //Discard entry
            memory.frames.erase(memory.LRU_set[0]);
            //Discard LRU and add new entry into LRU_set
            add_page_LRU(page, memory.LRU_set);
        } else if(memory.replacement_type == "FIFO"){
            //Discard entry
            memory.frames.erase(memory.FIFO_set.front());
            memory.FIFO_set.pop();
            //Add new entry to memory
            memory.frames.insert(page);
            //Add new entry FIFO_set
            memory.FIFO_set.push(page);
        } else if(memory.replacement_type == "LIFO"){
            //Discard entry
            memory.frames.erase(memory.LIFO_set.top());
            memory.LIFO_set.pop();
            //Add new entry to memory
            memory.frames.insert(page);
            //Add new entry FIFO_set
            memory.LIFO_set.push(page);
        } else if(memory.replacement_type == "MRU"){
            //Discard entry
            memory.frames.erase(memory.MRU_set[0]);
            //Discard MRU and add new entry into MRU_set
            add_page_MRU((memory.memory_size == memory.MRU_set.size()), page, memory.MRU_set);
        } else if(memory.replacement_type == "LFU"){
            //Discard old entry and update LFU frequency
            replace_LFU(page, memory.LFU_page_frequency, memory.frames);
        } else if(memory.replacement_type == "OPT"){
            //Discard old entry
            replace_OPT(page, memory.OPT_future_tokens, memory.frames);
        } else if(memory.replacement_type == "BONUS"){
            //Discard old entry
            replace_BONUS_OPT(page, memory.OPT_future_tokens, memory.frames);
        }
        
        //Add new entry to memory
        //Can move this into each if statement to guarantee return upon completion
        memory.frames.insert(page);
        

        return;
    }
    
    //If no:
    //Add to memory
    //std::cout << "WE GOT ROOM! No need to PR!\nMISS FOR " << memory.replacement_type << std::endl;
    memory.frames.insert(page);
    memory.number_of_misses++;
    //increment_page_references(page, memory.page_references);
    
    //Add to a set
    if(memory.replacement_type == "LRU"){
        memory.LRU_set.push_back(page);
    } else if(memory.replacement_type == "FIFO"){
        memory.FIFO_set.push(page);
    } else if(memory.replacement_type == "LIFO"){
        memory.LIFO_set.push(page);
    } else if(memory.replacement_type == "MRU"){
        //Could just say 'false'
        add_page_MRU((memory.memory_size == memory.MRU_set.size()), page, memory.MRU_set);
    } else if(memory.replacement_type == "LFU"){
        increment_page_frequency(page, memory.LFU_page_frequency);
    }
}

void increment_page_references(const std::string &page, std::unordered_map<std::string, int> &page_references){
    page_references[page]++;
}

std::string print_page_references(const std::unordered_map<std::string, int> &page_references){
    std::string print_statement = "\nPage References:\n";
    int reference_count = 0;

    for(const std::pair<std::string, int> &entry : page_references){
        reference_count += entry.second;
        print_statement += entry.first;
        print_statement += ": ";
        print_statement += std::to_string(entry.second);
        print_statement += "\n";
    }
    print_statement += "Total References: ";
    print_statement += std::to_string(reference_count);
    print_statement += "\n";

    return print_statement;
}

//Could modify to add strings and create a final statement if they get all mixed up
void print_memory(const Main_Memory &memory, sem_t* print_sem, const std::queue<std::string> &offline_set){
    sem_wait(print_sem);
    std::string print_statement = "Current ";
    print_statement += memory.replacement_type;
    print_statement += " Set:\n[";

    if(memory.replacement_type == "FIFO"){
        print_statement += print_set(memory.FIFO_set);
    } else if(memory.replacement_type == "LRU" || memory.replacement_type == "MRU"){
        if(memory.replacement_type == "MRU"){
            print_statement += print_vector(memory.MRU_set);
        } else{
            print_statement += print_vector(memory.LRU_set);
        }
    } else if(memory.replacement_type == "LIFO"){
        print_statement += print_stack(memory.LIFO_set);
    } else if(memory.replacement_type == "LFU" || memory.replacement_type == "OPT" || memory.replacement_type == "BONUS"){
        print_statement += print_unordered_set(memory.frames);
    }

    //std::cout << "NUMBER OF MISSES: " << memory.number_of_misses << std::endl;
    print_statement += "\nNumber of Replacements: ";
    print_statement += std::to_string(memory.page_replacements);
    print_statement += "\nNumber of Misses: ";
    print_statement += std::to_string(memory.number_of_misses);
    print_statement += "\nNumber of Delayed Replacements: ";
    print_statement += std::to_string(memory.delayed_replacements);
    print_statement += print_page_references(memory.page_references);
    print_statement += "\n";

    if(offline_set.size() > 0){
        print_statement += "WARNING: NO BANDWIDTH ACCESS TO DOWNLOAD LAST ";
        print_statement += std::to_string(offline_set.size());
        print_statement += " PAGE/S\n";
    }

    std::cout << print_statement << std::endl;
    sem_post(print_sem);
}

////////////////////
//FIFO HELPERS//
////////////////////

//Prints out the requested current set (stored as queue)
std::string print_set(const std::queue<std::string> &set){
    std::string print_statement;
    std::queue<std::string> printed_set = set;
    //std::cout << "Current FIFO Set:\n[";
    
    for(int page = 0; page < set.size(); page++){
        //std::cout << printed_set.front();
        print_statement += printed_set.front();
        if(page < set.size() - 1){
            //std::cout << ", ";
            print_statement += ", ";
        }
        printed_set.pop();
    }
    //std::cout << "]\n" << std::endl;
    print_statement += "]";
    //std::cout << print_statement << std::endl;
    return print_statement;
}

////////////////////
//END FIFO HELPERS//
////////////////////



///////////////////
//LRU HELPERS//
///////////////////

//Sorts the LRU_set
void LRU_sort(const std::string &page, std::vector<std::string> &LRU_set){
    std::vector<std::string> sorted_set;

    for(const std::string &stored_page : LRU_set){
        if(stored_page != page){
            sorted_set.push_back(stored_page);
        }
    }

    sorted_set.push_back(page);
    LRU_set = sorted_set;
}

//Removes the LRU page and adds the new one to the set
void add_page_LRU(const std::string &page, std::vector<std::string> &LRU_set){
    std::vector<std::string> new_set;

    for(int stored_page = 1; stored_page < LRU_set.size(); stored_page++){
        new_set.push_back(LRU_set[stored_page]);
    }

    new_set.push_back(page);
    LRU_set = new_set;
}

//Prints out the requested current set (stored as vector)
std::string print_vector(const std::vector<std::string> &set){
    std::string print_statement;
    //std::cout << "Current " << replacement_type << " Set:\n[";
    
    for(int page = 0; page < set.size(); page++){
        //std::cout << set[page];
        print_statement += set[page];
        if(page < set.size() - 1){
            //std::cout << ", ";
            print_statement += ", ";
        }
    }
    //std::cout << "]\n" << std::endl;
    print_statement += "]";
    //std::cout << print_statement << std::endl;
    return print_statement;
}

///////////////////
//END LRU HELPERS//
///////////////////

////////////////////
//LIFO HELPERS//
////////////////////

//Prints out the requested current set (stored as stack)
std::string print_stack(const std::stack<std::string> &set){
    std::string print_statement;
    std::stack<std::string> printed_set = set;
    //std::cout << "Current LIFO Set:\n[";
    
    for(int page = 0; page < set.size(); page++){
        //std::cout << printed_set.top();
        print_statement += printed_set.top();
        if(page < set.size() - 1){
            //std::cout << ", ";
            print_statement += ", ";
        }
        printed_set.pop();
    }
    //std::cout << "]\n" << std::endl;
    print_statement += "]";
    //std::cout << print_statement << std::endl;
    return print_statement;
}

////////////////////
//END LIFO HELPERS//
////////////////////

///////////////////
//MRU HELPERS//
///////////////////

//Sorts the MRU_set
//Furthest 'left' page is the first to be removed
void MRU_sort(const std::string &page, std::vector<std::string> &MRU_set){
    std::vector<std::string> sorted_set;
    sorted_set.push_back(page);

    for(const std::string &stored_page : MRU_set){
        if(stored_page != page){
            sorted_set.push_back(stored_page);
        }
    }

    MRU_set = sorted_set;
}

//Removes the MRU page and adds the new one to the set
//If MRU_set full, index == 1 otherwise we add the whole vector
void add_page_MRU(bool index, const std::string &page, std::vector<std::string> &MRU_set){
    std::vector<std::string> new_set;
    new_set.push_back(page);

    for(int stored_page = index; stored_page < MRU_set.size(); stored_page++){
        new_set.push_back(MRU_set[stored_page]);
    }
    
    MRU_set = new_set;
}

///////////////////
//END MRU HELPERS//
///////////////////

///////////////////
//LFU HELPERS//
///////////////////

//Add to the count for a page frequency
//Don't need a function for this....
void increment_page_frequency(const std::string &page, std::unordered_map<std::string, int> &page_frequency){
    page_frequency[page]++;
}

//Find the LFU currently in memory
std::string find_LFU(const std::unordered_map<std::string, int> &page_frequency, const std::unordered_set<std::string> &memory){
    int current_min;
    std::string current_LFU;

    for(const auto &entry : page_frequency){
        if(memory.count(entry.first)){
            current_min = entry.second;
            current_LFU = entry.first;
            break;
        }
    }
    
    for(const std::string &page : memory){
        if(page == current_LFU){
            continue;
        }
        
        int compare_frequency = page_frequency.at(page);
        if(current_min > compare_frequency){
            current_min = compare_frequency;
            current_LFU = page;
        }
    }

    return current_LFU;
}

//Perform LFU replacement
//Insertion performed in code body, changed to keep consistent across all replacement types
void replace_LFU(const std::string &page, std::unordered_map<std::string, int> &page_frequency, std::unordered_set<std::string> &memory){
    std::string page_to_remove = find_LFU(page_frequency, memory);
    increment_page_frequency(page, page_frequency);
    memory.erase(page_to_remove);
    //memory.insert(page);
}

//Prints out the requested current set (stored as unordered_set)
std::string print_unordered_set(const std::unordered_set<std::string> &set){
    std::string print_statement;
    int page_count = 0;

    //std::cout << "Current LFU Set:\n[";
    for(const std::string &page : set){
        //std::cout << page;
        print_statement += page;
        if(page_count < set.size() - 1){
            //std::cout << ", ";
            print_statement += ", ";
        }
        page_count++;
    }
    //std::cout << "]\n" << std::endl;
    print_statement += "]";
    //std::cout << print_statement << std::endl;
    return print_statement;
}

///////////////////
//END LFU HELPERS//
///////////////////

///////////////////
//OPT HELPERS//
///////////////////

//Find the OPT currently in memory
std::string find_OPT(const std::vector<std::string> &future_tokens, const std::unordered_set<std::string> &memory){
    std::string furthest_page = ":";
    std::unordered_set<std::string> marked_pages;

    for(const std::string &token : future_tokens){
        std::string current_page = read_page_in_token(token);
        if(memory.count(current_page) && !marked_pages.count(current_page)){
            furthest_page = current_page;
        }
        marked_pages.insert(current_page);
    }

    if(furthest_page == ":"){
        for(const std::string &page : memory){
            furthest_page = page;
            break;
        }
    }

    return furthest_page;
}

//Add token read from pipe into our future_tokens vector
//Replace token with the token at the front of the vector to send to replacement function
void add_to_future_tokens(std::vector<std::string> &future_tokens, std::string &token){
    std::string OPT_current = future_tokens[0];

    std::vector<std::string> updated_tokens;
    for(int location = 1; location < future_tokens.size(); location++){
        updated_tokens.push_back(future_tokens[location]);
    }

    updated_tokens.push_back(token);
    
    future_tokens = updated_tokens;
    token = OPT_current;
}

//Perform OPT replacement
//Insertion performed in code body, changed to keep consistent across all replacement types
void replace_OPT(const std::string &page, const std::vector<std::string> &future_tokens, std::unordered_set<std::string> &memory){
    std::string page_to_remove = find_OPT(future_tokens, memory);
    memory.erase(page_to_remove);
    //memory.insert(page);
}

//Empties our future_token vector applying page replacement as it does
void empty_future_tokens(Main_Memory &main_memory, std::queue<std::string> &offline_set){
    while(main_memory.OPT_future_tokens.size() >= 1){
        std::string message = take_front_token(main_memory.OPT_future_tokens);
        take_token(message, main_memory, offline_set);
    }

    //std::cout << "COMPLETED OPT" << std::endl;
}

//Takes the 'first' token in our futur_tokens vector and returns it as std::string current_token
std::string take_front_token(std::vector<std::string> &future_tokens){
    std::vector<std::string> remaining_tokens;
    std::string current_token = future_tokens[0];
    for(int token_number = 1; token_number < future_tokens.size(); token_number++){
        remaining_tokens.push_back(future_tokens[token_number]);
    }

    future_tokens = remaining_tokens;
    return current_token;
}

///////////////////
//END OPT HELPERS//
///////////////////

/////////////////////
//BONUS HELPERS//
/////////////////////

//Find the BONUS OPT currently in memory on MISS
std::string find_BONUS_OPT(const std::vector<std::string> &future_tokens, const std::unordered_set<std::string> &memory){
    std::string furthest_page = ":";
    char furthest_page_bandwidth = ':';
    std::unordered_set<std::string> marked_pages;

    for(const std::string &token : future_tokens){
        std::string current_page = read_page_in_token(token);
        
        if(memory.count(current_page) && !marked_pages.count(current_page)){
            char current_bandwidth = token[token.size() - 1];
            //Discard furthest 'a' that is NOT preceded by 'n'
            //Only discard 'n' if no 'a' available
            if(furthest_page_bandwidth == 'a' && current_bandwidth != 'a'){
                //Bypass this page
                //It is 'n'
                continue;
                
            } else if(furthest_page_bandwidth == 'a' && current_bandwidth == 'a'){
                //Both are 'a'
                //If current is 'a' before 'n' AND it's first occurrence is further away add it
                furthest_page = current_page;
                continue;

            } else if(furthest_page_bandwidth != 'a' && current_bandwidth == 'a'){
                //Trade our 'n' for an 'a' IF we haven't seen that page before
                furthest_page = current_page;
                furthest_page_bandwidth = current_bandwidth;
                continue;

            } else if(furthest_page_bandwidth != 'a' && current_bandwidth == 'n'){
                //Catch all for our initial statement furthest_page = ":";
                furthest_page = current_page;
                furthest_page_bandwidth = current_bandwidth;
                continue;

            }
        }

        //We've seen this page now
        //We only care what bandwidth is at the first occurrence
        //Don't look at it again
        marked_pages.insert(current_page);
    }

    if(furthest_page == ":"){
        for(const std::string &page : memory){
            //If we HAVE to replace a page, and OPT will not work, pick a random page
            furthest_page = page;
            break;
        }
    }

    return furthest_page;
}

//Perform OPT replacement on MISS
//Insertion performed in code body, changed to keep consistent across all replacement types
void replace_BONUS_OPT(const std::string &page, const std::vector<std::string> &future_tokens, std::unordered_set<std::string> &memory){
    std::string page_to_remove = find_BONUS_OPT(future_tokens, memory);
    memory.erase(page_to_remove);
    //memory.insert(page);
}

//Find n_before_a
//This is the page we want to add to memory
std::string find_n_before_a(const std::vector<std::string> &future_tokens, const std::unordered_set<std::string> &memory){
    std::unordered_set<std::string> a_before_n;
    
    for(const std::string &token : future_tokens){
        std::string current_token = read_page_in_token(token);
        if(memory.count(current_token)){
            //Skip it, it's in memory already
            continue;
        }

        if(token[token.size() - 1] == 'a'){
            //Don't have to worry about this messing up logic flow, as we cannot add a token that we saw an 'n' first since the loop breaks at any token who's first bandwidth id is 'n'
            a_before_n.insert(current_token);
        } else if(!a_before_n.count(current_token)){
            //Don't need to explicitly check 'n' since the previous step checks 'a'
            //First page not in memory AND with no badwidth
            return token;
        }
    }

    return "";
}

//Find unnecessary page in memory
//Finds tokens in memory, but not in future_tokens
std::unordered_set<std::string> find_unneeded_pages(const std::string &avoid_page, const std::vector<std::string> &future_tokens, const std::unordered_set<std::string> &memory){
    std::unordered_set<std::string> unneeded_pages;
    bool add_page;

    for(const std::string &page : memory){
        if(page == avoid_page){
            //Skip this page, we need it
            continue;
        }
        
        add_page = true;
        
        for(const std::string &token : future_tokens){
            std::string current_page = read_page_in_token(token);
            if(page == current_page){
                //Found page in future, do NOT add to unneeded_pages
                add_page = false;
                break;
            }
        }

        if(add_page){
            //If we didn't see it, add it
            unneeded_pages.insert(page);
        }
    }

    return unneeded_pages;
}

//Find safe page to replace if no unneeded pages in memory
//Replace the furthest one out
std::string find_BONUS_OPT_on_hit_(const std::string &avoid_page, const std::vector<std::string> &future_tokens, const std::unordered_set<std::string> &memory){
    std::string furthest_page = "";
    char furthest_page_bandwidth = ':';
    std::unordered_set<std::string> modified_memory = memory;
    //Avoids removing the page we're currently using
    modified_memory.erase(avoid_page);
    std::unordered_set<std::string> unneeded_pages = find_unneeded_pages(avoid_page, future_tokens, memory);
    std::string current_page;

    if(unneeded_pages.size() > 0){
        for(const std::string &page : unneeded_pages){
            //Pick one and send it
            furthest_page = page;
            return furthest_page;
        }
    }
    
    std::unordered_set<std::string> marked_pages;
    
    for(const std::string &token : future_tokens){
        current_page = read_page_in_token(token);
        
        if(modified_memory.count(current_page) && !marked_pages.count(current_page)){
            char current_bandwidth = token[token.size() - 1];
            //Discard furthest 'a' that is NOT preceded by 'n'
            //DO NOT DISCARD 'n'
            if(furthest_page_bandwidth == 'a' && current_bandwidth != 'a'){
                //Bypass this page
                //It is 'n'
                continue;
                
            } else if(furthest_page_bandwidth == 'a' && current_bandwidth == 'a'){
                //Both are 'a'
                //If current is 'a' before 'n' add it
                furthest_page = current_page;
                continue;

            } else if(furthest_page_bandwidth != 'a' && current_bandwidth == 'a'){
                //Trade our ':' for an 'a' IF a_before_n
                furthest_page = current_page;
                continue;

            } else if(furthest_page_bandwidth != 'a' && current_bandwidth == 'n'){
                //Don't want it
                continue;
            }
        }
        marked_pages.insert(current_page);
    }

    //If we still have our default values, we do not have a valid page to replace
    //So we do NOT replace a page
    return furthest_page;
}




//Replace unnecessary page with one needed
void replace_BONUS_OPT_on_hit(const std::string &avoid_page, Main_Memory &main){
    std::string page_to_add = find_n_before_a(main.OPT_future_tokens, main.frames);
    std::string page_to_remove = "";

    if(page_to_add != ""){
        page_to_remove = find_BONUS_OPT_on_hit(avoid_page, main.OPT_future_tokens, main.frames);
    }
    
    if(page_to_remove != ""){
        main.frames.erase(page_to_remove);
        main.frames.insert(page_to_add);
        main.page_replacements++;
    }
}

/////////////////////
//END BONUS HELPERS//
/////////////////////

//////////////////////////////////////
//END PAGE REPLACEMENT FUNCTIONS//
//////////////////////////////////////

///////////////////
//MAIN HELPERS//
///////////////////

void execute_replacement_strategies(int &pid, const int &look_ahead, const int &memory_size, Pipes &message_pipes, Semaphores &replacement_type_sems){
    pid = fork();
    if(pid == 0){
        page_replacement(look_ahead, "FIFO", memory_size, replacement_type_sems.FIFO_sem, replacement_type_sems.FIFO_ready_sem, message_pipes.FIFO_pipe, replacement_type_sems.PRINT_sem);
        exit(0);
    }

    pid = fork();
    if(pid == 0){
        page_replacement(look_ahead, "LRU", memory_size, replacement_type_sems.LRU_sem, replacement_type_sems.LRU_ready_sem, message_pipes.LRU_pipe, replacement_type_sems.PRINT_sem);
        exit(0);
    }

    pid = fork();
    if(pid == 0){
        page_replacement(look_ahead, "LIFO", memory_size, replacement_type_sems.LIFO_sem, replacement_type_sems.LIFO_ready_sem, message_pipes.LIFO_pipe, replacement_type_sems.PRINT_sem);
        exit(0);
    }

    pid = fork();
    if(pid == 0){
        page_replacement(look_ahead, "MRU", memory_size, replacement_type_sems.MRU_sem, replacement_type_sems.MRU_ready_sem, message_pipes.MRU_pipe, replacement_type_sems.PRINT_sem);
        exit(0);
    }

    pid = fork();
    if(pid == 0){
        page_replacement(look_ahead, "LFU", memory_size, replacement_type_sems.LFU_sem, replacement_type_sems.LFU_ready_sem, message_pipes.LFU_pipe, replacement_type_sems.PRINT_sem);
        exit(0);
    }

    pid = fork();
    if(pid == 0){
        page_replacement(look_ahead, "OPT", memory_size, replacement_type_sems.OPT_sem, replacement_type_sems.OPT_ready_sem, message_pipes.OPT_pipe, replacement_type_sems.PRINT_sem);
        exit(0);
    }

    pid = fork();
    if(pid == 0){
        page_replacement(look_ahead, "BONUS", memory_size, replacement_type_sems.BONUS_sem, replacement_type_sems.BONUS_ready_sem, message_pipes.BONUS_pipe, replacement_type_sems.PRINT_sem);
        exit(0);
    }
}

void write_to_pipes(Pipes &message_pipes, std::string &token){
    write(message_pipes.FIFO_pipe[1], token.c_str(), token.size() + 1);
    write(message_pipes.LRU_pipe[1], token.c_str(), token.size() + 1);
    write(message_pipes.LIFO_pipe[1], token.c_str(), token.size() + 1);
    write(message_pipes.MRU_pipe[1], token.c_str(), token.size() + 1);
    write(message_pipes.LFU_pipe[1], token.c_str(), token.size() + 1);
    write(message_pipes.OPT_pipe[1], token.c_str(), token.size() + 1);
    write(message_pipes.BONUS_pipe[1], token.c_str(), token.size() + 1);
}

void post_read_semaphores(Semaphores &replacement_type_sems){
    sem_post(replacement_type_sems.FIFO_sem);
    sem_post(replacement_type_sems.LRU_sem);
    sem_post(replacement_type_sems.LIFO_sem);
    sem_post(replacement_type_sems.MRU_sem);
    sem_post(replacement_type_sems.LFU_sem);
    sem_post(replacement_type_sems.OPT_sem);
    sem_post(replacement_type_sems.BONUS_sem);
}

void wait_ready_semaphores(Semaphores &replacement_type_sems){
    sem_wait(replacement_type_sems.FIFO_ready_sem);
    sem_wait(replacement_type_sems.LRU_ready_sem);
    sem_wait(replacement_type_sems.LIFO_ready_sem);
    sem_wait(replacement_type_sems.MRU_ready_sem);
    sem_wait(replacement_type_sems.LFU_ready_sem);
    sem_wait(replacement_type_sems.OPT_ready_sem);
    sem_wait(replacement_type_sems.BONUS_ready_sem);
}

///////////////////
//END MAIN HELPERS//
///////////////////
