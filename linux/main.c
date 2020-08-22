// JPEG perf test
// Written by Larry Bank
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "../src/JPEGDEC.h"
#include "../test_images/tulips.h"

JPEGIMAGE jpg;

long micros()
{
long iTime;
struct timespec res;

    clock_gettime(CLOCK_MONOTONIC, &res);
    iTime = 1000000*res.tv_sec + res.tv_nsec/1000;

    return iTime;
} /* micros() */

void JPEGDraw(JPEGDRAW *pDraw)
{
} /* JPEGDraw() */

int main(int argc, char *argv[])
{
long lTime;

    if (JPEG_openRAM(&jpg, (uint8_t *)tulips, sizeof(tulips), JPEGDraw))
    {
        lTime = micros();
	if (JPEG_decode(&jpg, 0, 0, 0)) { // full size
	    lTime = micros() - lTime;
            printf("full sized decode in %d us\n", (int)lTime);
	}
	JPEG_close(&jpg);
    }

    if (JPEG_openRAM(&jpg, (uint8_t *)tulips, sizeof(tulips), JPEGDraw))
    {
        lTime = micros();
        if (JPEG_decode(&jpg, 0, 0, JPEG_SCALE_HALF)) { // 1/2 size
            lTime = micros() - lTime;
            printf("half sized decode in %d us\n", (int)lTime);
        }
        JPEG_close(&jpg);
    }
    if (JPEG_openRAM(&jpg, (uint8_t *)tulips, sizeof(tulips), JPEGDraw))
    {
        lTime = micros();
        if (JPEG_decode(&jpg, 0, 0, JPEG_SCALE_QUARTER)) { // 1/4 size
            lTime = micros() - lTime;
            printf("quarter sized decode in %d us\n", (int)lTime);
        }
        JPEG_close(&jpg);
    }
    if (JPEG_openRAM(&jpg, (uint8_t *)tulips, sizeof(tulips), JPEGDraw))
    {
        lTime = micros();
        if (JPEG_decode(&jpg, 0, 0, JPEG_SCALE_EIGHTH)) { // 1/8 size
            lTime = micros() - lTime;
            printf("eighth sized decode in %d us\n", (int)lTime);
        }
        JPEG_close(&jpg);
    }

    return 0;
} /* main() */
