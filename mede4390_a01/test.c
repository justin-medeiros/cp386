 #include <syscall.h>
 #include <unistd.h>
 #include <stdio.h>
 #include <stdlib.h>

int main(){
    // int pid = fork();
    // if(pid<0){
    //     printf("ERROR");
    // } else if(pid!=0){
    //     printf("PARENT\n");
    //     printf("PARENT PID: %d\n", pid);
    // } else{
    //     printf("CHILD\n");
    //     printf("CHILD PID: %d\n", pid);
    // }

    // char *program_name = "ls";
    // //A null terminated array of character pointers char
    // char *args[]={program_name, NULL};
    // execlp(program_name,args);
    // return 0;

    int child_pid = fork();
    // Parent process 
    if (child_pid > 0){
        printf("In Parent Process.!\n"); // Making the Parent
        // Process to Sleep for some time.
        sleep(10);
    }
    else{
        printf("In Child process.!\n");
        exit(0);
    }

// Orphan
    // int child_pid = fork();
    // // Parent process
    // if (child_pid > 0){
    //     printf("In Parent Process.!\n");
    // }
    // else{
    //     printf("In Child process.!\n"); // Making the
    //     // Child Process to Sleep for some time.
    //     sleep(10);
    //     printf("After Sleep Time");
    // } 

}
