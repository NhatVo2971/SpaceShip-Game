#include "mbox.h"
#include "uart.h"
#include "framebf.h"
#include "function.h"
#include "text.h"
#include "display_image.h"
#include "game.h"
#include "object.h"
#include "game_universe_background.h"


// Use RGBA32 (32 bits for each pixel)
#define COLOR_DEPTH 32
// Pixel Order: BGR in memory order (little endian --> RGB in byte order)
#define PIXEL_ORDER 0
// Screen info
unsigned int width, height, pitch;
/* Frame buffer address
 * (declare as pointer of unsigned char to access each byte) */
unsigned char *fb;
/**
 * Set screen resolution to 1024x768
 */
void framebf_init()
{
    mBuf[0] = 35 * 4; // Length of message in bytes
    mBuf[1] = MBOX_REQUEST;
    mBuf[2] = MBOX_TAG_SETPHYWH;  // Set physical width-height
    mBuf[3] = 8;                  // Value size in bytes
    mBuf[4] = 0;                  // REQUEST CODE = 0
    mBuf[5] = 1024;               // Value(width)
    mBuf[6] = 768;                // Value(height)
    mBuf[7] = MBOX_TAG_SETVIRTWH; // Set virtual width-height
    mBuf[8] = 8;
    mBuf[9] = 0;
    mBuf[10] = 1024;
    mBuf[11] = 768;
    mBuf[12] = MBOX_TAG_SETVIRTOFF; // Set virtual offset
    mBuf[13] = 8;
    mBuf[14] = 0;
    mBuf[15] = 0;                 // x offset
    mBuf[16] = 0;                 // y offset
    mBuf[17] = MBOX_TAG_SETDEPTH; // Set color depth
    mBuf[18] = 4;
    mBuf[19] = 0;
    mBuf[20] = COLOR_DEPTH;         // Bits per pixel
    mBuf[21] = MBOX_TAG_SETPXLORDR; // Set pixel order
    mBuf[22] = 4;
    mBuf[23] = 0;
    mBuf[24] = PIXEL_ORDER;
    mBuf[25] = MBOX_TAG_GETFB; // Get frame buffer
    mBuf[26] = 8;
    mBuf[27] = 0;
    mBuf[28] = 16;                // alignment in 16 bytes
    mBuf[29] = 0;                 // will return Frame Buffer size in bytes
    mBuf[30] = MBOX_TAG_GETPITCH; // Get pitch
    mBuf[31] = 4;
    mBuf[32] = 0;
    mBuf[33] = 0; // Will get pitch value here
    mBuf[34] = MBOX_TAG_LAST;
    // Call Mailbox
    if (mbox_call(ADDR(mBuf), MBOX_CH_PROP) // mailbox call is successful ?
        && mBuf[20] == COLOR_DEPTH          // got correct color depth ?
        && mBuf[24] == PIXEL_ORDER          // got correct pixel order ?
        && mBuf[28] != 0                    // got a valid address for frame buffer ?
    )
    {
        /* Convert GPU address to ARM address (clear higher address bits)
         * Frame Buffer is located in RAM memory, which VideoCore MMU
         * maps it to bus address space starting at 0xC0000000.
         * Software accessing RAM directly use physical addresses
         * (based at 0x00000000)
         */
        mBuf[28] &= 0x3FFFFFFF;
        // Access frame buffer as 1 byte per each address
        fb = (unsigned char *)((unsigned long)mBuf[28]);
        uart_puts("Got allocated Frame Buffer at RAM physical address: ");
        uart_hex(mBuf[28]);
        uart_puts("\n");
        uart_puts("Frame Buffer Size (bytes): ");
        uart_dec(mBuf[29]);
        uart_puts("\n");
        width = mBuf[5];  // Actual physical width
        height = mBuf[6]; // Actual physical height
        pitch = mBuf[33]; // Number of bytes per line
    }
    else
    {
        uart_puts("Unable to get a frame buffer with provided setting\n");
    }
}

// Function to draw pixel with ARGB32
//----------------------------------------------------------------------------
void drawPixelARGB32(int x, int y, unsigned int attr)
{
    int offs = (y * pitch) + (COLOR_DEPTH / 8 * x);
    /* //Access and assign each byte
     *(fb + offs ) = (attr >> 0 ) & 0xFF; //BLUE
     *(fb + offs + 1) = (attr >> 8 ) & 0xFF; //GREEN
     *(fb + offs + 2) = (attr >> 16) & 0xFF; //RED
     *(fb + offs + 3) = (attr >> 24) & 0xFF; //ALPHA
     */
    // Access 32-bit together
    *((unsigned int *)(fb + offs)) = attr;
}

void drawPixelARGB32noBackground(int x, int y, unsigned int attr)
{
    int offs = (y * pitch) + (COLOR_DEPTH / 8 * x);
    /* //Access and assign each byte
     *(fb + offs ) = (attr >> 0 ) & 0xFF; //BLUE
     *(fb + offs + 1) = (attr >> 8 ) & 0xFF; //GREEN
     *(fb + offs + 2) = (attr >> 16) & 0xFF; //RED
     *(fb + offs + 3) = (attr >> 24) & 0xFF; //ALPHA
     */
    // Access 32-bit together
    if (attr !=  0x00000000){
        *((unsigned int *)(fb + offs)) = attr;
    }
}

// Function to draw rectangle
//----------------------------------------------------------------------------
void drawRectARGB32(int x1, int y1, int x2, int y2, unsigned int attr, int fill)
{
    for (int y = y1; y <= y2; y++)
        for (int x = x1; x <= x2; x++)
        {
            if ((x == x1 || x == x2) || (y == y1 || y == y2))
                drawPixelARGB32(x, y, attr);
            else if (fill)
                drawPixelARGB32(x, y, attr);
        }
}


// Function to draw line
//----------------------------------------------------------------------------
void drawLineARGB32(int x1, int y1, int x2, int y2, unsigned int attr)
{
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    while (1)
    {
        drawPixelARGB32(x1, y1, attr);

        if (x1 == x2 && y1 == y2)
            break;

        int e2 = 2 * err;
        if (e2 > -dy)
        {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx)
        {
            err += dx;
            y1 += sy;
        }
    }
}

// Function to draw circle
//----------------------------------------------------------------------------
void drawCircleARGB32(int centerX, int centerY, int radius, unsigned int attr)
{
    int x = radius;
    int y = 0;
    int radiusError = 1 - x;

    while (x >= y)
    {
        drawPixelARGB32(centerX + x, centerY + y, attr);
        drawPixelARGB32(centerX - x, centerY + y, attr);
        drawPixelARGB32(centerX + x, centerY - y, attr);
        drawPixelARGB32(centerX - x, centerY - y, attr);
        drawPixelARGB32(centerX + y, centerY + x, attr);
        drawPixelARGB32(centerX - y, centerY + x, attr);
        drawPixelARGB32(centerX + y, centerY - x, attr);
        drawPixelARGB32(centerX - y, centerY - x, attr);

        y++;

        if (radiusError < 0)
        {
            radiusError += 2 * y + 1;
        }
        else
        {
            x--;
            radiusError += 2 * (y - x) + 1;
        }
    }
}
// Function to draw pixel
//----------------------------------------------------------------------------
void drawPixel(int x, int y, unsigned char attr) {
    int offs = (y * pitch) + (x * 4);
    *((unsigned int *)(fb + offs)) = vgapal[attr & 0xff];
    // *((unsigned int *)(fb + offs)) = vgapal[attr];
}
// Function print character with font
//----------------------------------------------------------------------------
void drawChar(unsigned char ch, int x, int y, unsigned char attr){
    // unsigned char *glyph = (unsigned char *)&letterA;

    // get index of character from epd_bitmap_allArray
    int index = (ch >= 'A' && ch <= 'Z') ? ch - 55 : ch - 48;
    index = (ch == ' ') ? 37 : index;
    index = (ch == '-') ? 36 : index;

    // Print glyph of character
    for (int i = 0; i < FONT_HEIGHT; i++) {
        for (int j = 0; j < FONT_WIDTH; j++) {
            // unsigned char mask = 1 << j;
            unsigned long mask = 0xffff;

            // unsigned char col = (*glyph & mask) ? (attr & 0x0f) : 1;
            unsigned char *glyph = (unsigned char *)&epd_bitmap_allArray[index][i * FONT_WIDTH + j];
            
            // Get the coordinate to assign color
            unsigned char col = (*glyph & mask) ? (attr & 0xff) : 1;
            // unsigned char col = (*glyph & mask) ? (attr & 0xff) : 1;

            // Move to next column until reach the end of bitmap epd_bitmap_allArray
            if (col != 1) {
                // Fill pixel with current column while neglecting the background pixel
                if ((unsigned char *)&epd_bitmap_allArray[index][i * FONT_WIDTH + j] != 0x0000){
                    drawPixel(x + j, y + i, col);
                }
            }
            
            // // Fill pixel with current column 
            // if ((unsigned char *)&epd_bitmap_allArray[index][i * FONT_WIDTH + j] != 0x0000){
            //     drawPixel(x + j, y + i, col);
            // }
        }
    }
}

// Function print string with font
//----------------------------------------------------------------------------
void drawString(int x, int y, char *s, char* color){
    unsigned char attr;

    // color list
    static const char text_color_list[][50] = {"black",             // 0. Black
                                                "blue",             // 1. Blue
                                                "green",            // 2. GReen
                                                "cyan",             // 3. Cyan
                                                "red",              // 4. Red
                                                "magenta",          // 5. Magenta
                                                "brown",            // 6. Brown
                                                "light grey",       // 7. Light Grey
                                                "dark grey",        // 8. Dark Grey
                                                "bright blue",      // 9. Bright Blue
                                                "bright green",     // 10. Bright Green
                                                "bright cyan",      // 11. Bright Cyan
                                                "bright red",       // 12. Bright Red
                                                "bright magenta",   // 13. Bright Magenta
                                                "yellow",           // 14. Yellow
                                                "white"             // 15. White
                                                };

    // Assign input color to text
    // int text_color_index = 0;
    for (int i = 0; i < 16; i++){
        if (comp_str(color, text_color_list[i]) == 0)
        {
            // text_color_index = i;
            // attr = text_color_list[i];
            attr = i + 2;
            break;
        }
    }
    
    // attr = text_color_list[text_color_index];
    
    while (*s) {
        if (*s == '\r') 
        {
            x = 0;
        } 
        else if (*s == '\n') 
        {
            x = 0;
            y += FONT_HEIGHT;
        } 
        else if (*s == ' ')
        {
            drawChar(*s, x, y, attr);
            x += FONT_WIDTH - 20;
        } 
        else 
        {
            drawChar(*s, x, y, attr);
            x += FONT_WIDTH;
        }
        s++;
    }
}


// Function print content with font
//----------------------------------------------------------------------------
void font()
{
    drawString(5, 180, "HOANG NGHIA TRI HUNG", "blue");

    drawString(5, 340, "MAI XUAN HUY", "red");

    drawString(5, 500, "TRUONG PHUOC HUY", "cyan");

    drawString(5, 660, "VO PHUC DUY NHAT", "green");
}


// Draw list of frame images in video
//----------------------------------------------------------------------------
void display_frame_image(unsigned int frame_image[], int x, int y, int width,int height) {
    int num = 0;

    while (y < height) {
        for (x = 0; x < width; x++) {
            int offs = (y * pitch) + (x * 4);
            *((unsigned int *)(fb + offs)) = frame_image[num];
            num++;
        }
        y++;
        x = 0;
    }
}

// Function to clear projectile
//----------------------------------------------------------------------------
void clear_projectile(Position position, Dimension dimension) {
    int width = dimension.width;
    int height = dimension.height;

    int x = position.x;
    int oldX = x;
    int y = position.y;

    for (int i = 0; i < (width * height); i++) {
        x++;
        if (i % width == 0) {
            y++;
            x = oldX;
        }
        drawPixelARGB32(x, y, background_universe_image[y * universe_background_width + x]);
    }
}
// Function to draw projectile
//----------------------------------------------------------------------------
void draw_projectile(Type type, Position position, Dimension dimension) {
    int *colorptr;
    int width = dimension.width;
    int height = dimension.height;

    if (type != PLAYER){
        if (type == BOSS) {
        colorptr = (int *)boss_bomb.image_pixels;
    } else
        colorptr = (int *)asteroid_image.image_pixels;
    }
    else if (type == PLAYER) {
        colorptr = (int *)red_laser.image_pixels;
    }
    

    int x = position.x;
    int oldX = x;
    int y = position.y;
    for (int i = 0; i < (width * height); i++) {
        x++;
        if (i % width == 0) {
            y++;
            x = oldX;
        }
        // Get the pixel color
        uint32_t pixelColor = colorptr[i];
        
        if (colorptr[i] != 0xFF000000)
        {
            drawPixelARGB32(x, y, pixelColor);
        }
        else {
            drawPixelARGB32(x, y, background_universe_image[y * universe_background_width + x]);
        }
        
    }
}

// Draw an object like ship , alien with pixel data in object.h
//----------------------------------------------------------------------------
void drawEntity(Entity entity) {
    int *colorptr;
    int width = entity.dimension.width;
    int height = entity.dimension.height;

    int x = entity.position.x;
    int oldX = x;
    int y = entity.position.y;

    if (entity.type == BOSS) {
        colorptr = (int *)boss_image.image_pixels;
    }
   if (entity.type == PLAYER)
        colorptr = (int *)blue_ship_sprite.image_pixels;
    // draw in 2D
    for (int i = 0; i < (width * height); i++) {
        x++;
        if (i % width == 0) {
            y++;
            x = oldX;
        }
        if (colorptr[i] != 0xFF000000)
        {
            drawPixelARGB32(x, y, colorptr[i]);
        }
        else {
            drawPixelARGB32(x, y, background_universe_image[y * universe_background_width + x]);
        }
    }
}