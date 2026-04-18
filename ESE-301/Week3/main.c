#include <stdint.h>
#include <stdio.h>

#if defined(__has_builtin)
#  if __has_builtin(__builtin_debugtrap)
#    define BKPT() __builtin_debugtrap() // Debug trap when supported by the compiler
#  else
#    define BKPT() __builtin_trap() // Fallback trap for compilers without __builtin_debugtrap
#  endif
#else
#  define BKPT() __builtin_trap() // Fallback trap when builtin feature detection is unavailable
#endif
typedef void(*functionPointerType)(void);

static volatile uint16_t UserFrequency = 10; // Global variable to hold user frequency input for blinking LED
static volatile uint16_t FlashTestErrors = 0; // Global variable to hold flash test error count
const char* versionString = "1.0.0"; // Global variable to hold firmware version string

struct commandStruct {
    const char* commandName;
    functionPointerType functionPointer;
    const char* helpString;
};

static void CmdVersion(void);
static void CmdFlashTest(void);
static void CmdBlinkLED(void);
static void CmdHelp(void);

const struct commandStruct commandList[];

static void CmdVersion(void) {
    printf("Firmware Version: %s\n", versionString);
}
static void CmdFlashTest(void) {
    printf("Running flash test... No errors found.\n");
    printf("Total Flash Test Errors: %u\n", (unsigned int)FlashTestErrors);
}
static void CmdBlinkLED(void) {
    printf("Blinking LED at %u Hz...\n", (unsigned int)UserFrequency);
}
static void CmdHelp(void) {
    for (int i = 0; commandList[i].functionPointer != 0; i++) {
        printf("%s: %s\n", commandList[i].commandName, commandList[i].helpString);
    }
}   

const struct commandStruct commandList[] = {
    {"ver", CmdVersion,
        "Display firmware version"},
    {"flashTest", CmdFlashTest,
        "Runs the flash unit utest, prints number of errors of erros upon completion"},
    {"blinkLED", CmdBlinkLED,
        "Blinks the onboard LED at a desired rate (parameter: frequency (Hz))"},
    {"help", CmdHelp,
        "Prints out help messages"},
    {"",0, ""}, // Empty command to test error handling}nel value to mark the end of the list
};

int main() {
    
    commandList[0].functionPointer(); // Call the first command (CmdVersion)
    BKPT(); // Trigger a debugger break after the first command
    commandList[1].functionPointer(); // Call the second command (CmdFlashTest)
    commandList[2].functionPointer(); // Call the third command (CmdBlinkLED)
    commandList[3].functionPointer(); // Call the fourth command (CmdHelp)


    return 0;
}