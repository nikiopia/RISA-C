void initMachine (Machine* sys) {
    sys->A = 0;
    sys->B = 0;
    sys->PC = 0;
    sys->RUNNING = 1;
    sys->NEGATIVE_FLAG = 0;

    for (unsigned short int i = 0; i < 0x100; i++) {
        sys->MEMORY[i] = 0x800;
    }
}

void LDA (Machine* sys, uint8 addr) {
    sys->A = sys->MEMORY[addr];
    sys->NEGATIVE_FLAG = sys->A < 0;
}

void STA (Machine* sys, uint8 addr) {
    sys->MEMORY[addr] = sys->A;
}

void ADD (Machine* sys) {
    sys->A = sys->A + sys->B;
    sys->NEGATIVE_FLAG = sys->A < 0;
}

void SUB (Machine* sys) {
    sys->A = sys->A - sys->B;
    sys->NEGATIVE_FLAG = sys->A < 0;
}

void MBA (Machine* sys) {
    sys->B = sys->A;
}

void JMP (Machine* sys, uint8 addr) {
    sys->PC = addr;
}

void JN (Machine* sys, uint8 addr) {
    if (sys->NEGATIVE_FLAG) {
        sys->PC = addr;
        return;
    }

    sys->PC++;
}

void OUT (Machine* sys, uint8 addr) {
    printf("%02X: %04X / %u\n", addr, sys->MEMORY[addr],
        sys->MEMORY[addr]);
}

void HLT (Machine* sys) {
    sys->RUNNING = 0;
}

void FDE (Machine* sys) {
    // FETCH, DECODE, EXECUTE
    uint8 OPCODE = *((uint8*)(&(sys->MEMORY[sys->PC])) + 1);
    uint8 PARAM = *(uint8*)(&(sys->MEMORY[sys->PC]));

    switch (OPCODE) {
        case 0:
            LDA(sys, PARAM);
            sys->PC++;
            break;
        case 1:
            STA(sys, PARAM);
            sys->PC++;
            break;
        case 2:
            ADD(sys);
            sys->PC++;
            break;
        case 3:
            SUB(sys);
            sys->PC++;
            break;
        case 4:
            MBA(sys);
            sys->PC++;
            break;
        case 5:
            JMP(sys, PARAM);
            break;
        case 6:
            JN(sys, PARAM);
            break;
        case 7:
            OUT(sys, PARAM);
            sys->PC++;
            break;
        default:
            HLT(sys);
            break;
    }
}

void readOPFormat (Machine* sys, char* line) {
    if (!line) {
        sys->RUNNING = 0;
        printf("readOPFormat: 'line' is nullptr\n\n");
        return;
    }

    // OPCODE and PARAM buffers
    char opcode[OP_BUFFERMAX];
    char param[OP_BUFFERMAX];
    // Empty the buffers
    for (uint8 j = 0; j < OP_BUFFERMAX; ++j) {
        opcode[j] = '\0';
        param[j] = '\0';
    }

    char* curr = line;
    uint8 i = 0;

    /*
     * FETCH
     */

    // Read OPCODE into buffer
    while (*curr != '\0' && *curr != ' ' && *curr != '\x0A' &&
        i < (OP_BUFFERMAX - 1)) {
        
        opcode[i] = *curr;
        curr++;
        i++;
    }
    opcode[i] = '\0';

    // Skip middle spaces
    while (*curr == ' ' && i < (OP_BUFFERMAX - 1)) {
        curr++;
        i++;
    }

    // Read PARAM into buffer
    uint8 offset = i;
    while (*curr != '\0' && *curr != ' ' && *curr != '\x0A' &&
        i < (OP_BUFFERMAX + offset - 1)) {
        
        param[i - offset] = *curr;
        curr++;
        i++;
    }
    param[i - offset] = '\0';

    /*
     * DECODE
     */

    // Count OPCODE chars
    i = 0;
    while (opcode[i] != '\0' && i < 4) {
        i++;
    }

    if (i == 0) {
        // No opcode, error
        sys->RUNNING = 0;
        printf("readOPFormat: no opcode\n");
        printf("Line: '%s'\n\n", line);
        return;
    }

    // Build opcode as a number for quick comparison
    uint32 opcode_32bit = 0;
    uint8* baseAddr = (uint8*)(&opcode_32bit);
    for (uint8 a = 0; a < i; ++a) {
        *(baseAddr + 3 - a) = opcode[a];
    }

    uint8 opcode_byte = 0;
    uint8 param_required = 1;
    switch (opcode_32bit) {
        case 0x4C444100:
            opcode_byte = 0;
            break;
        case 0x53544100:
            opcode_byte = 1;
            break;
        case 0x41444400:
            opcode_byte = 2;
            param_required = 0;
            break;
        case 0x53554200:
            opcode_byte = 3;
            param_required = 0;
            break;
        case 0x4D424100:
            opcode_byte = 4;
            param_required = 0;
            break;
        case 0x4A4D5000:
            opcode_byte = 5;
            break;
        case 0x4A4E0000:
            opcode_byte = 6;
            break;
        case 0x4F555400:
            opcode_byte = 7;
            break;
        case 0x484C5400:
            opcode_byte = 8;
            param_required = 0;
            break;
        default:
            sys->RUNNING = 0;
            break;
    }

    if (!(sys->RUNNING)) {
        printf("readOPFormat: unknown opcode\n");
        printf("Line: '%s'\n\n", line);
        return;
    }

    // Count param chars
    i = 0;
    while (param[i] != '\0' && i < 4) {
        i++;
    }

    if (i == 0 && param_required) {
        // No param, error
        sys->RUNNING = 0;
        printf("readOPFormat: missing required param\n");
        printf("Line: '%s'\n\n", line);
        return;
    }

    // 0xE0 vs E0 cases
    if (i > 2 && (param[0] != '0' || param[1] != 'x')) {
        sys->RUNNING = 0;
        printf("readOPFormat: too many param digits or missing 0x prefix\n");
        printf("Line: '%s'\n\n", line);
        return;
    }

    uint8 startIndex = (uint8)(i > 2) * 2; // if i > 2, this = 2, else = 0

    // Build param as a number for mem write
    uint8 param_byte = 0;
    uint8 convertedChar;
    for (uint8 a = startIndex; a < i; ++a) {
        param_byte = param_byte << 4;
        convertedChar = convHexChar(param[a]);

        if (convertedChar == 50) {
            // Error has occurred
            sys->RUNNING = 0;
            printf("readOPFormat: param characters not in hex\n");
            printf("Line: '%s'\n\n", line);
            return;
        }

        param_byte = param_byte + convertedChar;
    }

    // Write the decoded info into memory
    int16 temp;
    baseAddr = (uint8*)(&temp);
    *(baseAddr + 1) = opcode_byte;
    *baseAddr = param_byte;
    sys->MEMORY[sys->PC] = temp;
}

void setPC (Machine* sys, char* line) {
    if (!line) {
        sys->RUNNING = 0;
        printf("setPC: 'line' is nullptr\n\n");
        return;
    }
    
    // Count characters to interpret
    uint8 i = 0;
    char* curr = line + 1; // line[0] == '$' to get here
    while (*curr != '\0' && *curr != ' ' && i < 4) {
        curr++;
        i++;
    }

    if (i == 0) {
        sys->RUNNING = 0;
        printf("setPC: no PC value\n");
        printf("Line: '%s'\n\n", line);
        return;
    }

    // 0xE0 vs E0 cases
    if (i > 2 && (line[1] != '0' || line[2] != 'x')) {
        sys->RUNNING = 0;
        printf("setPC: too many param digits or missing 0x prefix\n");
        printf("Line: '%s'\n\n", line);
        return;
    }

    uint8 startIndex = (uint8)(i > 2) * 2; // if i > 2, this = 2, else = 0

    // Convert the chars to a value
    uint8 newPC = 0;
    uint8 convertedChar;
    for (uint8 a = startIndex; a < i; ++a) {
        newPC = newPC << 4;
        convertedChar = convHexChar(line[a + 1]);
        
        if (convertedChar == 50) {
            sys->RUNNING = 0;
            printf("setPC: values not in hex\n");
            printf("Line: '%s'\n\n", line);
            return;
        }

        newPC = newPC + convertedChar;
    }

    sys->PC = newPC;
}

void saveNumber (Machine* sys, char* line) {
    if (!line) {
        sys->RUNNING = 0;
        printf("saveNumber: 'line' is nullptr\n\n");
        return;
    }
    
    // Check for the x too!
    if (line[1] != 'x') {
        sys->RUNNING = 0;
        printf("saveNumber: 0x prefix missing\n");
        printf("Line: '%s'\n\n", line);
    }

    // Count chars
    uint8 i = 0;
    char* curr = line + 2;
    while (*curr != '\0' && *curr != ' ' && i < 4) {
        curr++;
        i++;
    }

    if (i == 0) {
        sys->RUNNING = 0;
        printf("saveNumber: no number given\n");
        printf("Line: '%s'\n\n", line);
        return;
    }
    
    // Convert to a store-able value
    int16 temp = 0;
    int16 convertedChar;
    for (uint8 a = 0; a < i; ++a) {
        temp = temp << 4;
        convertedChar = (int16)(convHexChar(line[a + 2]));

        if (convertedChar == 50) {
            sys->RUNNING = 0;
            printf("saveNumber: input not in hex\n");
            printf("Line: '%s'\n\n", line);
            return;
        }

        temp = temp + convertedChar;
    }

    sys->MEMORY[sys->PC] = temp;
}

void IMPORTCODE (Machine* sys) {
    FILE* codeFile = fopen("test.s", "r");
    
    if (!codeFile) {
        sys->RUNNING = 0;
        printf("IMPORTCODE: Failed to open code file\n\n");
        return;
    }
    
    char line[80];

    // Empty the string
    for (uint8 i = 0; i < 80; ++i) {
        line[i] = '\0';
    }

    while (fgets(line, 80, codeFile) && sys->RUNNING) {
        // Skip blank/comment lines
        if (line[0] == '\x0A' || line[0] == '\x0D' ||
            line[0] == ';') { continue; }
        // Remove newline chars
        for (uint8 i = 0; i < 80; ++i) {
            if (line[i] == '\x0A' || line[i] == '\x0D') {
                line[i] = '\0';
            }
        }

        // Guarantee null termination
        line[79] = '\0';

        switch (line[0]) {
            case '$':
                // Set PC
                setPC(sys, line);
                break;
            case '0':
                // Read num into memory
                saveNumber(sys, line);
                sys->PC++;
                break;
            default:
                // Read the OPCODE PARAM format
                readOPFormat(sys, line);
                sys->PC++;
                break;
        }

        // Empty the string
        for (uint8 i = 0; i < 80; ++i) {
            line[i] = '\0';
        }
    }
}

uint8 convHexChar (char A) {
    uint8 rawValue = (uint8)(A);
    uint8 inRange = (uint8)((rawValue >= 48 && rawValue <= 57) ||
        (rawValue >= 65 && rawValue <= 70) ||
        (rawValue >= 97 && rawValue <= 102));
    uint8 delta = (uint8)(rawValue >= 48 && rawValue <= 57) * 48 +
        (uint8)(rawValue >= 65 && rawValue <= 70) * 55 +
        (uint8)(rawValue >= 97 && rawValue <= 102) * 87;
    uint8 returnVal = (rawValue - delta) * inRange + 50 * (uint8)(!inRange);
    return returnVal;
}
