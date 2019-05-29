#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <getopt.h>

int main(){
    int pid = fork();
    if(pid == 0){
        execl("sequential_min_max1", " ", "5", "4");
        printf("OH NO! \n");
    }
    else{
        printf("sequential_min_max1 start! \n");
        wait(NULL);
    }    return 0;
}