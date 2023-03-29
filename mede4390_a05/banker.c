#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 255
#define NUM_THREADS 5
#define NUM_RESOURCES 4

// Global variables
int maximumResources[NUM_THREADS][NUM_RESOURCES];
int availableResources[NUM_RESOURCES];
int needResources[NUM_THREADS][NUM_RESOURCES];
int allocationResources[NUM_THREADS][NUM_RESOURCES];

int safetyAlg(int needVector[][NUM_RESOURCES], int work[], int row){
    // Initialize safety values
    int finish[NUM_RESOURCES] = {0, 0, 0, 0};

    for(int i = 0; i < NUM_RESOURCES; i++){
        finish[i] = 0;
        // printf("NEED: %d\n", needVector[row][i]);
        // printf("WORK: %d\n", work[i]);
        if(needVector[row][i] <= work[i]){
            finish[i] = 1;
        } else{
            break;
        }
    }

    for(int j = 0; j < NUM_RESOURCES; j++){
        if(finish[j] == 0){
            return 0;
        }
    }
    return 1;
}

char *requestResources(int request[]){
     // Copy contents of original array to these temporary values
    int tempAvailableResources[NUM_RESOURCES];
    int tempNeedResources[NUM_THREADS][NUM_RESOURCES];
    int tempAllocationResources[NUM_THREADS][NUM_RESOURCES];
    int tempMaxResources[NUM_THREADS][NUM_RESOURCES];
    memcpy(tempAvailableResources, availableResources, sizeof(availableResources));
    memcpy(tempNeedResources, needResources, sizeof(needResources));
    memcpy(tempAllocationResources, allocationResources, sizeof(allocationResources));
    memcpy(tempMaxResources, maximumResources, sizeof(maximumResources));

    int curRow = request[0];
    
    for(int j = 1; j < NUM_RESOURCES+1; j++) {
        int curNum = request[j];
        // Request algorithm and cannot have current num to be allocated if greater than the maximum resource
        if(curNum <= tempNeedResources[curRow][j-1]  && curNum <= tempAvailableResources[j-1] && curNum <= tempMaxResources[curRow][j-1]){
            tempAvailableResources[j-1] -= curNum;
            tempAllocationResources[curRow][j-1] += curNum;
            tempNeedResources[curRow][j-1] = tempMaxResources[curRow][j-1] - tempAllocationResources[curRow][j-1];
        } else{
            return "Resources cannot be allocated";
        }
    }

    // Check safety algorithm if we can actually allocate those resources
    if(safetyAlg(needResources, availableResources, curRow) == 1){
        memcpy(availableResources, tempAvailableResources, sizeof(tempAvailableResources));
        memcpy(needResources, tempNeedResources, sizeof(tempNeedResources));
        memcpy(allocationResources, tempAllocationResources, sizeof(tempAllocationResources));
        memcpy(maximumResources, tempMaxResources, sizeof(tempMaxResources));
        return "State is safe, and request is satisfied";
    } else{
        return "State is not safe, and request is denied";
    }
}

char *releaseResources(int request[]){
    int curRow = request[0];

    for(int j = 1; j < NUM_RESOURCES+1; j++) {
        // Do not release resources if any of the resources requested is greater than allocated resources
       if(request[j] > allocationResources[curRow][j-1]){
            return "Resources cannot be released as they are not in use";
       }
    }

    for(int j = 1; j < NUM_RESOURCES+1; j++) {
        // Release resources
        allocationResources[curRow][j-1] -= request[j];
        availableResources[j-1] += request[j];
        needResources[curRow][j-1] += request[j];
    }
    return "The resources have been released successfully";
}
void printAvailableResources(){
    for(int i = 0; i< NUM_RESOURCES; i++){
        if(i == NUM_RESOURCES-1){
            printf("%d\n", availableResources[i]);
        } else{
            printf("%d ", availableResources[i]);
        }
        
    }
}

void printMatrix(int matrix[][NUM_RESOURCES]){
    for (int i = 0; i < NUM_THREADS; i++) {
        for (int j = 0; j < (NUM_RESOURCES); j++) {
            if(j == NUM_RESOURCES-1){
                printf("%d\n", matrix[i][j]);
            } else{
                printf("%d ", matrix[i][j]);
            }
            
        }
    }
}

int main(int argc, char ** argv){
    int fd = open("sample_in_banker.txt", O_RDONLY);
    if(fd == -1){
        printf("File could not be opened or does not exist.\n");
        return 1;
    }

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
                allocationResources[r][c] = 0;
                needResources[r][c] = maximumResources[r][c] - allocationResources[r][c];
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

   // Check if there is a number left in the buffer
    if(bufferIndex > 0){
        buffer[bufferIndex] = '\0';
        maximumResources[r][c] = atoi(buffer);
        allocationResources[r][c] = 0;
    }

    // Output
    printf("Number of Customers: %ld\n", sizeof(maximumResources)/sizeof(maximumResources[0]));
    printf("Currently Available resources: ");
    // Store available resources
    for(int j = 1; j<argc; j++){
        availableResources[j-1] = atoi(argv[j]);
    }
    // Print available resources
    printAvailableResources(availableResources);

    // Print the maximumn resources matrix
    printMatrix(maximumResources);

    // Close the file as no longer needed
    close(fd);

    // Loop for user input
    char line[BUFFER_SIZE];
    char command[2];

    do {
        printf("Enter Command: ");
        fgets(line, BUFFER_SIZE, stdin);
        
        // Store the command entered by the user in a variable
        char* token = strtok(line, " ");
        strcpy(command, token);
        // An array to store the thread id and resources
        int request[NUM_RESOURCES+1]; 
        int i = 0;
        while(token != NULL && i < 10) {
            if(i != 0){
                request[i-1] = atoi(token);
            } 
            token = strtok(NULL, " ");
            i++;
        }
        // Prevent memory leaks
        free(token);
     
        if(strstr(command, "Status") != NULL){
            printf("Available Resources:\n");
            printAvailableResources();
            printf("Maximum Resources:\n");
            printMatrix(maximumResources);
            printf("Allocated Resources:\n");
            printMatrix(allocationResources);
            printf("Need Resources:\n");
            printMatrix(needResources);
        } else if(strstr(command, "RQ") != NULL){
           printf("%s\n", requestResources(request));
        } else if(strstr(command, "RL") != NULL){
            printf("%s\n", releaseResources(request));
        } else if(strstr(command, "Run") != NULL){
            printf("RUN"); 
        } else if(strstr(command, "Exit") != NULL){
            printf("Program Complete\n");
            break;
        } else{
            printf("Invalid input, use one of RQ, RL, Status, Run, Exit\n");
        }
    } while (1);
    
    

    return 0;
}