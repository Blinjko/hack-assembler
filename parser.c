#include "parser.h"
#include "util.h"
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>




/* Free any allocated pointers in the ParsedCommand structure */
extern void ParsedCommand_free(struct ParsedCommand* parsed_command)
{

    if (parsed_command != NULL) {

        if (parsed_command->symbol != NULL) {
            free(parsed_command->symbol);
            parsed_command->symbol = NULL;
        }

        if (parsed_command->destination != NULL) {
            free(parsed_command->destination);
            parsed_command->destination = NULL;
        }

        if (parsed_command->computation != NULL) {
            free(parsed_command->computation);
            parsed_command->computation = NULL;
        }

        if (parsed_command->jump != NULL) {
            free(parsed_command->jump);
            parsed_command->jump = NULL;
        }

        parsed_command->type = NONE_COMMAND;
    }
}




/* Creates a parser structure
 * Expects the given parser to be unitialized & not NULL
 * Return 0 on success, parser will also be allocated
 * Return -1 on failure and set errno
 */
extern int Parser_create(Parser* parser, FILE* file)
{
    if (parser != NULL &&
        file   != NULL) {
        
        parser->source_file = file;
        parser->current_command.symbol = NULL;
        parser->current_command.destination = NULL;
        parser->current_command.computation = NULL;
        parser->current_command.jump = NULL;
        parser->current_command.type = NONE_COMMAND;

        return 0;
    }

    else {
        errno = EINVAL;
        return -1;
    }
}

/* Free any memory allocated within the parser structure
 * and close the file */
extern void Parser_free(Parser* parser)
{
    if (parser != NULL && parser->source_file != NULL) {

        // close the file
        fclose(parser->source_file);
        parser->source_file = NULL;

        // free any remaining memory in the ParsedCommand structure

        ParsedCommand_free(&parser->current_command);
    }
}

/* Returns 1 if there are more commands
 * Returns 0 if there is not more commands
 * Returns -1 on error, errno will be set */
extern int Parser_hasMoreCommands(Parser* parser)
{
    if (parser != NULL) {

        if (feof(parser->source_file) == 0) {
            return 1;
        }

        else {
            return 0;
        }
    }

    else {
        errno = EINVAL;
        return -1;
    }
}


/* Internal function used to the heavy lifting of Parsing
 * Expects the given command to be trimmed and parser->current_command.type
 * to be assigned a valid value. Will disect the command into its various
 * fields and store them in the parser->current_command structure, note it will override the current values.
 * Pointers are dynamically allocated and freed as needed.
 * Return 0 on success
 * Return -1 on failure, errno will be set */
static int Parser_parseCommand(Parser* parser, char* command)
{
    if (parser != NULL &&
        command != NULL) {

        // Free the current parsed command to avoid leaks
        ParsedCommand_free(&parser->current_command);

        // Determine the command type
        parser->current_command.type = findCommandType(command);

        if (parser->current_command.type == A_COMMAND) {

           char* token = strtok(command, "@");

           // occurs if it command is malformed
           if (token == NULL) {
               errno = EINVAL;
               return -1;
           }

           // copy the string into the datastructure
           parser->current_command.symbol = strdup(token);
           if (parser->current_command.symbol == NULL) {
               return -1;
           }
        }

        else if (parser->current_command.type == C_COMMAND) {

            // Normal C command
            char* c_dest_token = strtok(command, "=");
            char* c_comp_token = strtok(NULL, "=");

            // Jump C command
            char* j_comp_token = strtok(command, ";");
            char* j_jump_token = strtok(NULL, ";");



            // Both cant be fufilled so its an error
            if ((c_dest_token != NULL && c_comp_token != NULL) &&
                (j_comp_token != NULL && j_jump_token != NULL)) {
                errno = EINVAL;
                return -1;
            }

            // Normal C command
            else if (c_dest_token != NULL && c_comp_token != NULL) {

                // copy the tokens
                parser->current_command.destination = strdup(c_dest_token);
                parser->current_command.computation = strdup(c_comp_token);

                // check for allocation errors, we dont care if we leave an allocated pointer
                // because the main structure will be freed upon error, thus freeing it anyway
                if (parser->current_command.destination == NULL ||
                    parser->current_command.computation == NULL) {
                    return -1;
                }
            }

            // Jump C command
            else if (j_comp_token != NULL && j_jump_token != NULL) {

                // copy the tokens
                parser->current_command.computation = strdup(j_comp_token);
                parser->current_command.jump        = strdup(j_jump_token);

                if (parser->current_command.computation == NULL ||
                    parser->current_command.jump == NULL) {
                    return -1;
                }
            }

            // Malformed command
            else {
                errno = EINVAL;
                return -1;
        }

        }

        // L Command
        else if (parser->current_command.type == L_COMMAND) {

            char* symbol_token = strtok(command, "()");
            parser->current_command.symbol = strdup(symbol_token);

            if (parser->current_command.symbol == NULL) {
                return -1;
            }
        }

        // non valid command
        else {
            errno = EINVAL;
            return -1;
        }

        // Success :)
        return 0;

    }

    else {
        errno = EINVAL;
        return -1;
    }
}

/* if Parser_hasMoreCommands returns 1 then this function will
 * return a non error value.
 * Return 0 = read a new command
 * Return 1 = EOF reached and no new command read, this can happen if there is tailing whitespace
 * Return -1 = Error, errno will be set
 */
extern int Parser_advance(Parser* parser) 
{
    if (parser != NULL) {
        /* Read line from the file
         * If EOF is reached return 1
         *
         * if not EOF
         * Trim whitespace
         *
         * If string was empty after trimming 
         * read a new line ( step 0 )
         *
         * If string was not empty
         * parse it into respective fields
         *
         */

        size_t buffer_size = 129; // Hopefully there are no lines over 128 charaters 0_0
        char* buffer = calloc(buffer_size, sizeof(char));

        if (buffer == NULL) {
            return -1;
        }

        while(1) {

            if (fgets(buffer, buffer_size - 1, parser->source_file) == NULL) {

                // Error occured while reading file
                if (feof(parser->source_file) == 0) {
                    free(buffer);
                    return -1;
                }

                // Eof
                else {
                    free(buffer);
                    return 1;
                }
            }

            char* trimmed_command = strtrim(buffer);
            if (trimmed_command == NULL) {
                free(buffer);
                return -1;
            }
            else if (trimmed_command[0] == '\0') {
                free(trimmed_command);
            }

            // valid command
            else {
                free(buffer);
                buffer = trimmed_command;
                break;
            }
        }

        // Parse the command
        if (Parser_parseCommand(parser, buffer) < 0) {

            free(buffer); // Not a double free
            return -1;
        }

        free(buffer);

        // Success
        return 0;

    }

    else {
        errno = EINVAL;
        return -1;
    }
}

/* get the current command type
 * return NONE_COMMAND if parser is NULL */
extern enum Command Parser_commandType(Parser* parser)
{
    if (parser != NULL) {
        return parser->current_command.type;
    }

    else {
        errno = EINVAL;
        return NONE_COMMAND;
    }
}



/* Getter functions for the various fields of the current command
 * Return NULL and set errno if the requested field isn't filled
 * or if parser is NULL */
extern const char* Parser_symbol(Parser* parser)
{
    if (parser != NULL &&
        parser->current_command.symbol != NULL) {

        return parser->current_command.symbol;
    }

    else {
        errno = EINVAL;
        return NULL;
    }
}

extern const char* Parser_dest(Parser* parser)
{
    if (parser != NULL &&
        parser->current_command.destination != NULL) {

        return parser->current_command.destination;
    }

    else {
        errno = EINVAL;
        return NULL;
    }
}
extern const char* Parser_comp(Parser* parser)
{
    if (parser != NULL &&
        parser->current_command.computation != NULL) {

        return parser->current_command.computation;
    }

    else {
        errno = EINVAL;
        return NULL;
    }
}

extern const char* Parser_jump(Parser* parser)
{
    if (parser != NULL &&
        parser->current_command.jump != NULL) {

        return parser->current_command.jump;
    }

    else {
        errno = EINVAL;
        return NULL;
    }
}
