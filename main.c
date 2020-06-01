
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <wait.h>
#include <stdlib.h>

struct process {
    int pid_num;
    char *ord[100];
    int num_ord;
};

struct process *processes[100];

void free_all();

int process_count = 0;

int count;
int main() {
    int dir;
    int cd_flag = 0;
    char path0[1024], path1[1024];
    char *array_commands[100];
    char command[100];
    pid_t pid;
    pid_t p;
    char order[100];
    int stop = 0;
    while (!stop) {
        count = 1;
        sleep((unsigned int) 0.5);
         printf("> ");
//get the command from the user and strip the \n
        fgets(order, 100, stdin);
        for( int i = 0; i < 100; i++) {
            if (strcmp(&order[i], "\n") == 0) {
                order[i] = 0;
            }
        }
//get the first word which is the command
        char *word = strtok(order, " ");
        strcpy(command, word);
        array_commands[0] = word;
        word = strtok(NULL, " ");
        //save the pramaters of the command
        while (word != NULL) {
            array_commands[count] = word;
            word = strtok(NULL, " ");
            count++;
        }
        array_commands[count] = NULL;

        if (strcmp(command, "cd") == 0) {
            printf("%d\n", getpid());
            if (count > 2) {
                fprintf(stderr, "Error: To many arguments\n");
            } else {
//save the command for the history
                int num = 0;
                struct process *p1 = malloc(sizeof(struct process) + 1);
                p1->pid_num = getpid();
                for (int i = 0; array_commands[i] != NULL; i++) {
                    p1->ord[i] = malloc(100);
                    strcpy(p1->ord[i], array_commands[i]);
                    num++;
                }
                p1->num_ord = num;
                processes[process_count] = p1;
                process_count++;
//split the order of the directory change to move one by one
                char *change_dir = strtok(array_commands[1], "/");
                if (change_dir == NULL) {}
                else {
                    do {
                        if (strcmp(change_dir, "-") == 0) {
                            if (cd_flag == 1) {
                                dir = chdir(path1);
                                if (dir == -1) {
                                    fprintf(stderr, "Error: No such file or directory\n");
                                    free_all();
                                    exit(0);
                                }
                            } else {
                                dir = chdir(path0);
                                if (dir == -1) {
                                    fprintf(stderr, "Error: No such file or directory\n");
                                    free_all();
                                    exit(0);
                                }
                            }
                        } else {
                            if (strstr(change_dir, "~") != NULL) {
                                char *path = malloc(sizeof(getenv("USERNAME")) + 10);
                                strcat(path, "/home/");
                                strcat(path, getenv("USERNAME"));
                                dir = chdir(path);
                                free(path);
                                if (dir == -1) {
                                    fprintf(stderr, "ERROR: No such file or directory\n");
                                    free_all();
                                    exit(0);
                                }
                            } else {
                                if (strcmp(change_dir, "..") == 0) {
                                    dir = chdir("../");
                                    if (dir == -1) {
                                        fprintf(stderr, "ERROR: No such file or directory\n");
                                        free_all();
                                        exit(0);
                                    }
                                } else {
                                    char *path = malloc(sizeof(change_dir) + 1);
                                    strcat(path, change_dir);
                                    strcat(path, "/");
                                    int ret;
                                    dir = chdir(path);
                                    free(path);
                                    if (dir == -1) {
                                        fprintf(stderr, "ERROR: No such file or directory\n");
                                        free_all();
                                        exit(0);
                                    }
                                }
                            }
                        }
                        //as long as its to move
                        change_dir = strtok(NULL, "/");
                    } while (change_dir != NULL);
                }
                //save the last two paths for the cd- command and mainten flags in order to know which one
                char up_path[1024];
                getcwd(up_path, sizeof(up_path));
                if (cd_flag == 0) {
                    strcpy(path0, up_path);
                    cd_flag = 1;
                } else {
                    strcpy(path1, up_path);
                    cd_flag = 0;
                }
            }
        } else {

            if (strcmp(command, "exit") == 0) {
                printf("%d\n", getpid());
                free_all();
                return(0);
            } else {

                if (strcmp(command, "history") == 0) {
                    //going over all of the saves process
                    for (int i = 0; i < process_count; i++) {
                        p = waitpid(processes[i]->pid_num, NULL, WNOHANG);
                        printf("%d ", processes[i]->pid_num);
                        for (int j = 0; j < processes[i]->num_ord; j++) {
                            printf("%s ", processes[i]->ord[j]);
                        }
                        if (p == 0) {
                            printf("RUNNING\n");
                        } else {
                            printf("DONE\n");
                        }
                    }
                    //print the history himself
                    printf("%d history RUNNING\n", getpid());
                    //save it in the process list
                    struct process *p2 = malloc(sizeof(struct process) + 1);
                    p2->pid_num = getpid();
                    p2->ord[0] = malloc(100);
                    strcpy(p2->ord[0],"history");
                    p2->num_ord = 1;
                    processes[process_count] = p2;
                    process_count++;
                } else {
                    if (strcmp(command, "jobs") == 0) {
                        for (int i = 0; i < process_count; i++) {
                        p = waitpid(processes[i]->pid_num, NULL, WNOHANG);
                        // only if the process is still running
                        if (p == 0) {
                            printf("%d ", processes[i]->pid_num);
                            for (int j = 0; j < processes[i]->num_ord; j++) {
                                printf("%s ", processes[i]->ord[j]);
                                }
                            printf("\n");
                            }
                        }
                        //add him himself to the process list
                        struct process *p2 = malloc(sizeof(struct process) + 1);
                        p2->pid_num = getpid();
                            p2->ord[0] = malloc(100);
                        strcpy(p2->ord[0],"jobs");
                        p2->num_ord = 1;
                        processes[process_count] = p2;
                        process_count++;

                    } else {
                        // if its a foreground, delete the &
                            if (strcmp(array_commands[--count], "&") == 0) {
                                for (int i = 0; i < 100; i++) {
                                    if (strcmp(array_commands[i], "&") == 0) {
                                        array_commands[i] = 0;
                                        break;
                                    }
                                }
                                //the son need to do the command
                                if ((pid = fork()) == 0) {
                                    printf("%d\n", getpid());
                                    //deal with the echo with the "
                                    if (strcmp(array_commands[0],"echo") == 0) {
                                        if (strstr(array_commands[1], "\"") != NULL) {
                                            strcpy(array_commands[1], &(array_commands[1])[1]);
                                        }
                                        if (strstr(array_commands[count-1], "\"") != NULL) {
                                            array_commands[count-1][strlen(array_commands[count-1])-1] = 0;
                                        }
                                    }
                                    int ex;
                                    ex = execvp(command, array_commands);
                                    if (ex == -1) {
                                        fprintf(stderr, "Error in system call");
                                    }
                                } else {
                                    if (pid == -1) {
                                        fprintf(stderr, "Error in system call");
                                    } else {
                                        int num = 0;
                                        struct process *p1 = malloc(sizeof(struct process) + 1);
                                        p1->pid_num = pid;
                                        for (int i = 0; array_commands[i] != NULL; i++) {
                                            p1->ord[i] = malloc(100);
                                            strcpy(p1->ord[i], array_commands[i]);
                                            num++;
                                        }
                                        p1->num_ord = num;
                                        processes[process_count] = p1;
                                        process_count++;
                                    }
                                }
                        } else {
                            if ((pid = fork()) == 0) {
                                printf("%d\n", getpid());
                                //deal with the echo with the "
                                if (strcmp(array_commands[0],"echo") == 0) {
                                    if (strstr(array_commands[1], "\"") != NULL) {
                                        strcpy(array_commands[1], &(array_commands[1])[1]);
                                    }
                                    if (strstr(array_commands[count], "\"") != NULL) {
                                        array_commands[count][strlen(array_commands[count])-1] = 0;
                                    }
                                }
                                int ex;
                                ex = execvp(command, array_commands);
                                if (ex == -1) {
                                    fprintf(stderr, "Error in system call");
                                }
                            } else {
                                if (pid == -1) {
                                    fprintf(stderr, "Error in system call");
                                } else {
                                    int num = 0;
                                    struct process *p2 = malloc(sizeof(struct process) + 1);;
                                    p2->pid_num = pid;
                                    for (int i = 0; array_commands[i] != NULL; i++) {
                                        p2->ord[i] = malloc(100);
                                        strcpy(p2->ord[i], array_commands[i]);
                                        num++;
                                    }
                                    p2->num_ord = num;
                                    processes[process_count] = p2;
                                    process_count++;
                                    int stat;
                                    wait(&stat);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void free_all() {
    for (int i = 0; i < process_count; i++) {
        for (int j = 0; j < processes[i]->num_ord; j++) {
            free(processes[i]->ord[j]);
        }
        free(processes[i]);
    }
}
