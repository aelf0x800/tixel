/*=================================================================================
 *
 * tixel - A header only library for drawing pixels in a terminal and handling
 *         user input.
 * 
 * LICENSE : MIT License
 *
 * Copyright (c) 2025 aelf0x800
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *===============================================================================*/
#ifndef TIXEL_H
#define TIXEL_H

#include <termios.h>

/*
 * Predefined error values
 */
#define TIXEL_OK                                  0
#define TIXEL_ERROR_BAD_DIMENSIONS               -1
#define TIXEL_ERROR_PIXEL_ALLOC_FAILED           -2
#define TIXEL_ERROR_FAILED_TO_STORE_ORIG_STATE   -3
#define TIXEL_ERROR_FAILED_TO_ENTER_RAW_MODE     -4
#define TIXEL_ERROR_FAILED_TO_RESTORE_ORIG_STATE -5

/*
 * Min & max macros
 */
#define TIXEL_MIN(X, Y) (X < Y ? X : Y)
#define TIXEL_MAX(X, Y) (X > Y ? X : Y)

/*
 * Color comparison macro
 */
#define TIXEL_COLOR_EQ(C0, C1) (C0.r == C1.r && C0.g == C1.g && C0.b == C1.b)

/*
 * RGB color
 */
typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} TixelColor;

/*
 * Tixel event types
 */
typedef enum {
    TIXEL_EVENT_TYPE_NONE,
    TIXEL_EVENT_TYPE_QUIT,
    TIXEL_EVENT_TYPE_KEY_PRESS,
    TIXEL_EVENT_TYPE_MOUSE_MOVE,
    TIXEL_EVENT_TYPE_MOUSE_CLICK
} TixelEventType;

/*
 * Keyboard modfiers
 */
#define TIXEL_CTRL(C) (C & 0x1f)

/*
 * Mouse buttons
 */
typedef enum {
    TIXEL_MOUSE_BTN_LEFT,
    TIXEL_MOUSE_BTN_MIDDLE,
    TIXEL_MOUSE_BTN_RIGHT
} TixelMouseBtn;

/*
 * Tixel event structure
 */
typedef struct {
    // Event type
    TixelEventType type;
    // User input
    char          key;
    unsigned      mouse_x;
    unsigned      mouse_y;
    TixelMouseBtn mouse_btn;
} TixelEvent;

/*
 * Tixel context structure
 */
typedef struct {
    // Pixel buffer
    TixelColor* pixels;
    unsigned    width;
    unsigned    height;
    // Termios terminal states
    struct termios orig_state;
    struct termios raw_state;
    // Quit key
    char quit_key;
} Tixel;

/*
 * Initialisation and de-initialisation function definitions
 */
static int tixel_init(Tixel* self, unsigned width, unsigned height);
static int tixel_deinit(Tixel* self);
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
/*
 * Event handling functions
 */
// See if an event has occured
void tixel_poll_event(Tixel* self, TixelEvent* event);

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
    if (tcgetattr(STDIN_FILENO, &self->orig_state) == -1)
        return TIXEL_ERROR_FAILED_TO_STORE_ORIG_STATE;

    // Set raw mode flags
    self->raw_state.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    self->raw_state.c_oflag &= ~OPOST;
    self->raw_state.c_cflag |= CS8;
    self->raw_state.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    // Time out for reading from stdin
    self->raw_state.c_cc[VMIN]  = 0;
    self->raw_state.c_cc[VTIME] = 1;
    // Enter raw mode
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &self->raw_state) == -1)
        return TIXEL_ERROR_FAILED_TO_ENTER_RAW_MODE;

    // Set default quit key
    self->quit_key = TIXEL_CTRL('q');

    return TIXEL_OK;
}

static int tixel_deinit(Tixel* self) {
    // Exit raw mode, clear terminal and show cursor
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &self->orig_state) == -1)
        return TIXEL_ERROR_FAILED_TO_RESTORE_ORIG_STATE;
    printf("\x1b[0m\x1b[2J\x1b[H\x1b[?25h");

    // Free memory and zero the struct
    if (self->pixels != NULL) {
        free(self->pixels);
        memset(self, 0, sizeof(Tixel));
    }

    return TIXEL_OK;
}

/*
 * Primative drawing function definitions
 */
// Displays the pixel buffer
void tixel_show(Tixel* self) {
    // Clear the terminal and hide the cursor
    printf("\x1b[0m\x1b[2J\x1b[H\x1b[?25l");

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
        for (unsigned x0 = x; x0 < x + width; x0++) {
            // Skip the center
            if (
                x0 != x && 
                y0 != y && 
                x0 != x + width - 1 && 
                y0 != y + height - 1
            ) continue;

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
) {
    // Draw the triangles lines
    tixel_draw_triangle_lines(self, x0, y0, x1, y1, x2, y2, color);
    
    // Fill the triangle using scanlines
    unsigned clo_x = TIXEL_MIN(x0, TIXEL_MIN(x1, x2));
    unsigned far_x = TIXEL_MAX(x0, TIXEL_MAX(x1, x2));
    unsigned clo_y = TIXEL_MIN(y0, TIXEL_MIN(y1, y2));
    unsigned far_y = TIXEL_MAX(y0, TIXEL_MAX(y1, y2));
    for (unsigned y = clo_y; y < far_y; y++) {
        // Find start of triangle on the scanline
        unsigned x = clo_x;
        while (
            x < far_x &&
            x < self->width && 
            !TIXEL_COLOR_EQ(self->pixels[y * self->width + x], color)
        ) x++;
        
        // Get width of triangle on the scanline
        unsigned width = 1;
        while (1) {
            // If the opposite end has not been reached
            if (x + width == far_x || x + width == self->width)
                break;

            // Check if the opposite has been reached
            if (TIXEL_COLOR_EQ(self->pixels[y * self->width + x + width], color))
                break;

            width++;
        }

        // Fill the scanline region
        for (unsigned i = 0; i < width; i++)
            self->pixels[y * self->width + x + i] = color;
    }
}

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

/*
 * Event handling functions
 */
// See if an event has occured
void tixel_poll_event(Tixel* self, TixelEvent* event) {
    // Reset event struct
    memset(event, 0, sizeof(TixelEvent));

    // Poll for keyboard
    read(STDIN_FILENO, &event->key, 1);
    if (event->key != 0)
        event->type = TIXEL_EVENT_TYPE_KEY_PRESS; 

    // Check if quit has been requested
    if (event->key == self->quit_key)
        event->type = TIXEL_EVENT_TYPE_QUIT;

    // TODO : mouse events
}

#endif // TIXEL_IMPLEMENTATION

#endif // TIXEL_H
