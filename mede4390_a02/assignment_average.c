#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>

// Global variable so all processes can use
int grades[10][6];

int teacherProcess(int fd){
    const int BUFFER_SIZE = 255;
    int r = 0;
    int c = 0;
    char ch;
    char buffer[BUFFER_SIZE]; // Used for storing each string
    int bufferIndex = 0;

    // Read every character in the file
    while(read(fd, &ch, sizeof(char)) > 0){
        // Get the grade from the file and store in the buffer
        if(ch != ' ' && ch != '\n'){
            buffer[bufferIndex++] = ch;
        } else{
            // When there is a grade in the buffer from the file, store it in the grades matrix
            if(bufferIndex > 0){
                buffer[bufferIndex] = '\0';
                grades[r][c] = atoi(buffer);
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
   // To handle the last number in the file 
   if(bufferIndex > 0){
        buffer[bufferIndex] = '\0';
        grades[r][c] = atoi(buffer);
   }
    return 0;
}

int calculateGrades(int startChapter, int endChapter){
    int rows = sizeof(grades) / sizeof(grades[0]);
    for(int i = startChapter; i <= endChapter; i++){ 
        // Creating TA Process to calculate average for every assignment
        int pid = fork();
        if(pid == 0){
            float totalGrades = 0;
            for(int r = 0; r < rows; r++){
                totalGrades += grades[r][i];
            }
            float averageGrades = totalGrades/rows;
            printf("Assignment %d - Average = %f\n", i+1, averageGrades);
            exit(0);
        } 
    }
    for(int i = startChapter; i <= endChapter; i++){
        wait(NULL);
    }
    return 0;
}


int main(int argc, char ** argv){
    if(argc != 2){
        printf("Error. Please include a file.\n");
        return 1;
    } else{
        int fd = open(argv[1], O_RDONLY);
        if(fd == -1){
            printf("File could not be opened or does not exist.\n");
            return 1;
        }
        teacherProcess(fd);

        for(int i = 0; i<3; i++){
            int pid = fork();
            // Creating GTA Process for every 2 chapters
            if(pid == 0){ 
                calculateGrades(i*2, (i*2)+1);
                exit(0);
            } else{
                wait(NULL);
            }
        }
    }
    return 0;
}
