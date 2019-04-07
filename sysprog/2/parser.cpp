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

void alloc1(char * &str, int &max_len, int len){
    if(len == max_len){
        max_len*=2;
        char *tmp = (char *) malloc(max_len * sizeof(char));
        memcpy(tmp, str, len);
        free(str);
        str = tmp;
    }
}


void add_char(char **str, int *max_len, int len, char c){
    if(len+1 == *max_len){
        *max_len = *max_len*2;
        *str =(char *) realloc(*str, *max_len * sizeof(char));
    }
    (*str)[len+1] = c;
}



cmd_line parse_line(void) {
    // c1 p1 p2 | c2 p3 p4
    int tokens_count = 0;
    int cmd_count = 1;
    int start = 0;
    int j = 0;
    int max_len = 3;
    int len = 0;
    char **tokens = (char**)malloc(tokens_count * sizeof(char));
    char *token = (char*)malloc( max_len* sizeof(char));
    int c = 0;
    char prev = 0;
    char new_char =0;
    while (true) {
        c = getchar();
        if(c == EOF || c == '\n')
            break;
        switch(c) {
            case ' ':
                token[j] = 0;
                tokens[tokens_count] = token;
                tokens_count++;
                tokens = (char **) realloc(tokens, (tokens_count + 1) * sizeof(char *));
                max_len = 5;
                token = (char *) malloc((max_len) * sizeof(char));
                j = 0;
                prev = (char)c;
                break;
            case '\"':
                prev = 0;
                while(true){
                    c = getchar();
                    if(c == EOF) {
                        break;
                    }
                    if(c == '\n'){
                        printf(">");
                    }
                    if(c != '\"' || prev == '\\'){
                        token[j] = (char)c;
                        j++;
                        alloc1(token, max_len, j);
                        prev = (char)c;
                    }else{
                        token[j] = 0;
                        tokens[tokens_count] = token;
                        tokens_count++;
                        tokens = (char **) realloc(tokens, (tokens_count + 1) * sizeof(char *));
                        max_len = 5;
                        token = (char *) malloc((max_len) * sizeof(char));
                        j = 0;
                        break;
                    }
                }
                break;
            case '>':
                break;
            case '\'':

                while(true){
                    c = getchar();
                    if(c == EOF) {
                        break;
                    }
                    if(c == '\n'){
                        printf(">");
                        continue;
                    }
                    if(c != '\'' || prev == '\\'){
                        token[j] = (char)c;
                        j++;
                        alloc1(token, max_len, j);
                        prev = (char)c;
                    }else{
                        token[j] = 0;
                        tokens[tokens_count] = token;
                        tokens_count++;
                        tokens = (char **) realloc(tokens, (tokens_count + 1) * sizeof(char *));
                        max_len = 5;
                        token = (char *) malloc((max_len) * sizeof(char));
                        j = 0;
                        break;
                    }
                }
                prev = (char)c;
                break;
            case '|':
                cmd_count++;
                token[j] = (char)c;
                j++;
                alloc1(token, max_len, j);
                prev = (char)c;
                break;

            case '\\':
                new_char = (char)getchar();
                if (new_char =='\n'){
                    continue;
                }
                token[j] = (char)c;
                j++;
                alloc1(token, max_len, j);
                prev = new_char;
                break;

            default:
                token[j] = (char)c;
                j++;
                alloc1(token, max_len, j);
                prev = (char)c;
        }

    }
    if (j > 0) {
        token[j] = 0;
        tokens[tokens_count] = token;
        tokens_count++;
    }

    cmd *cmds = (cmd *) malloc(cmd_count * sizeof(cmd));
    cmd command{};
    char **argv = (char **)malloc((tokens_count + 1) * sizeof(char *));
    j = 0;
    int cmd_num = 0;
    for (int i = 0;i< tokens_count; i++){
        printf("t%d - %s\n",i,tokens[i]);

        if(strcmp(tokens[i], "|") == 0){ // lines are equal
            argv[j] = NULL;
            cmds[cmd_num].argv = argv;
            cmds[cmd_num].name = argv[0];
            cmds[cmd_num].argc = j;

            cmd_num++;
            j = 0;
            argv = (char **)malloc((tokens_count + 1 - i)* sizeof(char *));
        }else {
            argv[j] = tokens[i];
            j++;
        }

    }

    argv[j] = NULL;
    cmds[cmd_num].argv = argv;
    cmds[cmd_num].name = argv[0];
    cmds[cmd_num].argc = j;


    cmd_line commands{};
    commands.count = cmd_count;
    commands.commands = cmds;
    return commands;
}
