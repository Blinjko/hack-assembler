#ifndef CODE_H
#define CODE_H

#include <stddef.h>
/* This module holds the functions and declarations needed to translate mneumonics to binary code */

/* These computations don't act on A or M */
#define COMP_0          "0101010"
#define COMP_1          "0111111"
#define COMP_NEG_1      "0111010"
#define COMP_D          "0001100"
#define COMP_NOT_D      "0001101"
#define COMP_NEG_D      "0001111"
#define COMP_D_PLUS_1   "0011111"
#define COMP_D_MINUS_1  "0001110"

/* These computations Act on A */
#define COMP_A          "0110000"
#define COMP_NOT_A      "0110001"
#define COMP_NEG_A      "0110011"
#define COMP_A_PLUS_1   "0110111"
#define COMP_A_MINUS_1  "0110010"
#define COMP_D_PLUS_A   "0000010"
#define COMP_D_MINUS_A  "0010011"
#define COMP_A_MINUS_D  "0000111"
#define COMP_D_AND_A    "0000000"
#define COMP_D_OR_A     "0010101"


/* These computations Act on M */
#define COMP_M          "1110000"
#define COMP_NOT_M      "1110001"
#define COMP_NEG_M      "1110011"
#define COMP_M_PLUS_1   "1110111"
#define COMP_M_MINUS_1  "1110010"
#define COMP_D_PLUS_M   "1000010"
#define COMP_D_MINUS_M  "1010011"
#define COMP_M_MINUS_D  "1000111"
#define COMP_D_AND_M    "1000000"
#define COMP_D_OR_M     "1010101"



/* Destination field binary translations */
#define DEST_NULL       "000"
#define DEST_M          "001"
#define DEST_D          "010"
#define DEST_MD         "011"
#define DEST_A          "100"
#define DEST_AM         "101"
#define DEST_AD         "110"
#define DEST_AMD        "111"


/* Jump field binary translations */
#define JUMP_NULL       "000"
#define JUMP_JGT        "001"
#define JUMP_JEQ        "010"
#define JUMP_JGE        "011"
#define JUMP_JLT        "100"
#define JUMP_JNE        "101"
#define JUMP_JLE        "110"
#define JUMP_JMP        "111"



/* For reference
static int  dest(const char*, char*);
static int  comp(const char*, char*);
static int  jump(const char*, char*);
static int  numToBinary(const char*, char*);
*/
extern int generateAInstruction(const char*, char*);
extern int generateCInstruction(const char*, const char*, const char*, char*);
extern int isMneumonic(const char*);

#endif
