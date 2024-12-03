#include <windows.h>
#include <stdio.h>
#include <time.h>
#include "notifications.h"

#define WAIT_DURATION 1800  // Wait duration in seconds (30 minutes)
#define BLOCK_DURATION 600  // Block duration in seconds (10 minutes)

int main() {
    printf("Program started. Waiting for 30 minutes before executing...\n");

    time_t start_time = time(NULL);          // Get the start time
    time_t wait_end_time = start_time + WAIT_DURATION; // Calculate the end time
    int countdown = 30;                      // Countdown in minutes

    // Countdown loop for 30 minutes
    while (1) {
        time_t current_time = time(NULL);
        // Check if a minute has passed
        if (current_time - start_time >= (30 - countdown) * 60) {
            if (countdown == 1) {
                printf("System will lock in %d minute.\n", countdown);
            } else {
                printf("System will lock in %d minutes.\n", countdown);
            }
            countdown--;
        }
        // Break when the wait period ends
        if (current_time >= wait_end_time) {
            break;
        }

        Sleep(1000); // Sleep for 1 second to avoid busy-waiting
    }

    printf("30 minutes have passed. Preparing to lock the workstation.\n");

    // Notify the user
    notify_user("The system will lock in 10 seconds.");
    Sleep(10000);  // Wait 10 seconds after notifying the user

    // Start blocking period
    time_t block_start_time = time(NULL);
    time_t block_end_time = block_start_time + BLOCK_DURATION;

    while (1) {
        time_t current_time = time(NULL);

        if (current_time < block_end_time) {
            // Lock the workstation during the block period
            LockWorkStation();
            Sleep(2000);  // Prevent instant re-login attempts by sleeping 2 seconds
        } else {
            // Exit the block period
            printf("Block period has ended. You can log in now.\n");
            break;
        }
    }

    return 0;
}
