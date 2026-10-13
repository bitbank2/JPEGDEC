/*
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>
#include <stdio.h>

#include "../src/JPEGDEC.h"
#include "progressive_test_image.h"

static int draw_count;

static int draw(JPEGDRAW *draw_info)
{
    if (draw_info->pPixels == NULL || draw_info->iWidth <= 0 || draw_info->iHeight <= 0) {
        return 0;
    }

    draw_count++;
    return 1;
}

int main(void)
{
    JPEGIMAGE jpeg;
    int result;

    result = JPEG_openRAM(&jpeg, (uint8_t *)progressive_test_image,
                          sizeof(progressive_test_image), draw);
    if (result == 0) {
        fprintf(stderr, "failed to open progressive JPEG: %d\n",
                JPEG_getLastError(&jpeg));
        return 1;
    }
    if (JPEG_getJPEGType(&jpeg) != JPEG_MODE_PROGRESSIVE) {
        fprintf(stderr, "fixture is not progressive\n");
        JPEG_close(&jpeg);
        return 1;
    }

    JPEG_setCropArea(&jpeg, 16, 16, 32, 32);
    result = JPEG_decode(&jpeg, 0, 0, 0);
    if (result == 0) {
        fprintf(stderr, "cropped progressive decode failed: %d\n",
                JPEG_getLastError(&jpeg));
        JPEG_close(&jpeg);
        return 1;
    }
    JPEG_close(&jpeg);

    if (draw_count == 0) {
        fprintf(stderr, "cropped progressive decode produced no output\n");
        return 1;
    }

    printf("Cropped progressive JPEG decoded in %d blocks\n", draw_count);
    return 0;
}
