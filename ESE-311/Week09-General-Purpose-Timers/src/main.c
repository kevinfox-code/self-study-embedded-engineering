#include <stdio.h>
#include "timers.h"

int main(void) {
    // Initialize timer functionalities
    timer_init();

    // Start the timer
    timer_start();

    // Main loop
    while (1) {
        // Check timer status and perform actions
        if (timer_check()) {
            // Timer event occurred
            printf("Timer event occurred!\n");
        }
    }

    return 0;
}