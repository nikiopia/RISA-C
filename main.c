#include "risa.h"

int main (int argc, char** argv) {
    // Start the machine
    Machine sys;
    initMachine(&sys);
    printf("Machine initialized.\n\n");

	IMPORTCODE(&sys);
	sys.PC = 0;

    // Core dumps
    printf("Core dumps:\n");
    for (uint8 i = 0; i < 0x1D; ++i) {
        OUT(&sys, i);
    }
    for (uint8 i = 0xE0; i < 0xE5; ++i) {
        OUT(&sys, i);
    }
    for (uint8 i = 0xF0; i < 0xF2; ++i) {
        OUT(&sys, i);
    }
    printf("\n");

    uint32 watchDog = 0; 
    printf("Machine output:\n");
    while (sys.RUNNING && watchDog < 1000) {
        FDE(&sys);
        watchDog++;
    }
    
    if (sys.RUNNING) {
        printf("Watchdog exit!\n");
    } else {
        printf("\n");
    }

    printf("Exited.\n");

    return 0;
}
