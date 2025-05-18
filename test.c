#define TIXEL_IMPLEMENTATION
#include "tixel.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../stb/stb_image.h"

int main(void) {
    Tixel t = {0};
    tixel_init(&t, 200, 200);
    tixel_clear(&t, (TixelColor){255, 0, 0});
   
    // Load test image
    int w, h, c;
    unsigned char* buffer = stbi_load("sun2.png", &w, &h, &c, 3);

    TixelEvent ev;
    while (ev.type != TIXEL_EVENT_TYPE_QUIT) {
        // Draw a blue pattern
        for (int y = 0; y < 200; y++)
            for (int x = 0; x < 200; x++)
                if (y % 2 == 0)
                    tixel_draw_pixel(&t, x, y, (TixelColor){0, 0, 255});

        tixel_draw_buffer(&t, 0, 0, w, h, buffer);
        tixel_draw_triangle(&t, 10, 10, 40, 50, 20, 50, (TixelColor){255, 0, 255});
        tixel_draw_triangle_lines(&t, 50, 10, 80, 50, 60, 50, (TixelColor){255, 0, 255});
        tixel_show(&t);
        tixel_poll_event(&t, &ev);
    }

    tixel_deinit(&t);
    stbi_image_free(buffer);
    return 0;
}
