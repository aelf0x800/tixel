/*
 * tixel - A header only library for drawing pixels in a terminal and handling
 *         user input.
 */
#ifndef TIXEL_H
#define TIXEL_H

#include <termios.h>

/*
 * Predefined error values
 */
#define TIXEL_OK                       0
#define TIXEL_ERROR_BAD_DIMENSIONS     -1
#define TIXEL_ERROR_PIXEL_ALLOC_FAILED -2

/*
 * RRGGBB color
 */
typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} TixelColor;

/*
 * Tixel context structure
 */
typedef struct {
    TixelColor* pixels;
    unsigned    width;
    unsigned    height;
    // Termios
    struct termios orig_state;
    struct termios raw_state;
} Tixel;

/*
 * Initialisation and de-initialisation function definitions
 */
static int  tixel_init(Tixel* self, unsigned width, unsigned height);
static void tixel_deinit(Tixel* self);
/*
 * Primitive drawing function definitions
 */
// Clear the pixel buffer with a color
void tixel_clear(Tixel* self, TixelColor color);
// Draw a pixel
void tixel_draw_pixel(Tixel* self, unsigned x, unsigned y, TixelColor color);
// Copy another pixel buffer to the main one with an offset
void tixel_draw_buffer(
    Tixel*         self, 
    unsigned       x_off, 
    unsigned       y_off,
    unsigned       width, 
    unsigned       height, 
    unsigned char* buffer
);
/*
 * Shape drawing function definitions
 */
// Draw a line
void tixel_draw_line(
    Tixel*     self, 
    unsigned   x0, 
    unsigned   y0,
    unsigned   x1, 
    unsigned   y1, 
    TixelColor color
);
// Draw a filled in rectangle
void tixel_draw_rectangle(
    Tixel*     self, 
    unsigned   x, 
    unsigned   y,
    unsigned   width, 
    unsigned   height, 
    TixelColor color
);
// Draw the lines of a rectangle
void tixel_draw_rectangle_lines(
    Tixel*     self, 
    unsigned   x, 
    unsigned   y, 
    unsigned   width, 
    unsigned   height, 
    TixelColor color
);
// Draw a filled in triangle
void tixel_draw_triangle(
    Tixel*     self, 
    unsigned   x0, 
    unsigned   y0, 
    unsigned   x1, 
    unsigned   y1, 
    unsigned   x2, 
    unsigned   y2, 
    TixelColor color
);
// Draw the lines of triangle
void tixel_draw_triangle_lines(
    Tixel*   self, 
    unsigned   x0, 
    unsigned   y0, 
    unsigned   x1, 
    unsigned   y1, 
    unsigned   x2, 
    unsigned   y2, 
    TixelColor color
);
// Draw a filled in ellipse
void tixel_draw_ellipse(
    Tixel*     self, 
    unsigned   x, 
    unsigned   y, 
    float      radius_h,
    float      radius_v,
    TixelColor color
);
// Draw the lines of an ellipse
void tixel_draw_ellipse_lines(
    Tixel*     self, 
    unsigned   x, 
    unsigned   y, 
    float      radius_h,
    float      radius_v,
    TixelColor color
);

/*
 * Provide the bodies for the functions
 */
#ifdef TIXEL_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * Initialisation and de-initialisation function bodies
 */
static int tixel_init(Tixel* self, unsigned width, unsigned height) {
    // Set width and height
    self->width  = width;
    self->height = height;
    if (self->width == 0 || self->height == 0)
        return TIXEL_ERROR_BAD_DIMENSIONS;

    // Allocate the pixels
    self->pixels = (TixelColor*)malloc(width * height * sizeof(TixelColor));
    if (self->pixels == NULL)
        return TIXEL_ERROR_PIXEL_ALLOC_FAILED;

    // Store original terminal state
    tcgetattr(STDIN_FILENO, &self->orig_state);

    // Enter raw mode
    self->raw_state.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    self->raw_state.c_oflag &= ~OPOST;
    self->raw_state.c_cflag |= CS8;
    self->raw_state.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &self->raw_state);

    return TIXEL_OK;
}

static void tixel_deinit(Tixel* self) {
    // Exit raw mode and clear terminal
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &self->orig_state);
    printf("\x1b[0m\x1b[2J\x1b[H");

    // Free memory and zero the struct
    if (self->pixels != NULL) {
        free(self->pixels);
        memset(self, 0, sizeof(Tixel));
    }   
}


/*
 * Primative drawing function definitions
 */
// Displays the pixel buffer
void tixel_show(Tixel* self) {
    // Clear the terminal
    printf("\x1b[0m\x1b[2J\x1b[H");

    for (unsigned y = 0; y < self->height; y += 2) {
        for (unsigned x = 0; x < self->width; x++) {
            // Set the top pixel color
            TixelColor top = self->pixels[y * self->width + x];
            printf("\x1b[38;2;%d;%d;%dm", top.r, top.g, top.b);
            
            // Set the bottom pixel color
            if (y + 1 < self->height) {
                TixelColor bot = self->pixels[(y + 1) * self->width + x];
                printf("\x1b[48;2;%d;%d;%dm", bot.r, bot.g, bot.b);
            }

            // Show the pixel
            printf("▀");
        }

        // Move cursor to next row
        printf("\x1b[B\x1b[1G");
    }
}

// Clear the pixel buffer with a color
void tixel_clear(Tixel* self, TixelColor color) {
    for (unsigned i = 0; i < self->width * self->height; i++)
        self->pixels[i] = color;
}

// Draw a pixel
void tixel_draw_pixel(Tixel* self, unsigned x, unsigned y, TixelColor color) {
    if (x < self->width && y < self->height)
        self->pixels[y * self->width + x] = color;
}

// Copy a buffer to the pixel buffer with an offset
void tixel_draw_buffer(
    Tixel*         self, 
    unsigned       x_off, 
    unsigned       y_off,
    unsigned       width, 
    unsigned       height, 
    unsigned char* buffer
) {
    // This assumes the buffer is data in the buffer has 3 components per pixel
    for (unsigned y = 0; y < height; y++)
        for (unsigned x = 0; x < width; x++) {
            TixelColor color = {
                .r = buffer[y * (width * 3) + x * 3],
                .g = buffer[y * (width * 3) + x * 3 + 1],
                .b = buffer[y * (width * 3) + x * 3 + 2]
            };
            if (x + x_off < self->width && y + y_off < self->height)
                tixel_draw_pixel(self, x + x_off, y + y_off, color);
        }
}

/*
 * Shape drawing function definitions
 */
// Draw a line
void tixel_draw_line(
    Tixel*     self, 
    unsigned   x0, 
    unsigned   y0,
    unsigned   x1, 
    unsigned   y1, 
    TixelColor color
) {
    int x_diff = abs(x1 - x0);
    int x_inc  = x0 < x1 ? 1 : -1;
    int y_diff = -abs(y1 - y0);
    int y_inc  = y0 < y1 ? 1 : -1;
    int error  = x_diff + y_diff;

    while (1) {
        tixel_draw_pixel(self, x0, y0, color);

        int error_2 = 2 * error;
        
        if (error_2 >= y_diff) {
            if (x0 == x1) 
                break;
            error += y_diff;
            x0    += x_inc;
        }
        
        if (error_2 <= x_diff) {
            if (y0 == y1) 
                break;
            error += x_diff;
            y0    += y_inc;
        }
    }
}

// Draw a filled in rectangle
void tixel_draw_rectangle(
    Tixel*     self, 
    unsigned   x, 
    unsigned   y,
    unsigned   width, 
    unsigned   height, 
    TixelColor color
) {
    for (unsigned y0 = y; y0 < y + height; y0++)
        for (unsigned x0 = x; x0 < x + width; x0++)
            // Only draw if in bounds
            if (x0 < self->width && y0 < self->height)
                tixel_draw_pixel(self, x0, y0, color);
}

// Draw the lines of a rectangle
void tixel_draw_rectangle_lines(
    Tixel*     self, 
    unsigned   x, 
    unsigned   y, 
    unsigned   width, 
    unsigned   height, 
    TixelColor color
) {
    for (unsigned y0 = y; y0 < y + height; y0++)
        for (unsigned x0 = x; x0 < x + width; x0++)
        {
            // Skip the center
            if (x0 != x && 
                y0 != y && 
                x0 != x + width - 1 && 
                y0 != y + height - 1)
                continue;

            // Only draw if in bounds
            if (x0 < self->width && y0 < self->height)
                tixel_draw_pixel(self, x0, y0, color);
        }
}

// Draw a filled in triangle
void tixel_draw_triangle(
    Tixel*     self, 
    unsigned   x0, 
    unsigned   y0, 
    unsigned   x1, 
    unsigned   y1, 
    unsigned   x2, 
    unsigned   y2, 
    TixelColor color
);
// Draw the lines of triangle
void tixel_draw_triangle_lines(
    Tixel*   self, 
    unsigned   x0, 
    unsigned   y0, 
    unsigned   x1, 
    unsigned   y1, 
    unsigned   x2, 
    unsigned   y2, 
    TixelColor color
) {
    tixel_draw_line(self, x0, y0, x1, y1, color);
    tixel_draw_line(self, x1, y1, x2, y2, color);
    tixel_draw_line(self, x2, y2, x0, y0, color);
}

// Draw a filled in ellipse
void tixel_draw_ellipse(
    Tixel*     self, 
    unsigned   x, 
    unsigned   y, 
    float      radius_h,
    float      radius_v,
    TixelColor color
);
// Draw the lines of an ellipse
void tixel_draw_ellipse_lines(
    Tixel*     self, 
    unsigned   x, 
    unsigned   y, 
    float      radius_h,
    float      radius_v,
    TixelColor color
);


#endif // TIXEL_IMPLEMENTATION

#endif // TIXEL_H
