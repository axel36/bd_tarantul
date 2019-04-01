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
    char **argv;
    char *buf = (char *)malloc((end - start+1)* sizeof(char));
    int j = 0;
    int count = 0;
    while(line[start] == ' '){
        start++;
    }
    while(line[end] == ' ' || line[end] == '\n'){
        end--;
    }
    for(int i = start; i<= end; i++){
        buf[j] = line[i];
        j++;
        if(line[i] == ' ') {
            count++;
        }
    }
    buf[end - start+1] = 0;
    argv = (char **)malloc((count + 1)* sizeof(char *));

    for(int i = 0; i< count+1; i++){
        argv[i] = strsep(&buf, " ");
    }

    command.name = argv[0];

    command.argc = count;
    command.argv = argv;

    return command;
}

cmd_line parse_line(char *line){
    int count  = 1;
    cmd *cmds = (cmd*)malloc(count*sizeof(cmd));
    int start = 0;
    int j = 0;
    for (int i = 0; i <= strlen(line); i++){
        if (line[i] == '|'){
            // > cmd | cmd2
            //  0123456789
            // i = 5;
            cmds[count - 1] = parse_command(line, start, i-1);

            cmds = (cmd*)realloc(cmds, (count+1)* sizeof(cmd));
            start = i+1;
            count++;
            j++;
        }
    }
    cmds[j] = parse_command(line, start, strlen(line)-1);



    cmd_line commands{};
    commands.count = count;
    commands.commands = cmds;
    return commands;
}
