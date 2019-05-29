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

#include <sys/sem.h>

#include "find_min_max.h"
#include "utils.h"

int main(int argc, char **argv) {
	key_t key = ftok("./qwe", 1);
        struct sembuf buf;
        buf.sem_num = 0;
        buf.sem_flg = SEM_UNDO;
          int semid = semget(key, 1, 0666 | IPC_CREAT);
	semctl(semid, 0, SETVAL, 0);  
					buf.sem_op = 1;
                                        semop(semid, &buf, 1);

        int seed = -1;
        int array_size = -1;
        int pnum = -1;
        bool with_files = false;

        while (true)
        {
                int current_optind = optind ? optind : 1;
                static struct option options[] = 	{{"seed", required_argument, 0, 0},
                                                        {"array_size", required_argument, 0, 0},
                                                        {"pnum", required_argument, 0, 0},
                                                        {"by_files", no_argument, 0, 'f'},
                                                        {0, 0, 0, 0}};
                int option_index = 0;
                int c = getopt_long(argc, argv, "f", options, &option_index);
                if (c == -1) break;
                switch (c) 
                {
                case 0:
                        switch (option_index) 
                        {
                        case 0:
                                seed = atoi(optarg);
                                // your code here
                                if(seed < 0) 
                                {
                                        seed = -1;
                                        printf("Bad seed\n");
                                }
                                // error handling
                                break;
                        case 1:
                                array_size = atoi(optarg);
                                // your code here
                                if(array_size < 1) 
                                {
                                        array_size = -1;
                                        printf("Bad array size\n");
                                }
                                // error handling
                                break;
                        case 2:
                                pnum = atoi(optarg);
                                // your code here
                                if((pnum < 1)||(pnum > array_size))
                                {
                                        pnum = -1;
                                        printf("Bad pnum\n");
                                }
                                // error handling
                                break;
                        case 3:
                                with_files = true;
                                break;
                        defalut:
                                printf("Index %d is out of options\n", option_index);
                        }
                        break;
                case 'f':
                        with_files = true;
                        break;
                case '?':
                        break;
                default:
                        printf("getopt returned character code 0%o?\n", c);
                }
        }
        if (optind < argc) 
        {
                printf("Has at least one no option argument\n");
                return 1;
        }

        if (seed == -1 || array_size == -1 || pnum == -1) 
        {
                printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" \n", argv[0]);
                return 1;
        }

        int *array = malloc(sizeof(int) * array_size);
        GenerateArray(array, array_size, seed);
        int active_child_processes = 0;

        struct timeval start_time;
        gettimeofday(&start_time, NULL);

        FILE* f = NULL;
        int pipefd[2];
        if(!with_files)
        {
                if(pipe(pipefd) == -1)	
                { 
                        printf("Pipe not opened :(\n");
                        return 1;
                }
        }
        else
        {
                if(!(f = fopen("temp", "wb")))
                {
                        printf("open file error");
                        return 1;
                }
                char buff[2*sizeof(int)];
                memset(&buff, 0, 2*sizeof(int));
                for(int i = 0; i < pnum; i++)
                        fwrite(&buff, 1, 2*sizeof(int), f);
                fseek(f, 0, SEEK_SET);
        }

        for (int i = 0; i < pnum; i++) 
        {
                pid_t child_pid = fork();
                if (child_pid >= 0) 
                {
                        // successful fork
                        active_child_processes += 1;
                        if (child_pid == 0) 
                        {
                                // child process
                                int d = array_size / pnum;
                                int s = d*i;
                                int e = (i == pnum - 1) ? array_size : d*(i+1);
                                struct MinMax min_max0 = GetMinMax(array, s, e);
                                // parallel somehow
                                if (with_files) 
                                {
          				semid = semget(key, 1, 0666);
        				buf.sem_op = -1;
                                        semop(semid, &buf, 1);
                                        fseek(f, 2*sizeof(int)*i, SEEK_SET);
                                        if(fwrite(&min_max0, sizeof(int), 2, f) != 2)
                                        {
                                                printf("error with file write :(\n");
                                                return 1;
                                        }
					fflush(f);
					buf.sem_op = 1;
                                        semop(semid, &buf, 1);
                                } 
                                else 
                                {
                                        // use pipe here
                                        close(pipefd[0]);
                                        write(pipefd[1], &min_max0, 2*sizeof(int));
                                        close(pipefd[1]);
                                }
                                return 0;
                        }

                }
                else
                {
                        printf("Fork failed!\n");
                        return 1;
                }
        }
        while (active_child_processes > 0) 
        {
                // your code here
                wait(NULL);
                active_child_processes -= 1;
        }


        struct MinMax min_max;
        min_max.min = INT_MAX;
        min_max.max = INT_MIN;

        if(with_files)
        {	
                fclose(f);
                f = fopen("temp", "rb");
                if(!f)
                {
                        printf("File open error.");
                        return 1;
                }
        }
        for (int i = 0; i < pnum; i++) 
        {
                struct MinMax min_max0;
                if (with_files) 
                {
                        // read from file
                        if(fread(&min_max0, sizeof(int), 2, f) != 2)
			{
				printf("read file error\n");
				return 1;
			}
                } 
                else
                {
                        // read from pipes
                        read(pipefd[0], &min_max0, 2*sizeof(int));
                }
                if (min_max0.min < min_max.min) min_max.min = min_max0.min;
                if (min_max0.max > min_max.max) min_max.max = min_max0.max;
        }
        if(with_files)
        {	
                fclose(f);
                remove("temp");
        }
        else
        {
                close(pipefd[1]);
                close(pipefd[0]);
        }

        struct timeval finish_time;
        gettimeofday(&finish_time, NULL);

        double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
        elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

        free(array);

        printf("Min: %d\n", min_max.min);
        printf("Max: %d\n", min_max.max);
        printf("Elapsed time: %fms\n", elapsed_time);
        fflush(NULL);

          semctl(semid, 1, IPC_RMID);
        return 0;
}
