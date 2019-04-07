#include <stdio.h>
#include <cstdlib>
#include <sys/types.h>
#include <unistd.h>

#include <string.h>
#include <sys/wait.h>
#include "parser.cpp"
#include "structs.h"


char * get_cmd(void) {
    char * line = (char *)malloc(100), * linep = line;
    size_t lenmax = 100, len = lenmax;
    int c;


    if(line == NULL)
        return NULL;

    for(;;) {
        c = fgetc(stdin);
        if(c == EOF)
            break;

        if(--len == 0) {
            len = lenmax;
            char * linen = (char *)realloc(linep, lenmax *= 2);

            if(linen == NULL) {
                free(linep);
                return NULL;
            }
            line = linen + (line - linep);
            linep = linen;
        }

        if((*line++ = c) == '\n')
            break;
    }
    *line = '\0';
    return linep;
}

int main() {
    const char *ext = "exit\n";
    for(;;){
        //char *line = get_cmd();
        printf("$>");
        fflush(stdout);
        cmd_line cmds = parse_line();
        if (strcmp(cmds.commands[0].name, ext) == 0){
            break;
        }


        if(cmds.count == 1){
            printf("name - %s\narg - %s\n",cmds.commands[0].name, cmds.commands[0].argv[1]);
            pid_t pid = fork();
            if(pid == -1){
                printf("\n bad fork");
            }
            else if( pid == 0){
                if(cmds.commands[0].argc > 0) {
                    if (execvp(cmds.commands[0].name, cmds.commands[0].argv) < 0) {
                        printf("\nbad execv\n");
                    }
                    printf("%s", cmds.commands[0].argv[1]);
                } else {
                    if (execlp(cmds.commands[0].name, cmds.commands[0].name, NULL) < 0) {
                        printf("\nbad execl for cmd: _%s_\n", cmds.commands[0].name);
                    }
                }
                exit(0);
            } else{
                wait(NULL);
            }
//
        } else {

            int in_fd, out_fd;
            for (int i = 0; i < cmds.count; i++) {
                int pipefd[2];// 0 = read, 1 = write

                if (pipe(pipefd) < 0) {
                    printf("\nPipe could not be initialized");
                }


                if (i > 0) {
                    dup2(in_fd, pipefd[0]);
                }
                in_fd = pipefd[1];
//                printf("proc%d: infd =%d\n", i, in_fd);
//                fflush(stdout);

                if (fork() == 0) { // in child
                    // 0 = read, 1 = write
                    printf("proc%d: in\n", i);
                    fflush(stdout);
                    dup2(pipefd[0], STDIN_FILENO);
                    close(pipefd[0]);


                    dup2(pipefd[1], STDOUT_FILENO);
                    close(pipefd[1]);

                    if (cmds.commands[i].argc > 0) {
                        if (execvp(cmds.commands[i].name, cmds.commands[i].argv) < 0) {
                            printf("\nbad execv\n");
                        }
                        printf("%s", cmds.commands[i].argv[1]);
                    } else {
                        if (execlp(cmds.commands[i].name, cmds.commands[i].name, NULL) < 0) {
                            printf("\nbad execl for cmd: _%s_\n", cmds.commands[0].name);
                        }
                    }

                    close(pipefd[1]);
                    close(pipefd[0]);
                    exit(0);

                } else {
                    close(pipefd[1]);
                }

            }

            for (int i = 0; i < cmds.count; i++) {
                wait(NULL);
            }

            for (int i = 0; i < cmds.count; i++) {

                printf("your cmd name is: %s \n", cmds.commands[i].name);
                for (int j = 0; j < cmds.commands[i].argc; j++) {
                    printf("\t param%d is: %s\n", j, cmds.commands[i].argv[j]);
                }
            }
        }
    }


    printf("\nBye!");
    return 0;
}