#include <stdio.h>
#include <cstdlib>

#include <string.h>
#include "parser.cpp"


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
    const char *exit = "exit";
    for(;;){
        char *line = get_cmd();
        if (strcmp(line, exit) == 0){
            break;
        }
        cmd_line cmds = parse_line(line);
        for(int i = 0; i < cmds.count; i++) {
            printf("your cmd name is: %s \n", cmds.commands[i].name);
            for(int j = 0; j < cmds.commands[i].argc; j++){
                printf("\t param%d is: %s\n", j, cmds.commands[i].argv[j]);
            }
        }
    }

    printf("\nBye!");
    return 0;
}