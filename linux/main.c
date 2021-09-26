// JPEG perf test
// Written by Larry Bank
// 
// Will open an arbitrary JPEG file if passed on the command line
// or will use the sample image (tulips)
//
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "../src/JPEGDEC.h"
#include "../src/jpeg.inl"
#include "../test_images/tulips.h"

JPEGIMAGE jpg;
uint8_t ucDitherBuffer[1024 * 16];

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
int rc;

    printf("JPEG decoder demo\n");
    printf("Run without parameters to test in-memory decoding\n");
    printf("Or pass a filename\n\n");

    if (argc == 2)
        rc = JPEG_openFile(&jpg, argv[1], JPEGDraw);
    else
	rc = JPEG_openRAM(&jpg, (uint8_t *)tulips, sizeof(tulips), JPEGDraw);
    if (rc)
    {
        lTime = micros();
        jpg.pDitherBuffer = ucDitherBuffer;
        jpg.ucPixelType = FOUR_BIT_DITHERED;
	if (JPEG_decode(&jpg, 0, 0, 0)) { // full size
	    lTime = micros() - lTime;
            printf("full sized decode in %d us\n", (int)lTime);
	}
	else
	{
            printf("Decode failed, last error = %d\n", JPEG_getLastError(&jpg));
	    return 0;
	}
	JPEG_close(&jpg);
    }
    else
    {
	printf("open() failed, last error = %d\n", JPEG_getLastError(&jpg));
	return 0;
    }

    if (argc == 2)
        rc = JPEG_openFile(&jpg, argv[1], JPEGDraw);
    else
        rc = JPEG_openRAM(&jpg, (uint8_t *)tulips, sizeof(tulips), JPEGDraw);
    if (rc)
    {
        lTime = micros();
        if (JPEG_decode(&jpg, 0, 0, JPEG_SCALE_HALF)) { // 1/2 size
            lTime = micros() - lTime;
            printf("half sized decode in %d us\n", (int)lTime);
        }
        JPEG_close(&jpg);
    }
    if (argc == 2)
        rc = JPEG_openFile(&jpg, argv[1], JPEGDraw);
    else
        rc = JPEG_openRAM(&jpg, (uint8_t *)tulips, sizeof(tulips), JPEGDraw);
    if (rc)
    {
        lTime = micros();
        if (JPEG_decode(&jpg, 0, 0, JPEG_SCALE_QUARTER)) { // 1/4 size
            lTime = micros() - lTime;
            printf("quarter sized decode in %d us\n", (int)lTime);
        }
        JPEG_close(&jpg);
    }
    if (argc == 2)
        rc = JPEG_openFile(&jpg, argv[1], JPEGDraw);
    else
        rc = JPEG_openRAM(&jpg, (uint8_t *)tulips, sizeof(tulips), JPEGDraw);
    if (rc)
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
