#include "uart.h"
#include "mbox.h"
#include "framebf.h"
#include "game_background.h"
#include "game_universe_background.h"

// Declare reset_arr to reset the cli buffer
//--------------------------------------------------------------------------------
void reset_arr(char* arr){
	int i = 0;
	while(arr[i] != '\0') {
		arr[i] = '\0';
		i++;
	 }
}
// Function to reset buffer
//--------------------------------------------------------------------------------
void reset_ptr(const char* cli_buffer){
	while(*cli_buffer != '\n'){
		cli_buffer++;
	}
}
// Function to compare input cli
//--------------------------------------------------------------------------------
int comp_str(const char* s1,const char* s2){	
	while (*s1 != '\0' && *s2 != '\0') {
		// Check if s1 no equal to s2
        if (*s1 < *s2) {
            return -1;
        } else if (*s1 > *s2) {
            return 1;
        }
        s1++;
        s2++;
    }

    // At this point, one or both strings have ended
    if (*s1 == '\0' && *s2 == '\0') {
        return 0; 	// Both strings are equal
    } else if (*s1 == '\0') {
        return -1; 	// s1 is Null
    } else {
        return 1; 	// s2 is either Null or no equal s1
    }
}

// Function return absolute int value
//--------------------------------------------------------------------------------
int abs(int x)
{
    return (x < 0) ? -x : x;
}

// Function convert num to string
//--------------------------------------------------------------------------------
void tostring(char str[], int num)
{
    int i, rem, len = 0, n;
 
    n = num;
    while (n != 0)
    {
        len++;
        n /= 10;
    }
    for (i = 0; i < len; i++)
    {
        rem = num % 10;
        num = num / 10;
        str[len - (i + 1)] = rem + '0';
    }
    str[len] = '\0';
}
// Generate a function to find the length of the string
//--------------------------------------------------------------------------------
int string_length(char *str)
{
  int i = 0;
  while (str[i] != '\0')
  {
    i++;
  }
  return i;
}

// Generate a function convert integer to string and return string
//--------------------------------------------------------------------------------
void integer_to_string(int n, char *str)
{
  int i = 0;
  while (n != 0)
  {
    int rem = n % 10;
    str[i] = rem + '0';
    n = n / 10;
    i++;
  }
  str[i] = '\0';
  // reverse the string
  int j = 0;
  int k = string_length(str) - 1;
  while (j < k)
  {
    char temp = str[j];
    str[j] = str[k];
    str[k] = temp;
    j++;
    k--;
  }
}
// Function copy pointer
//--------------------------------------------------------------------------------
void *memcpy(void *dest, const void *src, unsigned long n)
{
    for (unsigned long i = 0; i < n; i++)
    {
        ((char *)dest)[i] = ((char *)src)[i];
    }
}
// Function to display the screen background image
//--------------------------------------------------------------------------------
void displayGameBackground(int x, int y){
  for (int h = 0; h < background_height; h++)
  {
    for (int w = 0; w < background_width; w++)
    {
      drawPixelARGB32(x + w, y + h, background_image[h * background_width + w]);
    }
  }
}
// Function to display the screen background universe image
//--------------------------------------------------------------------------------
void displayGameUniverseBackground(int x, int y){
  for (int h = 0; h < universe_background_height; h++)
  {
    for (int w = 0; w < universe_background_width; w++)
    {
      drawPixelARGB32(x + w, y + h, background_universe_image[h * universe_background_width + w]);
    }
  }
}
// Function to clear emulator screen
//--------------------------------------------------------------------------------
void clearscreen(int x, int y){
  for (int h = 0; h < background_height; h++)
  {
    for (int w = 0; w < background_width; w++)
    {
      drawPixelARGB32(x + w, y + h, 0x00000000);
    }
  }
}

// Function to display commands menu
//--------------------------------------------------------------------------------
void displayMenu()
{
    uart_puts("\n----------------------------------------------------------------------------------------------\n");
    uart_puts(
        "\n\tEnter a number to choose command:\n"
        "\t1.\tDisplay a scrollable image\n"
        "\t2.\tDisplay a video\n"
        "\t3.\tDisplay member names on screen\n"
        "\t4.\tPlay game\n"
        "\t0.\tClear the screen\n");
    uart_puts("\n----------------------------------------------------------------------------------------------\n");
}


