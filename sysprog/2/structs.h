//
// Created by lexa on 31.03.19.
//
#include <stdio.h>

#ifndef INC_2_STRUCTS_H
#define INC_2_STRUCTS_H
struct cmd {
    const char *name;
    const char **argv;
    int argc;
};

struct cmd_line {
    int count;
    cmd *commands;
};

#endif //INC_2_STRUCTS_H
