#include "symbol.h"
#include <stdlib.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>

// Mapping of the predefined symbols and their values
static const char* const PREDEFINED_SYMBOLS[23] = { 
    "SP",  "LCL", "ARG", "THIS", "THAT", "R0",     "R1", "R2", "R3", "R4", "R5", "R6", "R7", "R8", "R9", "R10",
    "R11", "R12", "R13", "R14",  "R15",  "SCREEN", "KBD" };

static const int PREDEFINED_SYMBOL_VALUES[23] = {
    SYMBOL_SP,  SYMBOL_LCL, SYMBOL_ARG, SYMBOL_THIS,   SYMBOL_THAT, SYMBOL_R0, SYMBOL_R1,  SYMBOL_R2,  SYMBOL_R3,
    SYMBOL_R4,  SYMBOL_R5,  SYMBOL_R6,  SYMBOL_R7,     SYMBOL_R8,   SYMBOL_R9, SYMBOL_R10, SYMBOL_R11, SYMBOL_R12, 
    SYMBOL_R13, SYMBOL_R14, SYMBOL_R15, SYMBOL_SCREEN, SYMBOL_KBD };

static const size_t TOTAL_PREDEFINED_SYMBOLS = 23;

/* Locally needed function(s) */


/* Add the predefined symbols to the given symbol table, it assumes
 * they don't already exist within the table
 * Return 0 on success
 * return -1 on failure */
static int SymbolTable_addPredefinedSymbols(SymbolTable* st)
{
    if (st != NULL) {

        // Add all the predefined symbols
        for (size_t index = 0; index < TOTAL_PREDEFINED_SYMBOLS; index++) {

            if (SymbolTable_addEntry(st, PREDEFINED_SYMBOLS[index], PREDEFINED_SYMBOL_VALUES[index]) == -1) {
                return -1;
            }
        }

        // Done :)
        return 0;
    }

    else {
        errno = EINVAL;
        return -1;
    }
}

/* Function used to resize the array of Symbol Address tuples
 * Return 0 on success
 * Return -1 on error, st is left untouched */
static int SymbolTable_resize(SymbolTable* st, size_t new_size)
{
    if (st != NULL &&
        st->capacity < new_size)
    {
        errno = 0;
        struct StructTuple* new_array = reallocarray(st->values, new_size, sizeof(struct StructTuple));
        // Error occurred
        if (errno != 0) {
            return -1;
        }

        // initialize the values of the new nodes
        for (size_t index = st->capacity; index < new_size; index++) {
            new_array[index].symbol = NULL;
            new_array[index].address = 0;
        }

        // update the values
        st->capacity = new_size;
        st->values = new_array;

        // Done :)
        return 0;
    }

    else {
        errno = EINVAL;
        return -1;
    }
}

/* Linearly search through the symbol table for a matching symbol
 * if one is found return the index of it
 * if it does't exist return -1 and errno will be 0
 * if an error occurred return -1 and errno will be set */
static ssize_t SymbolTable_getValueIndex(SymbolTable* st, const char* symbol)
{
    if (st != NULL &&
        symbol != NULL) {

        for (size_t index = 0; index < st->size; index++) {

            // check if we have a match
            if (st->values[index].symbol != NULL &&
                strcmp(st->values[index].symbol, symbol) == 0) {

                // match found return its place
                return index;
            }
        }

        // No value found
        errno = 0;
        return -1;
    }

    else {
        errno = EINVAL;
        return -1;
    }
}


/* Create / Initialze A symbol table
 * If given an already initialized symbol table memory leaks will occurr
 * this function assumes that the given table is not allocated.
 * capacity is the initial size of the symbol table
 * Return 0 on success, st will also be populated
 * Return -1 on failure, errno will be set */
extern int SymbolTable_create(SymbolTable* st, size_t capacity)
{
    if (st != NULL &&
        capacity > 0) {

        // Initialize the values
        st->size = 0;
        st->capacity = 0;
        st->values = NULL;

        // Allocate the table
        int error = SymbolTable_resize(st, capacity + TOTAL_PREDEFINED_SYMBOLS);

        // Failed to allocate the table
        if (error < 0) {
            return -1;
        }

        // Add the predefined symbols
        error = SymbolTable_addPredefinedSymbols(st);

        // Failed to add the predefined symbols
        if (error < 0) {
            SymbolTable_free(st);
            return -1;
        }

        return 0;
    }

    else {
        errno = EINVAL;
        return -1;
    }
}

/* Free an allocated symbol table */
extern void SymbolTable_free(SymbolTable* st)
{
    if (st != NULL) {

        // Free all memory alloacted within the tuples
        for (size_t index = 0; index < st->size; index++) {
            
            if (st->values[index].symbol != NULL) {
                free(st->values[index].symbol);
            }
        }

        // Free the tuple array
        free(st->values);

        // Clear the values
        st->size = 0;
        st->capacity = 0;
        st->values = NULL;
    }
}

/* Add an entry to the symbol table
 * If the entry already exists, it will be updated
 * if the entry doesn't exist it will be added
 * the symbol table is resized as needed
 * Return 0 on success
 * Return -1 on error, set errno */
extern int SymbolTable_addEntry(SymbolTable* st, const char* symbol, int address)
{
    // A constant to dictate how much the symbol table
    // should grow on a resize
    static const size_t expansion_value = 10;
    if (st != NULL &&
        address < 32766 && // max value a symbol could have given the ram size, 2^15 - 1
        symbol != NULL)
    {
        ssize_t error = 0;

        // Check if the symbol already exists
        error = SymbolTable_getValueIndex(st, symbol);

        // Value doesn't exist already
        if (error == -1 && errno == 0) {

            // If the table is full resize
            if (st->capacity == st->size) {
                error = SymbolTable_resize(st, st->capacity + expansion_value);

                // Failed to resize
                if (error < 0) {
                    return -1;
                }
            }

            // Copy the symbol string
            st->values[st->size].symbol = strdup(symbol);

            // Allocation error
            if(st->values[st->size].symbol == NULL) {
                return -1;
            }

            // Insert the address
            st->values[st->size].address = address;

            // Add one to the size
            st->size += 1;
        }

        // Value exists already and is in range
        else if (error > 0 && error < st->size) {

            // Assign the new value
            st->values[error].address = address;
        }

        // Error occurred
        else {
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

/* Check if the symbol table contains the given symbol
 * return 1 if it does
 * return 0 if it doesn't
 * return -1 on error, errno will be set */
extern int SymbolTable_contains (SymbolTable* st, const char* symbol)
{
    if (st != NULL &&
        symbol != NULL) {

        // search for the value in the table
        ssize_t index = SymbolTable_getValueIndex(st, symbol);

        // Value exists
        if (index > 0) {
            return 1;
        }

        // value doesn't exist
        else if (index == -1 && errno == 0) {
            return 0;
        }

        // Error occurred while searching
        else {
            return -1;
        }
    }

    else {
        errno = EINVAL;
        return -1;
    }
}

/* Function to get the associated address for the given
 * symbol.
 * return the address on success
 * return -1 if the requested symbol doesn't exist
 * return -1 and set errno if an error occured */
extern int SymbolTable_getAddress(SymbolTable* st, const char* symbol)
{
    if (st != NULL &&
        st->size > 0 &&
        symbol != NULL) {

        ssize_t index = SymbolTable_getValueIndex(st, symbol);

        // Value exists and is in range
        if (index > 0 && index < st->size) {
            return st->values[index].address;
        }

        // value doesn't exist
        else if (index == -1 && errno == 0) {
            errno = 0;
            return -1;
        }

        // Error occurred while searching
        else {
            // Errno will be set in this case
            return -1;
        }

    }

    else {
        errno = EINVAL;
        return -1;
    }
}
