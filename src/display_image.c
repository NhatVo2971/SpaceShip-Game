#include "image.h"
#include "framebf.h"
#include "function.h"
#include "uart.h"
#include "game_over.h"
#include "game_win.h"
#include "game_image.h"
#include "life.h"
#include "spaceship.h"
#include "mbox.h"
#include "game_universe_background.h"

#define MAX_DOWN_HEIGHT 935
#define BUFFER_STEP 40

// Function to display second image to scroll
void display_image(int row)
{
  for (int h = 0; h < image_height; h++)
    for (int w = 0; w < image_width; w++)
      drawPixelARGB32(w, h, image[(h + row) * image_width + w]);
}

// Function to display scrollable large image
void controlScrollableImage()
{

	framebf_init();
  uart_puts("\nPress 'w' to scroll up \n");
	uart_puts("Press 's' to scroll down \n");
	uart_puts("Press x to exit image! \n\n");
	
  // Declare where the terminal will show at certain position of the image
  int row = 200;
  display_image(row);

	while (1)
	{
    // Declare character variable to get user input
    char character = uart_getc();
    
    // Scroll down the image withing the range of row to scroll as defined
    if (character == 'w' && (row <= image_height - MAX_DOWN_HEIGHT - BUFFER_STEP))
		{
      row = row + BUFFER_STEP;
			display_image(row);
		}
    // Scroll up the image withing the range of row to scroll as defined
    else if (character == 's' && (row >= BUFFER_STEP))
		{
			row = row - BUFFER_STEP;
			display_image(row);
		}
		else if (character == 'x')
    {
      // Prompt message for user
      uart_puts("\n\nSuccessfully out!\n");
      return;
    }
	}
}

// Function to display word "score"
//--------------------------------------------------------------------------
void displayScore(int x, int y)
{
  for (int h = 0; h < word_score_height; h++)
  {
    for (int w = 0; w < word_score_width; w++)
    {
      
      unsigned long mask = 0xffff;

      unsigned char *glyph = (unsigned char *)&word_score_image[h * word_score_width + w];
            
      // Get the coordinate to assign color - green
      unsigned char col = (*glyph & mask) ? (4 & 0xff) : 1;

      // Move to next column until reach the end of bitmap epd_bitmap_allArray
      if (col != 1) {
        // Fill pixel with current column while neglecting the background pixel
        if ((unsigned char *)&word_score_image[h * word_score_width + w] != 0x0000){
          drawPixel(x + w, y + h, col);
        }
      }
    }
  }
}


// Function to display the player explosion
//--------------------------------------------------------------------------
void displayExplosion(int x, int y)
{
  for (int h = 0; h < explosion_height; h++)
  {
    for (int w = 0; w < explosion_width; w++)
    {
      drawPixelARGB32noBackground(x + w, y + h, explosion_image[h * explosion_width + w]);
    }
  }
}
// Function to display the asteroid explosion
//--------------------------------------------------------------------------
void displayExplosionBig(int x, int y)
{
  for (int h = 0; h < explosion2_height; h++)
  {
    for (int w = 0; w < explosion2_width; w++)
    {
      if (explosion2_image[h * explosion2_width + w]!=0x00000000)
      drawPixelARGB32noBackground(x + w, y + h, explosion2_image[h * explosion2_width + w]);
    }
  }
}


// Function to clear score
//--------------------------------------------------------------------------
void clearScore(unsigned int num, int x, int y)
{

   for (int h = 0; h < 50; h++)
  {
    for (int w = 0; w < 50; w++)
    {
      // Get the position x on the game background
      int imageX = (x + w) % 1024;
      // Get the position y on the game background
      int imageY = (y + h) % 768;
      // Get the color of that portion of game background
      int PixelColor = background_universe_image[imageX + imageY * 1024];
      
      
      drawPixelARGB32(x + w, y + h, PixelColor);
    }
  }

}
// Function to clear lives
//--------------------------------------------------------------------------
void clearPlayerLife(int x, int y)
{
  for (int h = 0; h < life_height; h++)
  {
    for (int w = 0; w < life_width; w++)
    {
      drawPixelARGB32(x + w, y + h, 0);
    }
  }
}


// Function to display word "lives"
//--------------------------------------------------------------------------
void displayWordPlayerLife(int x, int y)
{
  for (int h = 0; h < word_lives_height; h++)
  {
    for (int w = 0; w < word_lives_width; w++)
    {
      unsigned long mask = 0xffff;

      unsigned char *glyph = (unsigned char *)&word_lives_image[h * word_lives_width + w];
            
      // Get the coordinate to assign color - bright magenta
      unsigned char col = (*glyph & mask) ? (15 & 0xff) : 1;

      // Move to next column until reach the end of bitmap epd_bitmap_allArray
      if (col != 1) {
        // Fill pixel with current column while neglecting the background pixel
        if ((unsigned char *)&word_lives_image[h * word_lives_width + w] != 0x0000){
          drawPixel(x + w, y + h, col);
        }
      }
    }
  }
}

// Function to display player live symbol (3 in total)
//--------------------------------------------------------------------------
void displayPlayerLife(int x, int y)
{
  for (int h = 0; h < life_height; h++)
  {
    for (int w = 0; w < life_width; w++)
    {
      drawPixelARGB32(x + w, y + h, life_image[h * life_width + w]);
    }
  }
}


// Function to display game win image
//--------------------------------------------------------------------------
void displayGameWinImage(int x, int y)
{
  for (int h = 0; h < game_win_height_image; h++)
  {
    for (int w = 0; w < game_win_width_image; w++)
    {
      drawPixelARGB32(x + w, y + h, game_win_image[h * game_win_width_image + w]);
    }
  }
}

// Function to display game over image
//--------------------------------------------------------------------------
void displayGameOverImage(int x, int y)
{
  for (int h = 0; h < game_over_height_image; h++)
  {
    for (int w = 0; w < game_over_width_image; w++)
    {
      drawPixelARGB32(x + w, y + h, game_over_image[h * game_over_width_image + w]);
    }
  }
}


// Function to display space ship image without drawing green pixels
//--------------------------------------------------------------------------
void displaySpaceShipImage(int x, int y)
{
  for (int h = 0; h < spaceship_height; h++)
  {
    for (int w = 0; w < spaceship_width; w++)
    {
      // Get the pixel color
      uint32_t pixelColor = spaceship_image[h * spaceship_width + w];
      
      // Check if it's not black (assuming green is 0x00FF00FF in ARGB32)
      if (pixelColor != 0x00000000)
      {
        // Draw the pixel if it's not black
        drawPixelARGB32(x + w, y + h, pixelColor);
      }
    }
  }
}
