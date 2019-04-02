#include "structs.h"
#include <string.h>
#include <stdlib.h>


cmd parse_command(char *line, int start, int end) {
    cmd command{};
    char **argv = (char **) malloc(2 * sizeof(char *));
    char *param = (char *) malloc((end - start + 1) * sizeof(char));

    int count = 0;
    while (line[start] == ' ') {
        start++;
    }
    while (line[end] == ' ' || line[end] == '\n') {
        end--;
    }

    int j = 0;
    char prev = 0;
    for (int i = start; i <= end; i++) {
        switch (line[i]) {
            case ' ':
                param[j] = 0;
                argv[count] = param;
                count++;
                argv = (char **) realloc(argv, (count + 1) * sizeof(char *));
                param = (char *) malloc((end - i + 1) * sizeof(char));
                j = 0;
                prev = 0;
                break;
            case '"':
                i++;
                for(; i <= end;i++){
                    if(line[i] == '\\'){
                        prev = '\\';
                        continue;
                    }
                    if(line[i] != '"' || prev == '\\'){
                        param[j] = line[i];
                        j++;
                        prev = line[i];
                    }else{
                        param[j] = 0;
                        argv[count] = param;
                        count++;
                        argv = (char **) realloc(argv, (count + 1) * sizeof(char *));
                        param = (char *) malloc((end - i + 1) * sizeof(char));
                        j = 0;
                        prev = 0;
                        break;
                    }
                }
                break;
            case '\'':
                break;
            default:
                param[j] = line[i];
                j++;

        }
    }
    argv[count+1] = NULL;
//    free(param); TODO: check

    command.name = argv[0];
    command.argc = count;
    command.argv = argv;

    return command;
}

cmd_line parse_line(char *line) {
    int count = 1;
    cmd *cmds = (cmd *) malloc(count * sizeof(cmd));
    int start = 0;
    int j = 0;
    int max_len = 3;
    int len = 0;
    char **tokens = (char**)malloc(max_len * sizeof(char));
    int token_max_len = 4;
    int token_len = 0;
    char *token = (char*)malloc( token_max_len* sizeof(char));
    for (int i = 0; i <= strlen(line); i++) {

        if (line[i] == '|') {
            // > cmd | cmd2
            //  0123456789
            // i = 5;
            cmds[count - 1] = parse_command(line, start, i - 1);

            cmds = (cmd *) realloc(cmds, (count + 1) * sizeof(cmd));
            start = i + 1;
            count++;
            j++;
        }
    }
    cmds[j] = parse_command(line, start, strlen(line) - 1);
//    printf("p-%s\n",line);


    cmd_line commands{};
    commands.count = count;
    commands.commands = cmds;
    return commands;
}
