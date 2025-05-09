#include "functions.hpp"

////////
////////

int main(int argc, char* argv[]){

    //print_command_line_args(argc, argv);

    if (argc < 2){
        std::cout << "Not enough args passed!\n";
        return 0;
    }    

    int output_pipe = std::stoi(argv[1]); //Change to int
    char final_output_buffer[100] = {};
    std::vector<std::string> final_words_list;

    read(output_pipe, final_output_buffer, 100);
    std::string final_word_string(final_output_buffer);
     
    close(output_pipe);

    vectorize(final_word_string, final_words_list);
    std::sort(final_words_list.begin(), final_words_list.end());
    final_words_list = combine_like_words(final_words_list);
    clean_words_vector(final_words_list);
    
    std::cout << "\nFinal Output:\n";
    std::string final_output = format_final_output(final_words_list);
    std::cout << final_output << std::endl;

    return 0;
}

////////
////////
