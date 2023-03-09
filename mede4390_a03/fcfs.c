#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define CATEGORIES 3
#define BUFFER_SIZE 255

typedef struct
{
    int threadId ;
    int arr_time;
    int burst_time;
    int completion_time;
    int waiting_time;
    int turnaround_time;
} threadInfo;

int calculateCompletionTime(int prevCompleteTime, int currentBurstTime){
    return prevCompleteTime + currentBurstTime;
}

int calculateWaittime(int threadTurnAroundTime, int threadBurstTime){
    return threadTurnAroundTime - threadBurstTime;
}

int calculateTurnaround(int threadCompleteTime, int threadArrivalTime){
    return threadCompleteTime - threadArrivalTime;
}


int main(int argc, char ** argv){
     if(argc != 2){
        printf("Error. Please include a file.\n");
        return 1;
    } else{
        int values[CATEGORIES];
        int fd = open(argv[1], O_RDONLY);
        if(fd == -1){
            printf("File could not be opened or does not exist.\n");
            return 1;
        }
        printf("%-16s%-16s%-16s%-16s%-24s%-20s\n", "Thread ID", "Arrival Time", "Burst Time", "Completion Time", "Turn-Around Time", "Waiting Time");

        threadInfo *prevThread = (threadInfo *) malloc(sizeof(threadInfo));
        threadInfo *currentThread = prevThread;
        currentThread->threadId = 0;
        currentThread->arr_time = 0;
        currentThread->burst_time = 0;
        currentThread->completion_time = 0;

        char buffer[BUFFER_SIZE];
        int bytes_read = 0;
        char *value = (char*) malloc(BUFFER_SIZE);
        int valCount = 0;
        int count = 0;
        float threadCount = 0.0;
        float waittimeTotal = 0.0;
        float turnaroundTotal = 0.0;
        
        // Read every character in the file
        while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
            for (int i = 0; i < bytes_read; i++) {
                char c = buffer[i];
                // If we encounter next space, print the values of the current thread
                if (c == '\n') {
                    prevThread = currentThread;
                    currentThread->threadId = values[0];
                    currentThread->arr_time = values[1];
                    currentThread->burst_time = values[2];
                    
                    currentThread->completion_time = calculateCompletionTime(prevThread->completion_time, currentThread->burst_time);
                    currentThread->turnaround_time = calculateTurnaround(currentThread->completion_time, currentThread->arr_time);
                    currentThread->waiting_time = calculateWaittime(currentThread->turnaround_time, currentThread->burst_time);
                    printf("%-24d%-16d%-16d%-16d%-16d%d\n", currentThread->threadId, currentThread->arr_time, currentThread->burst_time, currentThread->completion_time, currentThread->turnaround_time, currentThread->waiting_time);
                    count = 0;
                    valCount = 0;
                    threadCount++;
                    waittimeTotal += currentThread->waiting_time;
                    turnaroundTotal += currentThread->turnaround_time;
                } else if (c != ' ' && c != ',') {
                    value[valCount++] = c;
                    // If it is the last character in the file, print the values of the current thread
                    if(i== bytes_read-1){
                        value[valCount] = '\0';
                        values[count] = atoi(value);
                        count++;
                        valCount = 0;

                        prevThread = currentThread;
                        currentThread->threadId = values[0];
                        currentThread->arr_time = values[1];
                        currentThread->burst_time = values[2];
                        
                        currentThread->completion_time = calculateCompletionTime(prevThread->completion_time, currentThread->burst_time);
                        currentThread->turnaround_time = calculateTurnaround(currentThread->completion_time, currentThread->arr_time);
                        currentThread->waiting_time = calculateWaittime(currentThread->turnaround_time, currentThread->burst_time);
                        printf("%-24d%-16d%-16d%-16d%-16d%d\n", currentThread->threadId, currentThread->arr_time, currentThread->burst_time, currentThread->completion_time, currentThread->turnaround_time, currentThread->waiting_time);
                        threadCount++;
                        waittimeTotal += currentThread->waiting_time;
                        turnaroundTotal += currentThread->turnaround_time;
                    }
                    // For if there is a multidigit number
                    else if (buffer[i+1] == ' ' || buffer[i+1] == ',' || buffer[i+1] == '\n') {
                        value[valCount] = '\0';
                        values[count] = atoi(value);
                        count++;
                        valCount = 0;
                    } 


                }
            }
        }
        // Average output
        printf("The average waiting time: %.2f\n", (waittimeTotal/threadCount));
        printf("The average turn-around time: %.2f\n", (turnaroundTotal/threadCount));
        
        free(value);
        free(prevThread);
        close(fd);
      
    }
    return 0;
}