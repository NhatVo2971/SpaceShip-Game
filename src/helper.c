#include "helper.h"
#include "framebf.h"

// Function to clear the emulator screen before showing new thing
//---------------------------------------------------------------------
void clear_emulator_screen(int display_width, int display_height) {
  // Clear both width and height of the screen
  for (int h = 0; h < display_height; h++) {
    for (int w = 0; w < display_width; w++) {
      drawPixel(w, h, 0);
    }
  }
}




