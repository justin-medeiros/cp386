#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char *argv[]){
    if(argc != 3){
        printf("Insufficient parameters passed.\n");
        exit(1);
    } else{
        // Open the file from the first argument passed in the terminal if it exists, else send an error message
        size_t inputFile = open(argv[1], O_RDONLY, 0555);

        if(inputFile != -1){
            // Create a file from the second argument passed in the terminal, else send an error message
            size_t outputFile = open(argv[2], O_RDWR | O_CREAT | O_EXCL , 0750); 
            if(outputFile == -1){
                // Remove output file if already exists and create a new one
                unlink(argv[2]);
                outputFile = open(argv[2], O_RDWR | O_CREAT | O_EXCL , 0750); 
            } 
            // Go through each line in the file and write each line to the output file
            size_t line;
            int *buffer = malloc(255);
            int writeResult;
            while((line = read(inputFile, buffer, sizeof buffer)) > 0){
                if(line == -1){
                    printf("Error reading from input file.\n");
                    exit(1);
                } else{
                    writeResult = write(outputFile, buffer, line);
                    if(writeResult == -1){
                        printf("Error writing to output file.\n");
                        exit(1);
                    }
                }
            }
            if(writeResult != -1){
                printf("The contents of %s have been successfully copied into the %s file.\n", argv[1], argv[2]);
            }
            free(buffer);
            close(inputFile);
            close(outputFile);
        } else{
            printf("Input file does not exist.\n");
            exit(1);
        }
    }
    return 0;
}