#ifndef UTIL_H
#define UTIL_H

#include "parser.h"

/* This file contains micelaneous utility functions that cant be provided 
 * by libc */


// trim whitespace in a string
extern char* strtrim(const char*);

// Finds what type of command is contained in the given string
extern enum Command findCommandType(const char*);

// Determines if the current string is a postive number
extern int isNum(const char*);


struct StructCommandArray {
    size_t size;
    size_t capacity;
    ParsedCommand* commands;
};

typedef struct StructCommandArray CommandArray;

extern int CommandArray_create(CommandArray*, size_t);
extern void CommandArray_free(CommandArray*);
extern int CommandArray_copyCommand(CommandArray*, Parser*);

#endif
