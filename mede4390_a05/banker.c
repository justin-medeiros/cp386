#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define BUFFER_SIZE 255
int main(int argc, char ** argv){
    int fd = open("sample_in_banker.txt", O_RDONLY);
    if(fd == -1){
        printf("File could not be opened or does not exist.\n");
        return 1;
    }

    int maximumResources[5][argc -1];
    int r = 0;
    int c = 0;
    char ch;
    char buffer[BUFFER_SIZE]; // Used for storing each number in file
    int bufferIndex = 0;

    // Read every character in the file
    while(read(fd, &ch, sizeof(char)) > 0){
        // Get the grade from the file and store in the buffer
        if(ch != ',' && ch != '\n'){
            buffer[bufferIndex++] = ch;
        } else{
            // When there is a number in the line store in buffer
            if(bufferIndex > 0){
                buffer[bufferIndex] = '\0';
                maximumResources[r][c] = atoi(buffer);
                c++;
                bufferIndex = 0;
            }
            // When we encounter a new line, want to set column back to 0 and move down a row
            if(ch == '\n'){
                r++;
                c = 0;
            }
        }
   }

   // Print the 2D matrix
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < (argc-1); j++) {
            printf("%d ", maximumResources[i][j]);
        }
        printf("\n");
    }

    close(fd);




    return 0;
}