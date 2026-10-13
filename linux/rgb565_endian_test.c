/*
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "../src/JPEGDEC.h"
#include "../test_images/tulips.h"
#include "../test_images/zebra.h"

static int check_rgb565_endian(const uint8_t *image, size_t image_size, const char *name)
{
    JPEGIMAGE jpeg;
    uint8_t *le;
    uint8_t *be;
    size_t output_size;
    int height;
    int width;
    int result = 1;

    if (JPEGDEC_openRAM(&jpeg, (uint8_t *)image, (int)image_size, NULL) == 0) {
        fprintf(stderr, "%s: failed to open JPEG\n", name);
        return 0;
    }
    width = JPEGDEC_getWidth(&jpeg);
    height = JPEGDEC_getHeight(&jpeg);
    JPEGDEC_close(&jpeg);
    if (width <= 0 || height <= 0 || (size_t)width > SIZE_MAX / (size_t)height / 2U) {
        fprintf(stderr, "%s: invalid dimensions\n", name);
        return 0;
    }
    output_size = (size_t)width * (size_t)height * 2U;
    le = malloc(output_size);
    be = malloc(output_size);
    if (le == NULL || be == NULL) {
        fprintf(stderr, "%s: allocation failed\n", name);
        result = 0;
        goto cleanup;
    }

    if (JPEGDEC_openRAM(&jpeg, (uint8_t *)image, (int)image_size, NULL) == 0) {
        fprintf(stderr, "%s: failed to reopen JPEG for LE output\n", name);
        result = 0;
        goto cleanup;
    }
    JPEGDEC_setFramebuffer(&jpeg, le);
    JPEGDEC_setPixelType(&jpeg, JPEGD_PIXEL_RGB565_LE);
    if (JPEGDEC_decode(&jpeg, 0, 0, 0) == 0) {
        fprintf(stderr, "%s: LE decode failed\n", name);
        JPEGDEC_close(&jpeg);
        result = 0;
        goto cleanup;
    }
    JPEGDEC_close(&jpeg);

    if (JPEGDEC_openRAM(&jpeg, (uint8_t *)image, (int)image_size, NULL) == 0) {
        fprintf(stderr, "%s: failed to reopen JPEG for BE output\n", name);
        result = 0;
        goto cleanup;
    }
    JPEGDEC_setFramebuffer(&jpeg, be);
    JPEGDEC_setPixelType(&jpeg, JPEGD_PIXEL_RGB565_BE);
    if (JPEGDEC_decode(&jpeg, 0, 0, 0) == 0) {
        fprintf(stderr, "%s: BE decode failed\n", name);
        JPEGDEC_close(&jpeg);
        result = 0;
        goto cleanup;
    }
    JPEGDEC_close(&jpeg);

    for (size_t i = 0U; i < output_size; i += 2U) {
        if (le[i] != be[i + 1U] || le[i + 1U] != be[i]) {
            fprintf(stderr, "%s: endian mismatch at pixel %zu\n", name, i / 2U);
            result = 0;
            break;
        }
    }

cleanup:
    free(le);
    free(be);
    return result;
}

int main(void)
{
    if (!check_rgb565_endian(tulips, sizeof(tulips), "4:2:0") ||
        !check_rgb565_endian(zebra, sizeof(zebra), "4:4:4")) {
        return 1;
    }

    printf("RGB565 LE and BE outputs are byte-swapped equivalents\n");
    return 0;
}
