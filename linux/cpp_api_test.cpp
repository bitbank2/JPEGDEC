//
// JPEG Decoder C++ API test
//
// Copyright 2020 BitBank Software, Inc. All Rights Reserved.
// Licensed under the Apache License, Version 2.0 (the "License");
//
#include <stdint.h>
#include <stdio.h>

#include "../src/JPEGDEC.h"
#include "../test_images/tulips.h"

static uint32_t draw_count;
static uint32_t pixel_count;

static int JPEGDraw(JPEGDRAW *pDraw)
{
    if (pDraw == NULL || pDraw->pPixels == NULL || pDraw->iWidthUsed < 1 ||
        pDraw->iHeight < 1 || pDraw->x < 0 || pDraw->y < 0 ||
        pDraw->x + pDraw->iWidthUsed > 640 || pDraw->y + pDraw->iHeight > 480) {
        return 0;
    }

    draw_count++;
    pixel_count += (uint32_t)pDraw->iWidthUsed * (uint32_t)pDraw->iHeight;
    return 1;
}

int main(void)
{
    JPEGDEC jpeg;

    if (jpeg.openRAM((uint8_t *)tulips, sizeof(tulips), JPEGDraw) == 0) {
        fprintf(stderr, "openRAM failed: %d\n", jpeg.getLastError());
        return 1;
    }
    if (jpeg.getWidth() != 640 || jpeg.getHeight() != 480) {
        fprintf(stderr, "unexpected dimensions: %dx%d\n", jpeg.getWidth(), jpeg.getHeight());
        return 1;
    }

    jpeg.setPixelType(JPEGD_PIXEL_RGB565_LE);
    if (jpeg.getPixelType() != JPEGD_PIXEL_RGB565_LE) {
        fprintf(stderr, "unexpected pixel type\n");
        return 1;
    }
    if (jpeg.decode(0, 0, 0) == 0) {
        fprintf(stderr, "decode failed: %d\n", jpeg.getLastError());
        return 1;
    }
    jpeg.close();

    if (draw_count == 0 || pixel_count != 640U * 480U) {
        fprintf(stderr, "unexpected callback coverage: %u draws, %u pixels\n",
                draw_count, pixel_count);
        return 1;
    }

    printf("C++ API decoded 640x480 JPEG in %u blocks\n", draw_count);
    return 0;
}
