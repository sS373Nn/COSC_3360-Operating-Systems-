#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(){
    int pipe_file_desc[2];
    int process_id;
    char buffer[20];

    if (pipe(pipe_file_desc) == -1){
        perror("error creating pipe");
        exit(0);
    }

    process_id = fork();
    if (process_id == -1){
        perror("error creating process using fork");
        exit(0);
    }

    if (process_id == 0){
        close(pipe_file_desc[1]);
        read(pipe_file_desc[0], buffer, 20);
        printf("%s\n", buffer);
        close(pipe_file_desc[0]);
    }

    else{
        close(pipe_file_desc[0]);
        write(pipe_file_desc[1], "Howdy from Houston!!", 20);
        close(pipe_file_desc[1]);
    }

    return 0;
}
