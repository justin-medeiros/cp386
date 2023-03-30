#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define BUFFER_SIZE 255
#define NUM_THREADS 5
#define NUM_RESOURCES 4

// Global variables
int maximumResources[NUM_THREADS][NUM_RESOURCES];
int availableResources[NUM_RESOURCES];
int needResources[NUM_THREADS][NUM_RESOURCES];
int allocationResources[NUM_THREADS][NUM_RESOURCES];

// Helper function to check if a thread can be run safely
int canRunSafely(int thread, int work[]) {
    for (int i = 0; i < NUM_RESOURCES; i++) {
        if (needResources[thread][i] > work[i]) {
            return 0;
        }
    }
    // Thread can be run safely
    return 1;
}

int safetyAlg(int sequence[]) {
    // Initialize safety values
    int finish[NUM_THREADS] = {0};
    int work[NUM_RESOURCES];
    memcpy(work, availableResources, sizeof(availableResources));
    int seqCount = 0;

    // Iterate until all threads are finished or there is no more possible thread to run
    int possibleThreadFound = 1;
    while (possibleThreadFound) {
        possibleThreadFound = 0;

        // Check all threads
        for (int i = 0; i < NUM_THREADS; i++) {
            // If the thread is not finished and can be run safely
            if (finish[i] == 0 && canRunSafely(i, work)) {
                finish[i] = 1;
                // Add the thread to the sequence
                if (sequence != NULL) {
                    sequence[seqCount++] = i;
                }
                // Update the work array
                for (int j = 0; j < NUM_RESOURCES; j++) {
                    work[j] += allocationResources[i][j];
                }
                possibleThreadFound = 1;
            }
        }
    }

    // Check if all threads finished
    for (int i = 0; i < NUM_THREADS; i++) {
        if (finish[i] == 0) {
            if (sequence != NULL) {
                sequence = NULL;
            }
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
    if(safetyAlg(NULL) == 1){
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

void* runThread(void* sequencePtr){
    // Will print all required output from the thread
    int sequenceNumber = *((int*) sequencePtr); 
	printf("--> Customer/Thread %d\n", sequenceNumber);
	printf("    Allocated resources:  ");
    for(int i = 0; i<NUM_RESOURCES;i++){
        if(i == NUM_RESOURCES-1){
            printf("%d\n",allocationResources[sequenceNumber][i]);
        } else{
             printf("%d ",allocationResources[sequenceNumber][i]);
        }
    }
	printf("    Needed: ");
    for(int i = 0; i<NUM_RESOURCES;i++){
        if(i == NUM_RESOURCES-1){
            printf("%d\n",needResources[sequenceNumber][i]);
        } else{
             printf("%d ",needResources[sequenceNumber][i]);
        }
    }
    printf("    Available:  ");
    for(int i = 0; i<NUM_RESOURCES;i++){
        if(i == NUM_RESOURCES-1){
            printf("%d\n",availableResources[i]);
        } else{
             printf("%d ",availableResources[i]);
        }
    }

	printf("    Thread has started\n");
    // Allocate all neccessary resources
    int tempNeedResource[5];
    tempNeedResource[0] = sequenceNumber;
    for (int i = 0; i<NUM_RESOURCES; i++){
        tempNeedResource[i+1] = needResources[sequenceNumber][i];
    }
    requestResources(tempNeedResource);
	printf("    Thread has finished\n");
	printf("    Thread is releasing resources\n");
    // Release all neccessary resources
    int tempAllocationResource[5];
    tempAllocationResource[0] = sequenceNumber;
    for (int i = 0; i<NUM_RESOURCES; i++){
        tempAllocationResource[i+1] = allocationResources[sequenceNumber][i];
    }
    releaseResources(tempAllocationResource);
	printf("    New Available:  ");
    for (int i = 0; i < NUM_RESOURCES; i++)
    {
        if(i == NUM_RESOURCES-1){
            printf("%d\n",availableResources[i]);
        } else{
             printf("%d ",availableResources[i]);
        }
    }
    
    return NULL;
}

void runResources(){
    int sequence[NUM_THREADS];
    if(safetyAlg(sequence)){
        printf("Safe Sequence is: ");
        for(int i = 0; i<NUM_THREADS;i++){
            if(i == NUM_THREADS-1){
                printf("%d\n",sequence[i]);
            } else{
                printf("%d ",sequence[i]);
            }
        }
        // Create and run threads for each sequence
        for(int i = 0; i<NUM_THREADS;i++){
            pthread_t tid;
            pthread_attr_t attr;
            pthread_attr_init(&attr); 
            pthread_create(&tid, &attr, runThread, (void*) &sequence[i]);

            pthread_join(tid, NULL);
        }
    } else{
        printf("There is no safe sequence\n");
    }
    

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
        
        // Get the number from the file and store in the buffer
        if(ch != ',' && ch != '\n'){
            buffer[bufferIndex++] = ch;
        } else{
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
        needResources[r][c] = maximumResources[r][c] - allocationResources[r][c];
    }

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
            runResources(); 
        } else if(strstr(command, "Exit") != NULL){
            printf("Program Complete\n");
            break;
        } else{
            printf("Invalid input, use one of RQ, RL, Status, Run, Exit\n");
        }
    } while (1);
    
    

    return 0;
}