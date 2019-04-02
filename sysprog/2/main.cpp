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

    printf("$>");
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
        char *line = get_cmd();
        int l = strlen(line));
        if(line[l-1]=='\\   ')
        if (strcmp(line, ext) == 0){
            break;
        }

        cmd_line cmds = parse_line(line);

        if(cmds.count == 1){
//            printf("name - %s\narg - %s\n",cmds.commands[0].name, cmds.commands[0].argv[1]);
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

            int ** pipefds = (int **)malloc((cmds.count - 1)* sizeof(int*)); // 0 = read, 1 = write
            for(int i = 0; i< cmds.count-1; i++){
                int tmp[2];
                if (pipe(tmp) < 0) {
                    printf("\nPipe could not be initialized");
                }
                pipefds[i] = tmp;
            }
            pid_t pd1, pd2;

            pd1 = fork();
            if (pd1 < 0) {
                printf("\nbad fork");
            }

            if (pd1 == 0) {
                close(pipefds[0][0]);
                dup2(pipefds[0][1], STDOUT_FILENO);
                close(pipefds[0][1]);

                if (execvp(cmds.commands[0].name, cmds.commands[0].argv) < 0) {
                    printf("\nCould not execute command 1..");
                    exit(0);
                }
                exit(0);
            } else {
                // Parent executing

                pd2 = fork();

                if (pd2 < 0) {
                    printf("\nCould not fork");
                }

                // Child 2 executing..
                // It only needs to read at the read end
                if (pd2 == 0) {
                    close(pipefds[0][1]);
                    dup2(pipefds[0][0], STDIN_FILENO);
                    close(pipefds[0][0]);
                    if (execvp(cmds.commands[1].name, cmds.commands[1].argv) < 0) {
                        printf("\nCould not execute command 2..");
                    }
                    exit(0);
                } else {
                    // parent executing, waiting for two children
                    close(pipefds[0][1]);
                    wait(NULL);
                    wait(NULL);
                }
            }
//            for (int i = 0; i < cmds.count; i++) {
//
//                printf("your cmd name is: %s \n", cmds.commands[i].name);
//                for (int j = 0; j < cmds.commands[i].argc; j++) {
//                    printf("\t param%d is: %s\n", j, cmds.commands[i].argv[j]);
//                }
//            }
        }
    }

    printf("\nBye!");
    return 0;
}