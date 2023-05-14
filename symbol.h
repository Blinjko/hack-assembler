#ifndef SYMBOL_H
#define SYMBOL_H

#include <stddef.h>

/* This module contains the symbol table data structure and definitions for the predefined symbols*/

/* Predefined symbols */

#define SYMBOL_SP       0
#define SYMBOL_LCL      1
#define SYMBOL_ARG      2
#define SYMBOL_THIS     3
#define SYMBOL_THAT     4
#define SYMBOL_R0       0
#define SYMBOL_R1       1
#define SYMBOL_R2       2
#define SYMBOL_R3       3
#define SYMBOL_R4       4
#define SYMBOL_R5       5
#define SYMBOL_R6       6
#define SYMBOL_R7       7
#define SYMBOL_R8       8
#define SYMBOL_R9       9
#define SYMBOL_R10      10
#define SYMBOL_R11      11
#define SYMBOL_R12      12
#define SYMBOL_R13      13
#define SYMBOL_R14      14
#define SYMBOL_R15      15
#define SYMBOL_SCREEN   16384
#define SYMBOL_KBD      24576

struct StructTuple {
    char* symbol;
    int   address;
};

struct StructSymbolTable {
    size_t size;           // How much entries are in the table
    size_t capacity;       // How many entries the table can hold
    struct StructTuple* values;
};

typedef struct StructSymbolTable SymbolTable;


extern int  SymbolTable_create      (SymbolTable*, size_t);
extern void SymbolTable_free        (SymbolTable*);
extern int  SymbolTable_addEntry    (SymbolTable*, const char*, int);
extern int  SymbolTable_contains    (SymbolTable*, const char*);
extern int  SymbolTable_getAddress  (SymbolTable*, const char*);

#endif
