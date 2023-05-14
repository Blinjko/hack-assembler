#include "parser.h"
#include "util.h"
#include "code.h"
#include "symbol.h"


#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>


static void logError(int error_num, const char* message);
static int  parseCommands(Parser* parser,
                          SymbolTable* symbol_table,
                          CommandArray* command_array);
static int generateCode(SymbolTable* symbol_table,
                        CommandArray* command_array,
                        FILE*         output_file);

int main() {
    /* Open the input file
     * Open the output file
     * create the parser
     * step through the file, converting instructions as we go
     * write instructions to the output
     * repeat until done */


    // Open the input file
    FILE* source_file = fopen("test.asm", "r");
    if (source_file == NULL) {
        logError(errno, "Failed to open source file");
        return -1;
    }

    // Open the output file
    FILE* output_file = fopen("test.hack", "w");
    if (output_file == NULL) {
        logError(errno, "Failed to open destination file");
        fclose(source_file);
        return -1;
    }


    int error = 0;

    // create the parser
    Parser parser;
    error = Parser_create(&parser, source_file);
    if (error < 0) {
        logError(errno, "Failed to create assembly parser");
        fclose(source_file);
        fclose(output_file);
        return -1;
    }

    // create the command array
    CommandArray command_array;
    error = CommandArray_create(&command_array, 128);
    if (error < 0) {
        logError(errno, "Failed to create the command array");
        Parser_free(&parser);
        fclose(output_file);
        return -1;
    }

    // create the symbol table
    SymbolTable symbol_table;
    error = SymbolTable_create(&symbol_table, 128);
    if (error < 0) {
        logError(errno, "Failed to create symbol table");
        Parser_free(&parser);
        CommandArray_free(&command_array);
        fclose(output_file);
        return -1;
    }

    // Parse the commands and insert labels into the symbol table
    error = parseCommands(&parser,
                          &symbol_table,
                          &command_array);

    if (error < 0) {
        Parser_free(&parser);
        CommandArray_free(&command_array);
        fclose(output_file);
        return -1;
    }

    // Parser isn't needed anymore
    Parser_free(&parser);


    // Generate code fromo the parsed commands
    error = generateCode(&symbol_table, &command_array, output_file);
    if (error < 0) {
        Parser_free(&parser);
        CommandArray_free(&command_array);
        fclose(output_file);
        return -1;
    }

    // Free resources
    CommandArray_free(&command_array);
    SymbolTable_free(&symbol_table);

    error = fflush(output_file);
    if (error != 0) {
            logError(errno, "Failed to flush output to output file");
            fclose(output_file);
            return -1;
    }
    
    fclose(output_file);

    return 0;
}

static void logError(int error_num, const char* message)
{
    fprintf(stderr, "ERROR: %s\nMessage: %s\n", strerror(error_num), message);
}
       
static int  parseCommands(Parser* parser,
                          SymbolTable* symbol_table,
                          CommandArray* command_array)
{
    int error = 0;
    size_t instruction_counter = 0;

    // Parse the commands and generate the symbols
    while (Parser_hasMoreCommands(parser)) {
        error = Parser_advance(parser);

        // error
        if (error < 0) {
            logError(errno, "Failed to parse instruction");
            return -1;
        }

        // no new command read
        else if (error == 1) {
            break;
        }


        /* If its an L command add it to the symbol table
         * Otherwise just add it to the command Array */

        enum Command command_type = Parser_commandType(parser);

        // Add to symbol table
        if (command_type == L_COMMAND) {
            const char* symbol = Parser_symbol(parser);

            // If the symbol isn't in the table, add it
            if (SymbolTable_contains(symbol_table, symbol) == 0) {

                error = SymbolTable_addEntry(symbol_table, symbol, instruction_counter);

                // Failure to add to the table
                if (error < 0) {
                    logError(errno, "Failed to add entry to symbol table");
                    return -1;
                }
            }

            else {
                logError(errno, "Duplicate or invalid symbol found");
                return -1;
            }
        }

        // Add to command array
        else {
            // copy the command
            error = CommandArray_copyCommand(command_array, parser);
            if (error < 0) {
                logError(errno, "Failed to copy command");
                return -1;
            }

            instruction_counter += 1;
        }
    }

    return 0;
}

static int generateCode(SymbolTable* symbol_table,
                        CommandArray* command_array,
                        FILE*         output_file)
{

    /* Iterate through all the parsed commands
     * substitue symbols as needed
     * generate code */
    int error = 0;

    size_t next_variable_address = 16;
    for (size_t index = 0; index < command_array->size; index++) {

        ParsedCommand* current_command = &command_array->commands[index];
        char binary_instruction[18] = "0000000000000000\n\0";

        if (current_command->type == A_COMMAND) {

            // Is a mnuemonic
           if (isMneumonic(current_command->symbol) == 1) {
               error = generateAInstruction(current_command->symbol, &binary_instruction[0]);

               if (error < 0) {
                   logError(errno, "Failed generate A instruction");
                   return -1;
               }
           } 

           // Is a known symbol
           else if (SymbolTable_contains(symbol_table, current_command->symbol) == 1) {
               size_t symbol_address = SymbolTable_getAddress(symbol_table, current_command->symbol);
               char num_str[6];
               sprintf(&num_str[0], "%ld", symbol_address);

               error = generateAInstruction(&num_str[0], &binary_instruction[0]);
               if (error < 0) {
                   logError(errno, "Failed generate A instruction");
                   return -1;
               }
           }

           // Is an unknown symbol, a variable
           else {
               error = SymbolTable_addEntry(symbol_table, current_command->symbol, next_variable_address); 
               if (error < 0) {
                   logError(errno, "Failed to create variable");
                   return -1;
               }

               char num_str[6];
               sprintf(&num_str[0], "%ld", next_variable_address);

               error = generateAInstruction(&num_str[0], &binary_instruction[0]);
               if (error < 0) {
                   logError(errno, "Failed generate A instruction");
                   return -1;
               }

               next_variable_address += 1;
           }
        }

        else if (current_command->type == C_COMMAND) {


            error = generateCInstruction(current_command->destination, current_command->computation, current_command->jump, &binary_instruction[0]);
            if (error < 0) {
                logError(errno, "Failed to generate C instruction");
                return -1;
            }
        }
        // Unknown command
        else {
            if (error < 0) {
                logError(errno, "Unknown command encountered during code generation");
                return -1;
            }
        }

        // By this point we should a valid command to write
        binary_instruction[16] = '\n';
        binary_instruction[17] = '\0';

        size_t bytes_written = fwrite(&binary_instruction[0], sizeof(char), 17, output_file);

        // not enough bytes were written and there was an error
        if (bytes_written != (sizeof(char) * 17) && ferror(output_file) != 0) {
            logError(errno, "Failed to write to output file");
            return -1;
        }
    }
}
