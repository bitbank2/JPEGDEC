//
// JPEG Decoder planar YUV420 output test
//
// Copyright 2020 BitBank Software, Inc. All Rights Reserved.
// Licensed under the Apache License, Version 2.0 (the "License");
//
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/JPEGDEC.h"
#include "../test_images/tulips.h"

#define GUARD_SIZE 64
#define GUARD_VALUE 0xa5

static int guard_ok(const uint8_t *p, size_t size)
{
    size_t i;

    for (i=0; i<GUARD_SIZE; i++) {
        if (p[size+i] != GUARD_VALUE) return 0;
    }
    return 1;
}

static uint32_t checksum(const uint8_t *p, size_t size)
{
    uint32_t sum = 0;
    size_t i;

    for (i=0; i<size; i++) sum = (sum * 33U) ^ p[i];
    return sum;
}

static int test_image(uint8_t *jpeg_data, int jpeg_size, int width, int height)
{
    JPEGIMAGE jpeg = {0};
    int chroma_width = (width + 1) / 2;
    int chroma_height = (height + 1) / 2;
    int y_pitch = width + 3;
    int chroma_pitch = chroma_width + 5;
    size_t y_size = (size_t)y_pitch * height;
    size_t chroma_size = (size_t)chroma_pitch * chroma_height;
    uint8_t *y = malloc(y_size + GUARD_SIZE);
    uint8_t *cb = malloc(chroma_size + GUARD_SIZE);
    uint8_t *cr = malloc(chroma_size + GUARD_SIZE);
    int result = 1;

    if (y == NULL || cb == NULL || cr == NULL) {
        fprintf(stderr, "allocation failed\n");
        result = 0;
        goto out;
    }
    memset(y, GUARD_VALUE, y_size + GUARD_SIZE);
    memset(cb, GUARD_VALUE, chroma_size + GUARD_SIZE);
    memset(cr, GUARD_VALUE, chroma_size + GUARD_SIZE);

    if (JPEG_openRAM(&jpeg, jpeg_data, jpeg_size, NULL) == 0) {
        fprintf(stderr, "JPEG_openRAM failed: %d\n", JPEG_getLastError(&jpeg));
        result = 0;
        goto out;
    }
    if (JPEG_getWidth(&jpeg) != width || JPEG_getHeight(&jpeg) != height) {
        fprintf(stderr, "unexpected dimensions: %dx%d\n", JPEG_getWidth(&jpeg),
                JPEG_getHeight(&jpeg));
        result = 0;
        goto out;
    }
    if (JPEG_decodeYUV420P(&jpeg, y, y_pitch, cb, chroma_pitch, cr, chroma_pitch) == 0) {
        fprintf(stderr, "JPEG_decodeYUV420P failed: %d\n", JPEG_getLastError(&jpeg));
        result = 0;
        goto out;
    }
    if (!guard_ok(y, y_size) || !guard_ok(cb, chroma_size) ||
        !guard_ok(cr, chroma_size)) {
        fprintf(stderr, "output guard overwritten for %dx%d image\n", width, height);
        result = 0;
        goto out;
    }
    if (checksum(y, y_size) == 0 || checksum(cb, chroma_size) == 0 ||
        checksum(cr, chroma_size) == 0) {
        fprintf(stderr, "empty YUV output for %dx%d image\n", width, height);
        result = 0;
        goto out;
    }

out:
    JPEG_close(&jpeg);
    free(y);
    free(cb);
    free(cr);
    return result;
}

static uint8_t *load_file(const char *name, int *size)
{
    FILE *f = fopen(name, "rb");
    uint8_t *data;
    long len;

    if (f == NULL) return NULL;
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return NULL;
    }
    len = ftell(f);
    if (len <= 0 || fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        return NULL;
    }
    data = malloc((size_t)len);
    if (data == NULL || fread(data, 1, (size_t)len, f) != (size_t)len) {
        free(data);
        fclose(f);
        return NULL;
    }
    fclose(f);
    *size = (int)len;
    return data;
}

int main(void)
{
    uint8_t *odd_jpeg;
    int odd_size;

    if (!test_image((uint8_t *)tulips, sizeof(tulips), 640, 480)) return 1;

    odd_jpeg = load_file("../demo.jpg", &odd_size);
    if (odd_jpeg == NULL) {
        fprintf(stderr, "failed to load ../demo.jpg\n");
        return 1;
    }
    if (!test_image(odd_jpeg, odd_size, 945, 707)) {
        free(odd_jpeg);
        return 1;
    }
    free(odd_jpeg);

    printf("YUV420P output passed even and odd-size tests\n");
    return 0;
}
