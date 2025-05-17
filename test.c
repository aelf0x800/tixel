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
    unsigned char* buffer = stbi_load("sun.png", &w, &h, &c, 3);

    printf("%d, %d, %d\n", buffer[300], buffer[104], buffer[105]); 

    // Draw a blue pattern
    for (int y = 0; y < 200; y++)
        for (int x = 0; x < 200; x++)
            if (y % 2 == 0)
                tixel_draw_pixel(&t, x, y, (TixelColor){0, 0, 255});

    // Draw image
    tixel_draw_buffer(&t, 0, 0, w, h, buffer);

    tixel_show(&t);
    sleep(3);
    tixel_deinit(&t);
    stbi_image_free(buffer);
    return 0;
}
