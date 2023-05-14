#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>

/* Parser header */

/* command type enum */

enum Command {
    A_COMMAND,
    C_COMMAND,
    L_COMMAND,
    NONE_COMMAND
};

struct ParsedCommand {
    char* symbol;
    char* destination;
    char* computation;
    char* jump;
    enum Command type;
};

typedef struct ParsedCommand ParsedCommand;

extern void ParsedCommand_free(struct ParsedCommand*);

struct ParserStruct {
    FILE* source_file;
    struct ParsedCommand current_command;
};

typedef struct ParserStruct Parser;

extern int             Parser_create(Parser*, FILE*);
extern void            Parser_free(Parser*);
extern int             Parser_hasMoreCommands(Parser*); 
extern int             Parser_advance(Parser*);
extern enum Command    Parser_commandType(Parser*);
extern const char*     Parser_symbol(Parser*);
extern const char*     Parser_dest(Parser*);
extern const char*     Parser_comp(Parser*);
extern const char*     Parser_jump(Parser*);


#endif
