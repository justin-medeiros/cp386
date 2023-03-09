#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#define ROWS 9
#define COLS 9
#define BOX_SIZE 3

int sudokuArray[ROWS][COLS];
int isSudokuValid = 1;

typedef struct
{
    int row;
    int column;
} parameters;

void *checkColumn(void *data){
    parameters *params = (parameters*)data;
    int r = params->row;

    // Temporary array to hold the count of the numbers that we have gone through
    int tempArray[COLS];

    // Set all values in the temporary array to 0
    memset(tempArray, 0, sizeof(tempArray));

    // Go through the entire column at the passed row and check to see if every number is unique
    for (int c = 0; c<COLS; c++){
        if(tempArray[sudokuArray[r][c]-1] == 0){
            tempArray[sudokuArray[r][c]-1] = 1;
        } else{
            isSudokuValid = 0;
            pthread_exit(0);
        }
    }

    pthread_exit(0);
}

void *checkRow(void *data){
    parameters *params = (parameters*)data;
    int c = params->column;

    // Temporary array to hold the count of the numbers that we have gone through
    char tempArray[ROWS];

    // Set all values in the temporary array to 0
    memset(tempArray, 0, sizeof(tempArray));

    // Go through the entire row at the passed column and check to see if every number is unique
    for (int r = 0; r<ROWS; r++){
        if(tempArray[sudokuArray[r][c]-1] == 0){
            tempArray[sudokuArray[r][c]-1] = 1;
        } else{
            isSudokuValid = 0;
            pthread_exit(0);
        }
    }

    pthread_exit(0);
}

void *checkGrid(void *data){
    parameters *params = (parameters*)data;
    int row = params->row;
    int col = params->column;

    // Temporary array to hold the count of the numbers that we have gone through
    char tempArray[ROWS];

    // Set all values in the temporary array to 0
    memset(tempArray, 0, sizeof(tempArray));

    // Go through the 3x3 box to see if every number is unique
    for (int i = row; i<row+BOX_SIZE; i++){
        for (int j = col; j<col+BOX_SIZE; j++){
            if(tempArray[sudokuArray[i][j]-1] == 0){
                tempArray[sudokuArray[i][j]-1] = 1;
            } else{
                isSudokuValid = 0;
                pthread_exit(0);
            }
        }
    }
    pthread_exit(0);
}

int main(int argc, char ** argv){
    if(argc != 2){
        printf("Error. Please include a file.\n");
        return 1;
    } else{
        int r = 0;
        int c = 0;

        int fd = open(argv[1], O_RDONLY);
        if(fd == -1){
            printf("File could not be opened or does not exist.\n");
            return 1;
        }
        char ch;

        // Read every character in the file and put into a sudoku 2d array
        while(read(fd, &ch, sizeof(char)) > 0){
            if(ch != ' '){
                if(ch == '\n'){
                    r++;
                    c = 0;
                } else{
                    sudokuArray[r][c] = atoi(&ch);
                    c++;
                }
            }
        }
        close(fd);

        parameters *data = (parameters *) malloc(sizeof(parameters));
        data->row = 0;
        data->column = 0;

        pthread_t tid_col;
        pthread_t tid_row;
        pthread_t tid_box;

        pthread_attr_t attr;
        pthread_attr_init(&attr); 

        // Loop through size of sudoku array to determine if columns and row is valid; can use COLS or ROWS as both are 9
        for(int i = 0; i<COLS; i++){
            // Determine if each column is valid
            pthread_create(&tid_col, &attr, checkColumn, data);
            pthread_join(tid_col, NULL);
            if(!isSudokuValid){
                break;
            }

            // Determine if each row is valid
            pthread_create(&tid_row, &attr, checkRow, data);
            pthread_join(tid_row, NULL);
            if(!isSudokuValid){
                break;
            }
            data->column++;
            data->row++;
        }

        // If sudoku is still valid, check every 3x3 box
        if(isSudokuValid){
            data->row = 0;
            data->column = 0;
            for(int i = 0; i<ROWS; i+= BOX_SIZE){
                for(int j = 0; j<COLS; j+= BOX_SIZE){
                    data->column=j;
                    pthread_create(&tid_box, &attr, checkGrid, data);
                    pthread_join(tid_box, NULL);
                    if(!isSudokuValid){
                        break;
                    }
                }
                data->row=i;
            }
        }

        // Output
        printf("Sudoku Puzzle Solution is:\n");
         for(int i = 0; i<ROWS; i++){
            for(int j = 0; j<COLS; j++){
                if(j == COLS-1){
                    printf("%d\n", sudokuArray[i][j]);
                } else{
                    printf("%d ", sudokuArray[i][j]);
                }
            } 
        }
        printf("Sudoku puzzle is %s\n", isSudokuValid ? "valid" : "invalid");
        free(data);
    }
    
    return 0;
}