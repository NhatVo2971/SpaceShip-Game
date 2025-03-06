#include "framebf.h"
#include "running_video.h"
#include "uart.h"
// Function of Delay Interrupt in one second
//-----------------------------------------------------------
void wait_ms(unsigned int n) {
    register unsigned long f, t, r;

    // Get the current counter frequency
    asm volatile("mrs %0, cntfrq_el0" : "=r"(f));
    // Read the current counter
    asm volatile("mrs %0, cntpct_el0" : "=r"(t));
    // Calculate expire value for counter
    t += ((f / 1000) * n) / 1000;
    do {
        asm volatile("mrs %0, cntpct_el0" : "=r"(r));
    } while (r < t);
}

// Function to display video
//-----------------------------------------------------------
void displayVideo(int x, int y) {
    // Add message to notify the user how to use
    uart_puts("Video is playing ...\n");
    uart_puts("Press x to stop ");
    char character = uart_get_char();
    // Keep displaying the video until the user press x
    while (character != 'x') {
        // // loop through all the frame image video
        for (int i = 0; i < NUM_FRAMES; i++) {
            // display each frame image
            display_frame_image(sample_video[i], x, y, video_width, video_height + y);
            wait_ms(100000);
        }
        character = uart_get_char();
    }

    // Add message to announce the user
    uart_puts("\nStopping video ...\n");
}