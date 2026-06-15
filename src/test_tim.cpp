#include "tim_decoder.h"
#include "tim_data.h"
#include <cstdio>

int main() {
    TimImage sonyTim, masayaTim;

    if (TimDecoder::Decode(tim_sony, sizeof(tim_sony), sonyTim)) {
        printf("Sony TIM: %d x %d, bpp=%d, palette=%zu colors\n",
               sonyTim.width, sonyTim.height, sonyTim.bpp, sonyTim.palette.size());

        // Scan row 20 (middle of second text row) for non-zero pixels
        printf("Row 20 pixel scan (non-zero ranges):\n");
        int y = 20;
        int startX = -1;
        for (int x = 0; x < sonyTim.width; x++) {
            // For 4bpp, 2 pixels per byte
            int byteIdx = (y * sonyTim.width + x) / 2;
            int nibble = (x % 2 == 0) ? (sonyTim.pixels[byteIdx] & 0x0F) : ((sonyTim.pixels[byteIdx] >> 4) & 0x0F);

            if (nibble != 0 && startX == -1) {
                startX = x;
            } else if (nibble == 0 && startX != -1) {
                printf("  Text region: x=%d to %d (width=%d)\n", startX, x-1, x-startX);
                startX = -1;
            }
        }
        if (startX != -1) {
            printf("  Text region: x=%d to %d (width=%d)\n", startX, sonyTim.width-1, sonyTim.width-startX);
        }
    }

    return 0;
}
