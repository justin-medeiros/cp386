#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include <dirent.h>
#define MEMORY_ALLOCATION_LENGTH 1024

int main(){
    char userInput;
    char cwd[MEMORY_ALLOCATION_LENGTH];
    getcwd(cwd, sizeof(cwd));
    DIR *dir = malloc(MEMORY_ALLOCATION_LENGTH);
    int isDirectoryOpen = 0;
    while(userInput != 'q'){
        printf("Select the option(s) appropriately by entering the number:\n"
        "Enter 1 for creating a directory\n"
        "Enter 2 for removing directory\n"
        "Enter 3 for printing working directory\n"
        "Enter 4 for changing directory one level up\n"
        "Enter 5 for reading the contents of directory\n"
        "Enter 6 for closing the current directory\n"
        "Enter q to exit the program\n"
        );
        scanf(" %c", &userInput);
        switch (userInput)
        {
        case '1':
            char *addDirectoryName = malloc(1024);
            while(1){
                printf("Enter the Directory name you want to create:\n");
                scanf("%s", addDirectoryName);
                char *path = malloc(MEMORY_ALLOCATION_LENGTH);
                // Create the entire path by concatenating the directory name typed by the user to the current directory
                strcpy(path, cwd);
                strcat(path, "/");
                strcat(path, addDirectoryName);
                // Create the directory in the current path
                int result = mkdir(path, 0777); // use 0777 to signify rwxrwxrwx mode
                free(path);
                if(result == 0){
                    printf("Directory is Created Successfully.\n");
                    break;
                } else{
                    printf("Directory already exists. Please try again.\n");
                    break;
                }
            }
            free(addDirectoryName);
            break;
        case '2':
            char *removeDirectoryName = malloc(1024);
            while(1){
                printf("Enter the Directory name you want to remove:\n");
                scanf("%s", removeDirectoryName);
                char *path = malloc(MEMORY_ALLOCATION_LENGTH);
                // Create the entire path by concatenating the directory name typed by the user to the current directory
                strcpy(path, cwd);
                strcat(path, "/");
                strcat(path, removeDirectoryName);
                // Remove the directory in the current path
                int result = rmdir(path);  
                free(path);
                if(result == 0){
                    printf("Directory is removed Successfully.\n");
                    break;
                } else{
                    printf("Directory does not exist. Please try again.\n");
                    break;
                }
            }
            free(removeDirectoryName);
            break;
        case '3':
            printf("Current Working Directory is: %s\n", cwd);
            break;
        case '4':
            if(strcmp(cwd, "/") == 0){
                printf("You are already in the root Directory. Cannot level up!\n");
            } else{
                printf("Working Directory Before Operation: %s\n", cwd);
                // Change the directory to the parent to level up
                int result = chdir("..");
                char levelUpCwd[MEMORY_ALLOCATION_LENGTH];
                getcwd(levelUpCwd, sizeof(levelUpCwd));
                if(result == 0){
                    printf("Directory Changed Successfully.\n");
                    printf("Working Directory After Operation: %s\n", levelUpCwd);
                } else{
                    printf("Directory could not be changed. Please try again.\n");    
                }
            }
            break;
        case '5':
            // Open the current directory
            dir = opendir(cwd);
            struct dirent *file;
            // Read all contents from the current directory
            while((file = readdir(dir)) != NULL){
                printf("%s\n", file->d_name);
            }
            isDirectoryOpen = 1;
            break;
        case '6':
            // Close directory if it is opened
            if(isDirectoryOpen == 1){
                int result = closedir(dir);
                isDirectoryOpen = 0;
                if(result == 0){
                    printf("Directory Closed Successfully.\n");
                } else{
                    printf("Directory could not be closed.\n");
                }
            } else{
                printf("Directory is already closed.\n");
            }
            break;
        default:
            break;
        }
    }
    return 0;
}

