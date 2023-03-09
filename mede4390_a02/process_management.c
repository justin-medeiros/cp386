#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <string.h>
#define READ_END 0
#define WRITE_END 1

// Constants
const int SIZE = 4096; 
const int BUFFER_SIZE = 255;
const int numItems = 5;


int writeShm(char *name, char *textfile){
    size_t line;
    // Shared memory descriptor
    int shm_fd;
    // Pointer to shared memory object 
    char *ptr;

    // Create shared memory as if it was a file
    shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        printf("Shared memory failed\n");
        exit(1);
    }

    // Configure size of shared memory segment
    ftruncate(shm_fd, SIZE);

    // Map the shared memory segment
    ptr = mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) {
        printf("Map failed\n");
        shm_unlink(name);
        exit(1);
    }

    int fd = open(textfile, O_RDONLY);
    if(fd == -1){
        printf("Error opening the file.");
        exit(1);
    }
    char *buffer = malloc(BUFFER_SIZE);
    while((line = read(fd, buffer, sizeof buffer)) > 0){
        if(line == -1){
            printf("Error reading from input file.\n");
            exit(1);
        } else{
            buffer[line] = '\0';
            ptr += sprintf(ptr, "%s", buffer);
        }
    }
    free(buffer);
    close(fd);
    return 0;
}

char **readShm(char *name){
    char **commandsArray = (char **) malloc(numItems * sizeof(char *));
    // Shared memory descriptor
    int shm_fd;
    // Pointer to shared memory object 
    char *ptr;
    // Open shared memory object and store in shared memory file descriptor
    shm_fd = shm_open(name, O_RDONLY, 0666);
    if (shm_fd == -1) {
        printf("Shared memory failed\n");
        exit(1);
    }

    ptr = mmap(0, SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
    if(ptr == MAP_FAILED){
        printf("Map failed.\n");
        exit(1);
    }

    // Use strtok to separate the string by newline characters '\n'
    char *copy = strdup(ptr);
    char *token = strtok(copy, "\n");
    int i = 0;
    while(token != NULL){
        commandsArray[i] = token;
        token = strtok(NULL, "\n");
        i++;
    }
    shm_unlink(name);
    return commandsArray;
}

int writePipe(int *pipeFd, char **commandsArray){
    close(pipeFd[READ_END]);
    // Instead of execvp outputting to console, it will write to the pipe
    dup2(pipeFd[WRITE_END], 1);
    for(int i = 0; i<sizeof(commandsArray);i++){
        // Seperate the string by whitespace
        char *token = strtok(commandsArray[i], " ");
        int i = 0;
        char *firstCommand = token;
        char **args = (char **)malloc(BUFFER_SIZE);
        // Assign commands to the args array
        while (token != NULL) {
            args[i++] = token;
            token = strtok(NULL, " ");
        }
        args[i] = NULL;
        // Need to put execvp in a child process because it will replace the current process
        int pid = fork();
        if (pid == 0) {
            execvp(firstCommand, args);
            exit(0);
        } else {
            wait(NULL);
        }
        // At the end of writing each message, pass a next line to seperate messages
        write(pipeFd[WRITE_END], "\n", 1);
        free(args);
    }
    close(pipeFd[WRITE_END]);
    return 0;
}

void writeOuput(char* command, char* output)
{
    FILE *fp;
    fp = fopen("output.txt", "a");
	fprintf(fp, "The output of: %s : is\n", command);
	fprintf(fp, ">>>>>>>>>>>>>>>\n%s\n<<<<<<<<<<<<<<<\n", output);	
    fclose(fp);
}

int readPipe(int *pipeFd, char **commandsArray){
    close(pipeFd[WRITE_END]);
    char read_msg[SIZE];
    int bytesRead = read(pipeFd[READ_END], read_msg, SIZE);
    int commandsIndex = 0;
    char *buffer = (char *)malloc(SIZE);
    int buffer_index = 0;

    // Read each character in the pipe and seperate the messages based on next line
    for(int i = 0; i<bytesRead; i++){
        if(read_msg[i] == '\n' && read_msg[i+1] == '\n'){
            // Reset the string for the next message
            buffer[buffer_index] = '\0';
            writeOuput(commandsArray[commandsIndex], buffer);
            buffer_index = 0;
            commandsIndex++;
            i++;
        } else{
            buffer[buffer_index++] = read_msg[i];
        }
    }
    free(buffer);
    close(pipeFd[READ_END]);
    return 0;
}


int main(int argc, char **argv){
    if(argc != 2){
        printf("Invalid amount of arguments\n");
        exit(1);
    }
    // Name of shared memory
    char *name = "shm";
    char **commandsArray = (char **) malloc(numItems * sizeof(char *));
    int pid, pid1;
    
    pid = fork();
    if(pid == -1){
        printf("Fork Failed.\n");
        exit(1);
    }

     // Child process to read shell commands from file
    if(pid == 0){
        writeShm(name, argv[1]);
    } else{ // Parent Process
        wait(NULL);
        // Read from memory block and write to array
        commandsArray = readShm(name);
         // Create pipe
        int pipeFd[2];
        if(pipe(pipeFd) == -1){
            printf("Pipe failed.\n");
            exit(1);
        }

        pid1 = fork();
        if(pid1 == -1){
            printf("Fork Failed.\n");
            exit(1);
        }
        // Child process to run commands and write to pipe
        if(pid1 == 0){ 
            writePipe(pipeFd, commandsArray);
        } else{ // Parent process to read from pipe and write each message in output file
            wait(NULL);
            readPipe(pipeFd, commandsArray); 
        }
        
    }
    return 0;
}