#include "util.h"
#include "parser.h"
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>


/* Trims the whitespace out of a string
 * and returns a newly allocated string that
 * has been trimmed.
 * Return NULL = error, errno will be set
 * Return valid pointer on success. 
 * NOTE if str is just whitespace an empty string will be returned */
extern char* strtrim(const char* str)
{
    if (str != NULL) {
        size_t length = strlen(str);

        char*  trimmed_str = calloc(length + 1, sizeof(char));
        size_t trimmed_str_index = 0;

        // failed memory allocation
        if (trimmed_str == NULL) {
            return NULL;
        }

        for (size_t index = 0; index < length; index++) {

            // if the current character isn't a space add it to the trimmed str
            if (isspace(str[index]) == 0) {

               trimmed_str[trimmed_str_index] = str[index]; 

               trimmed_str_index += 1;
            }
        }

        // the trimmed_str is set to NULL when allocated so
        // no need for a NULL terminator to be placed

        return trimmed_str;
    }

    else {
        errno = EINVAL;
        return NULL;
    }
}


/* Returns the command type if instruction holds a valid command
 * Returns NONE_COMMAND if an error occurred or if the instruction
 * is invalid 
 * NOTE: it is assumed that the instruction has been trimmed and
 * contains no whitespace */
extern enum Command findCommandType(const char* instruction)
{
    /* The function does some simple string checks and can
     * be fooled but truly invalid commands will be caught later */

    if (instruction != NULL) {

        // A commands always start with @
        if (instruction[0] == '@') {
            return A_COMMAND;
        }

        // Labels always start with a '(' and will contain another ')'
        else if (instruction[0] == '(' && strchr(instruction, ')') != NULL) {
            return L_COMMAND;
        }

        else if (strchr(instruction, '=') != NULL || strchr(instruction, ';') != NULL) {
            return C_COMMAND;
        }

        else {
            errno = EINVAL;
            return NONE_COMMAND;
        }
    }

    else {
        errno = EINVAL;
        return NONE_COMMAND;
    }
}


/* Check if a string is a valid number
 * return 1 = yes valid positive number
 * return 0 = not valid number */
extern int isNum(const char* num_str)
{
    if (num_str != NULL) {

        size_t length = strlen(num_str);

        // test if every character is a number
        for (size_t str_index = 0; str_index < length; str_index++) {

            if (isdigit(num_str[str_index]) == 0) {
                return 0;
            }
        }

        // if we get here all characters must be a digit
        return 1;
    }

    else {
        return 0;
    }
}



/* resize the given command array to the new capacity
 * return 0 = success command_array will have the new capacity
 * return -1 on failure, command_array will remain the same
 */
static int CommandArray_resize(CommandArray* command_array, size_t new_capacity)
{
    if (command_array != NULL &&
        command_array->capacity < new_capacity) {

        errno =  0;
        ParsedCommand* new_array = reallocarray(command_array->commands, new_capacity, sizeof(ParsedCommand));

        // error occurred while allocating
        if (errno != 0) {
            return -1;
        }

        // initialize the new entries
        for (size_t index = command_array->size; index < new_capacity; index++) {
            new_array[index].symbol = NULL;
            new_array[index].destination = NULL;
            new_array[index].computation = NULL;
            new_array[index].jump = NULL;
            new_array[index].type = NONE_COMMAND;
        }

        // set the new values in the structure
        command_array->commands = new_array;
        command_array->capacity = new_capacity;

        // Done :)
        return 0;
    }
    else {
        errno = EINVAL;
        return -1;
    }
}

/* Create a dynamic array of parsed commands
 * return 0 on successful creation, will overwrite command_array's values
 * return -1 on error */
extern int CommandArray_create(CommandArray* command_array, size_t capacity)
{
    if (command_array != NULL) {

        // Initialize
        command_array->commands = NULL;
        command_array->size = 0;
        command_array->capacity = 0;

        if (CommandArray_resize(command_array, capacity) == -1) {
            // Failed to make the array
            return -1;
        }

        // Done :)
        return 0;
    }

    else {
        errno = EINVAL;
        return -1;
    }
}

/* Free an allocated CommandArray
 */
extern void CommandArray_free(CommandArray* command_array)
{
    if (command_array != NULL &&
        command_array->commands != NULL &&
        command_array->capacity > 0) {

        // Free all the parsed commands
        for (size_t index = 0; index < command_array->size; index++) {
            ParsedCommand_free(&command_array->commands[index]);
        }

        // free the array
        free(command_array->commands);

        // reset the values
        command_array->commands = NULL;
        command_array->size = 0;
        command_array->capacity = 0;
    }
}

/* Copy the current parsed command from the parser into
 * the commandArray.
 * return 0 on success
 * return -1 on failure */
extern int CommandArray_copyCommand(CommandArray* command_array, Parser* parser)
{

    // How much entries to add if the command array is full
    static const size_t expansion_value = 10;

    if (command_array != NULL &&
        parser != NULL) {

        // Check if theres space
        if (command_array->size == command_array->capacity) {

            if (CommandArray_resize(command_array, command_array->capacity + expansion_value) == -1) {
                // Failed to resize the array
                return -1;
            }
        }

        // the current working ParsedCommand structure
        ParsedCommand* current_command = &command_array->commands[command_array->size];

        // Get the command type
        enum Command command_type = Parser_commandType(parser);

        if (command_type == A_COMMAND || command_type == L_COMMAND) {

            // copy the field
            current_command->symbol = strdup(Parser_symbol(parser));

            // check for memory error
            if (current_command->symbol == NULL) {
                return -1;
            }
        }

        else if (command_type == C_COMMAND) {

            // copy the computation field, it will always be filled out 
            current_command->computation = strdup(Parser_comp(parser));

            // check for memory error
            if (current_command->computation == NULL) {
                return -1;
            }

            // Copy the jump field if it exists
            if (Parser_jump(parser) != NULL) {

                current_command->jump = strdup(Parser_jump(parser));

                // check for memory error
                if (current_command->jump == NULL) {
                    free(current_command->computation);
                    current_command->computation = NULL;
                    return -1;
                }
            }

            // Copy the destination field exists, the dest and jump fields should never exist together
            else if (Parser_dest(parser) != NULL) {

                current_command->destination = strdup(Parser_dest(parser));

                // check for memory error
                if (current_command->destination == NULL) {
                    free(current_command->computation);
                    current_command->computation = NULL;
                    return -1;
                }
            }

        }

        // Unknown command type
        else {
            return -1;
        }

        current_command->type = command_type;

        command_array->size += 1;

        // Successful copy :)
        return 0;

    }

    else {
        errno = EINVAL;
        return -1;
    }
}
