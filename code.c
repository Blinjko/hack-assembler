#include "code.h"
#include "util.h"
#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>


/* Constants */

/* Computation mneumonic array mappings to simplify translation */
static const size_t TOTAL_COMPUTATIONS = 8 + 10 + 10; // 8 that dont act on A or M, 10 that Act on A, and 10 that Act on M
static const char* const COMPUTATION_MNEUMONICS[28] = 
{ "0", "1",  "-1", "D",   "!D",  "-D",  "D+1", "D-1",                        // Computations that Dont act on A or M
  "A", "!A", "-A", "A+1", "A-1", "D+A", "D-A", "A-D", "D&A", "D|A",          // Computations that Act on A
  "M", "!M", "-M", "M+1", "M-1", "D+M", "D-M", "M-D", "D&M", "D|M" };        // Computations that Act on M

const char* const COMPUTATION_BINARY[28] =
{ COMP_0, COMP_1,     COMP_NEG_1, COMP_D,        COMP_NOT_D,     COMP_NEG_D,    COMP_D_PLUS_1,  COMP_D_MINUS_1,                              // Computations that dont act on A or M
  COMP_A, COMP_NOT_A, COMP_NEG_A, COMP_A_PLUS_1, COMP_A_MINUS_1, COMP_D_PLUS_A, COMP_D_MINUS_A, COMP_A_MINUS_D, COMP_D_AND_A, COMP_D_OR_A,   // Computations that act on A
  COMP_M, COMP_NOT_M, COMP_NEG_M, COMP_M_PLUS_1, COMP_M_MINUS_1, COMP_D_PLUS_M, COMP_D_MINUS_M, COMP_M_MINUS_D, COMP_D_AND_M, COMP_D_OR_M }; // Computations that act on M



/* Destination field array mappings, this makes it easier to translate */
static const size_t TOTAL_DESTINATIONS = 8;
static const char* const DESTINATION_MNEUMONICS[8] = { "M",    "D",    "MD",    "A",    "AM",    "AD",    "AMD",    "" };
static const char* const DESTINATION_BINARY[8] =     { DEST_M, DEST_D, DEST_MD, DEST_A, DEST_AM, DEST_AD, DEST_AMD, DEST_NULL };

/* Jump field array mappings, this makes it easier to translate */
static const size_t TOTAL_JUMPS = 8;
static const char* const JUMP_MNEUMONICS[8] = { "JGT",    "JEQ",    "JGE",    "JLT",    "JNE",    "JLE",    "JMP",    "" };
static const char* const JUMP_BINARY[8] =     { JUMP_JGT, JUMP_JEQ, JUMP_JGE, JUMP_JLT, JUMP_JNE, JUMP_JLE, JUMP_JMP, JUMP_NULL };


/* Determine if the given string is a known mneumonic
 * Return 1 = yes
 * Return 0 = no
 * Note if passed a NULL ptr 0 will be retured */
extern int isMneumonic(const char* mneumonic)
{
    if (mneumonic != NULL) {

        // Check all the Computation mneumonics
        for (size_t index = 0; index < TOTAL_COMPUTATIONS; index++) {

            if (strcmp(mneumonic, COMPUTATION_MNEUMONICS[index]) == 0) {
                return 1;
            }
        }

        // Check all the destination mneumonics
        for (size_t index = 0; index < TOTAL_DESTINATIONS; index++) {

            if (strcmp(mneumonic, DESTINATION_MNEUMONICS[index]) == 0) {
                return 1;
            }
        }

        // Check all the jump mneumonics
        for (size_t index = 0; index < TOTAL_JUMPS; index++) {

            if (strcmp(mneumonic, JUMP_MNEUMONICS[index]) == 0) {
                return 1;
            }
        }

        // No matches found
        return 0;
    }

    else {
        return 0;
    }
}


/* Translate the given destination mneumonic into binary
 * and outputs it into binary_out. binary_out is assumed to
 * have a length >= 4
 * Return 0 on success, binary_out will be written to
 * Return -1 on error, set errno, binary_out will be untouched
 */
static int  dest(const char* mneumonic, char* binary_out)
{
    if (mneumonic != NULL &&
        binary_out != NULL) {

        // Loop through the mneumonics and try to find a match
        for (int mneumonic_index = 0; mneumonic_index < TOTAL_DESTINATIONS; mneumonic_index++) {

            // If we find a match get its binary counterpart
            if (strcmp(mneumonic, DESTINATION_MNEUMONICS[mneumonic_index]) == 0) {

                // Copy into the buffer
                strcpy(binary_out, DESTINATION_BINARY[mneumonic_index]);

                // Yay :)
                return 0;
            }
        }

        // Couldn't find a valid mnuemonic, so return an error
        errno = EINVAL;
        return -1;
    }

    else {
        errno = EINVAL;
        return -1;
    }
}

/* Translate the given computation mneumonic into binary
 * and outputs it into binary_out. binary_out is assumed to
 * have a length >= 8
 * Return 0 on success, binary_out will be written to
 * Return -1 on error, set errno, binary_out will be untouched
 */
static int comp(const char* mneumonic, char* binary_out)
{
    if (mneumonic != NULL &&
        binary_out != NULL) {

        // Loop through the mneumonics and try to find a match
        for (int mneumonic_index = 0; mneumonic_index < TOTAL_COMPUTATIONS; mneumonic_index++) {

            // If we find a match get its binary counterpart
            if (strcmp(mneumonic, COMPUTATION_MNEUMONICS[mneumonic_index]) == 0) {

                // Copy into the buffer
                strcpy(binary_out, COMPUTATION_BINARY[mneumonic_index]);

                // Yay :)
                return 0;
            }
        }

        // Couldn't find a valid mnuemonic, so return an error
        errno = EINVAL;
        return -1;
    }

    else {
        errno = EINVAL;
        return -1;
    }
}

/* Translate the given jump mneumonic into binary
 * and outputs it into binary_out. binary_out is assumed to
 * have a length >= 4
 * Return 0 on success, binary_out will be written to
 * Return -1 on error, set errno, binary_out will be untouched
 */
static int jump(const char* mneumonic, char* binary_out)
{
    if (mneumonic != NULL &&
        binary_out != NULL) {

        // Loop through the mneumonics and try to find a match
        for (int mneumonic_index = 0; mneumonic_index < TOTAL_JUMPS; mneumonic_index++) {

            // If we find a match get its binary counterpart
            if (strcmp(mneumonic, JUMP_MNEUMONICS[mneumonic_index]) == 0) {

                // Copy into the buffer
                strcpy(binary_out, JUMP_BINARY[mneumonic_index]);

                // Yay :)
                return 0;
            }
        }

        // Couldn't find a valid mnuemonic, so return an error
        errno = EINVAL;
        return -1;
    }

    else {
        errno = EINVAL;
        return -1;
    }
}


/* Convert a numeric string to a binary string
 * binary_out is assumed to be pre-allocated and its
 * length >= 16, 15 binary digits, 1 null terminator
 * the max decimal number that can be translated is 32767, (2^15) - 1
 * any number higher than this will result in an error.
 * unsigned numbers only, no negatives
 * Return 0 on success and binary_out will have the bits written to it
 * Return -1 on error, binary_out is untouched */
static int numToBinary(const char* num_str, char* binary_out)
{
    if (num_str != NULL &&
        binary_out != NULL &&
        isNum(num_str) == 1) {

        const uint16_t max_value = 32767;
        uint16_t decimal_num = (uint16_t) atoi(num_str);

        // Overflow protection
        if (decimal_num > max_value) {
            errno = EINVAL;
            return -1;
        }

        // translate to binary
        for (int binary_index = 14; binary_index >= 0; binary_index--) {

            char binary_digit = ((decimal_num >> binary_index) & 1) ? '1' : '0';

            binary_out[14 - binary_index] = binary_digit;
        }

        // set the null terminator
        binary_out[15] = '\0';

        // Success :)
        return 0;
    }

    else {
        errno = EINVAL;
        return -1;
    }
}

/* generate an A instruction given the symbol
 * binary_out is assumed to have a length >= 17
 * 16 binary characters, 1 null terminator
 * Return 0 on success, and binary_out will be filled
 * Return -1 on failure, set errno */
extern int generateAInstruction(const char* symbol, char* binary_out)
{
    if (symbol != NULL &&
        binary_out != NULL) {

        int error = 0;

        // convert the symbol to a number
        error = numToBinary(symbol, binary_out + 1);
        if (error < 0) {
            return -1;
        }

        // set the first bit to 0 to indicate its an A instruction
        binary_out[0] = '0';

        binary_out[16] = '\0';

        // Done :)
        return 0;
    }

    else {
        errno = EINVAL;
        return -1;
    }
}


/* generate an C instruction given appropriate parameters
 * binary_out is assumed to have a length >= 17
 * 16 binary characters, 1 null terminator
 * Return 0 on success, and binary_out will be filled
 * Return -1 on failure, set errno */
extern int generateCInstruction(const char* destination, const char* computation, const char* jmp, char* binary_out)
{
    if (computation != NULL &&
        (destination != NULL || jmp != NULL) &&
        binary_out != NULL) {

        // NULL dest means a jump
        // NULL jump means a regular C instruction
        // Maybe they both can be set, ill design it in a way so it will still work
        
        int error = 0;

        // fill the beginning field to indicate its a C instruction
        binary_out[0] = '1';
        binary_out[1] = '1';
        binary_out[2] = '1';

        // computation field starts 3 bits in
        error = comp(computation, binary_out + 3);
        if (error < 0) {
            return -1;
        }

        // destination field starts 10 bits in
        if (destination != NULL) {
            error = dest(destination, binary_out + 10);
            if (error < 0) {
                return -1;
            }
        }
        else {
            strcpy(binary_out + 10, DEST_NULL);
        }


        // jump field starts 13 bits in
        if (jmp != NULL) {
            error = jump(jmp, binary_out + 13);
            if (error < 0) {
                return -1;
            }
        }
        else {
            strcpy(binary_out + 13, JUMP_NULL);
        }

        binary_out[16] = '\0';

        // Done :)
        return 0;
    }

    else {
        errno = EINVAL;
        return -1;
    }
}
