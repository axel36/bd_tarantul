#include "structs.h"
#include <string.h>
#include <stdlib.h>

void get_substr(const char *line,char *rez, int start, int end){
    int j = 0;
    for(int i = start; i < end; i++){
        rez[j] = line[i];
        j++;
    }
}
cmd parse_command(char *line, int start, int end){
    cmd command{};
    char *tmp;
    const char **argv;

    for(int i = start; i< end; i++){
        if(line[i] == ' '){
            tmp = (char*)malloc((i-start)* sizeof(char));
            get_substr(line, tmp, start, i);
            start = i+1;
            break;
        }
    }
    command.name = tmp;
    if(start == end){
        command.argc = 0;
        return command;
    }
    // name param1 param2
    int argc = 1;
    argv = (const char **) malloc(argc * sizeof(char *));
    for(int i = start; i< end; i++){
        if(line[i] == ' '){
            tmp = (char*)malloc((i-start)* sizeof(char));
            get_substr(line, tmp, start, i);
            argv[argc - 1] = tmp;
            argv = (const char**)realloc(argv, (argc+1)* sizeof(char *));
            start = i+1;
            argc++;
        }
    }
    tmp = (char*)malloc((end-start)* sizeof(char));
    get_substr(line, tmp, start, end);
    argv[argc - 1] = tmp;
    command.argc = argc;
    command.argv = argv;
    return command;
}

cmd_line parse_line(char *line){
    size_t count  = 1;
    cmd *cmds = (cmd*)calloc(count, sizeof(cmd));
    int start = 0;
    int j = 0;
    for (int i = 0; i <= strlen(line); i++){
        if (line[i] == '|'){
            // > cmd |
            //  012345
            // i = 5;
            cmds[count - 1] = parse_command(line, start+1, i-1);

            cmds = (cmd*)realloc(cmds, (count+1)* sizeof(cmd));
            start = i+1;
            count++;
            j++;
        }
    }
    cmds[j] = parse_command(line, start+1, strlen(line)-1);



    cmd_line commands{};
    commands.count = count;
    commands.commands = cmds;
    return commands;
}
