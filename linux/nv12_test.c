//
// JPEG Decoder NV12 output test
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

static int test_image(uint8_t *jpeg_data, int jpeg_size, int width, int height)
{
    JPEGIMAGE jpeg = {0};
    int chroma_width = (width + 1) / 2;
    int chroma_height = (height + 1) / 2;
    int y_pitch = width + 3;
    int chroma_pitch = chroma_width + 5;
    int uv_pitch = chroma_width * 2 + 7;
    size_t y_size = (size_t)y_pitch * height;
    size_t chroma_size = (size_t)chroma_pitch * chroma_height;
    size_t uv_size = (size_t)uv_pitch * chroma_height;
    uint8_t *y_planar = malloc(y_size + GUARD_SIZE);
    uint8_t *cb = malloc(chroma_size + GUARD_SIZE);
    uint8_t *cr = malloc(chroma_size + GUARD_SIZE);
    uint8_t *y_nv12 = malloc(y_size + GUARD_SIZE);
    uint8_t *uv = malloc(uv_size + GUARD_SIZE);
    int i, j;
    int result = 1;

    if (y_planar == NULL || cb == NULL || cr == NULL || y_nv12 == NULL || uv == NULL) {
        fprintf(stderr, "allocation failed\n");
        result = 0;
        goto out;
    }
    memset(y_planar, GUARD_VALUE, y_size + GUARD_SIZE);
    memset(cb, GUARD_VALUE, chroma_size + GUARD_SIZE);
    memset(cr, GUARD_VALUE, chroma_size + GUARD_SIZE);
    memset(y_nv12, GUARD_VALUE, y_size + GUARD_SIZE);
    memset(uv, GUARD_VALUE, uv_size + GUARD_SIZE);

    if (JPEGDEC_openRAM(&jpeg, jpeg_data, jpeg_size, NULL) == 0 ||
        JPEGDEC_decodeYUV420P(&jpeg, y_planar, y_pitch, cb, chroma_pitch,
                           cr, chroma_pitch) == 0) {
        fprintf(stderr, "planar decode failed: %d\n", JPEGDEC_getLastError(&jpeg));
        result = 0;
        goto out;
    }
    JPEGDEC_close(&jpeg);

    if (JPEGDEC_openRAM(&jpeg, jpeg_data, jpeg_size, NULL) == 0 ||
        JPEGDEC_decodeNV12(&jpeg, y_nv12, y_pitch, uv, uv_pitch) == 0) {
        fprintf(stderr, "NV12 decode failed: %d\n", JPEGDEC_getLastError(&jpeg));
        result = 0;
        goto out;
    }
    if (!guard_ok(y_nv12, y_size) || !guard_ok(uv, uv_size)) {
        fprintf(stderr, "NV12 guard overwritten for %dx%d image\n", width, height);
        result = 0;
        goto out;
    }

    for (i=0; i<height; i++) {
        if (memcmp(&y_planar[i*y_pitch], &y_nv12[i*y_pitch], (size_t)width) != 0) {
            fprintf(stderr, "Y plane mismatch on row %d\n", i);
            result = 0;
            goto out;
        }
    }
    for (i=0; i<chroma_height; i++) {
        for (j=0; j<chroma_width; j++) {
            if (uv[i*uv_pitch+j*2] != cb[i*chroma_pitch+j] ||
                uv[i*uv_pitch+j*2+1] != cr[i*chroma_pitch+j]) {
                fprintf(stderr, "UV mismatch at %d,%d\n", j, i);
                result = 0;
                goto out;
            }
        }
    }

out:
    JPEGDEC_close(&jpeg);
    free(uv);
    free(y_nv12);
    free(cr);
    free(cb);
    free(y_planar);
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

    printf("NV12 output matches YUV420P for even and odd-size tests\n");
    return 0;
}
