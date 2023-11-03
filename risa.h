#ifndef RISA_H
#define RISA_H

/*
 * DEFINES
 */

#define OP_BUFFERMAX 8

/*
 * TYPEDEFS
 */

typedef unsigned char uint8;
typedef unsigned long int uint32;

typedef short int int16;

typedef struct machine {
    int16 A;
    uint8 NEGATIVE_FLAG;
    int16 B;
    uint8 PC;
    uint8 RUNNING;
    int16 MEMORY[0x100];
} Machine;

/*
 * INSTRUCTION PROTOTYPES
 */

// Load A reg with value at [addr]
void LDA (Machine* sys, uint8 addr);

// Store A reg value at [addr]
void STA (Machine* sys, uint8 addr);

// Add B reg to A reg
void ADD (Machine* sys);

// Subtract B reg from A reg
void SUB (Machine* sys);

// Move A reg to B reg
void MBA (Machine* sys);

// Jump to addr (Set PC=addr)
void JMP (Machine* sys, uint8 addr);

// Jump to addr (If NEGATIVE_FLAG set) (Set PC=addr)
void JN  (Machine* sys, uint8 addr);

// Output memory value at [addr]
void OUT (Machine* sys, uint8 addr);

// Halt the machine
void HLT (Machine* sys);

/*
 * FUNCTION PROTOTYPES
 */

// Machine initialization
void initMachine (Machine* sys);

// Fetch next instruction, Decode it, and Execute
void FDE (Machine* sys);

// Load code file into memory (test.s)
void IMPORTCODE (Machine* sys);

// Set PC value from string
void setPC (Machine* sys, char* line);

// Save number into memory from string
void saveNumber (Machine* sys, char* line);

// Read in the OPCODE PARAM format
void readOPFormat (Machine* sys, char* line);

// Convert hex char to uint8
uint8 convHexChar (char A);

/*
 * INCLUDES
 */

#include <stdio.h>
#include "risa.c"

#endif
