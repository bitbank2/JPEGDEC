//
//  main.cpp
//  JPEGDEC_Test
//
//  Created by Laurence Bank on 2/6/25.
//

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include "../../../src/JPEGDEC.cpp"
#include "../../../test_images/tulips.h" // 640x480 56k byte test image
#include "corrupt1.h" // invalid header offsets
#include "corrupt2.h" // global buffer overflow 1
#include "corrupt3.h" // global buffer overflow 2
#include "corrupt4.h" // FPE 1
#include "corrupt5.h" // FPE 2
#define LOOP_COUNT 100

JPEGDEC jpg;
int x1, y1, x2, y2;
int iWidth, iHeight;
uint16_t *pOldPixels;
int bDMAFailed;
//
// Return the current time in microseconds
//
int Micros(void)
{
int iTime;
struct timespec res;
                        
    clock_gettime(CLOCK_MONOTONIC, &res);
    iTime = (int)(1000000*res.tv_sec + res.tv_nsec/1000);
                
    return iTime;
} /* Micros() */

//
// Simple logging print
//
void JPEGLOG(int line, char *string, const char *result)
{
    printf("Line: %d: msg: %s%s\n", line, string, result);
} /* JPEGLOG() */

// Draw callback
int JPEGDraw(JPEGDRAW *pDraw)
{
    if (pDraw->pPixels == pOldPixels) {
        bDMAFailed = 1; // DMA option should toggle the buffer pointer with each callback
    }
    pOldPixels = pDraw->pPixels;
    
    // record the max extents of the pixel positions
    if (pDraw->x < x1) x1 = pDraw->x;
    if (pDraw->y < y1) y1 = pDraw->y;
    if (pDraw->x + pDraw->iWidthUsed -1 > x2) x2 = pDraw->x + pDraw->iWidthUsed -1;
    if (pDraw->y + pDraw->iHeight-1 > y2) y2 = pDraw->y + pDraw->iHeight-1;
    return 1; // continue to decode
} /* JPEGDraw() */

int main(int argc, const char * argv[]) {
    int i, rc, iTime1, iTime2;
    int w, h;
    uint8_t *pFuzzData;
    char *szTestName;
    const char *szStart = " - START";
    // Test 1 - Decode to the correct full image dimensions
    x1 = y1 = 1000;
    x2 = y2 = 0;
    szTestName = (char *)"JPEG full image decode";
    JPEGLOG(__LINE__, szTestName, szStart);
    if (jpg.openFLASH((uint8_t *)tulips, sizeof(tulips), JPEGDraw)) {
        if (jpg.decode(0,0,0)) { // full sized decode
            iWidth = jpg.getWidth();
            iHeight = jpg.getHeight();
            w = 1 + x2 - x1; h = 1 + y2 - y1;
            if (iHeight == h && iWidth == w) {
                JPEGLOG(__LINE__, szTestName, " - PASSED");
            } else {
                JPEGLOG(__LINE__, szTestName, " - FAILED");
                if (iHeight != h) {
                    printf("Image Height = %d, decoded Height = %d\n", iHeight, h);
                } else {
                    printf("Image Width = %d, decoded Width = %d\n", iWidth, w);
                }
            }
        } else { // decode failed
            JPEGLOG(__LINE__, szTestName, " - decode failed");
        }
        jpg.close();
    } else {
        JPEGLOG(__LINE__, szTestName, " - open failed");
    }
    // Test 2 - Decode to the correct cropped dimensions
    szTestName = (char *)"JPEG full image decode";
    JPEGLOG(__LINE__, szTestName, szStart);
    if (jpg.openFLASH((uint8_t *)tulips, sizeof(tulips), JPEGDraw)) {
        jpg.setCropArea(50, 50, 125, 170); // purposely using coordinates which will get adjusted
        jpg.getCropArea(&x1, &y1, &iWidth, &iHeight);
        x1 = y1 = 1000;
        x2 = y2 = 0;
        if (jpg.decode(0,0,0)) { // cropped decode
            w = 1 + x2 - x1; h = 1 + y2 - y1;
            if (iHeight == h && iWidth == w) {
                JPEGLOG(__LINE__, szTestName, " - PASSED");
            } else {
                JPEGLOG(__LINE__, szTestName, " - FAILED");
                if (iHeight != h) {
                    printf("Image Height = %d, decoded Height = %d\n", iHeight, h);
                } else {
                    printf("Image Width = %d, decoded Width = %d\n", iWidth, w);
                }
            }
        } else { // decode failed
            JPEGLOG(__LINE__, szTestName, " - decode failed");
        }
        jpg.close();
    } else {
        JPEGLOG(__LINE__, szTestName, " - open failed");
    }
    // Test 3 - Decode a color image as grayscale (faster)
    szTestName = (char *)"JPEG color->gray image decode";
    JPEGLOG(__LINE__, szTestName, szStart);
    iTime1 = Micros();
    for (i=0; i<LOOP_COUNT; i++) { // need to do it many times to get an accurate time
        jpg.openFLASH((uint8_t *)tulips, sizeof(tulips), JPEGDraw);
        jpg.decode(0,0,JPEG_LUMA_ONLY); // grayscale decode
        jpg.close();
    }
    iTime1 = Micros() - iTime1; // total decode time in microseconds
    iTime2 = Micros();
    for (i=0; i<LOOP_COUNT; i++) {
        jpg.openFLASH((uint8_t *)tulips, sizeof(tulips), JPEGDraw);
        jpg.decode(0,0,0);
        jpg.close();
    }
    iTime2 = Micros() - iTime2;
    printf("%d iterations: color decode - %d us, grayscale decode - %d us\n", LOOP_COUNT, iTime2, iTime1);
    if (iTime1 <= (iTime2 * 5)/8) { // it should be at least 40% faster
        JPEGLOG(__LINE__, szTestName, " - PASSED");
    } else {
        JPEGLOG(__LINE__, szTestName, " - FAILED");
    }
    // Test 4 - open a corrupt image without crashing
    szTestName = (char *)"JPEG purposely corrupt image 1";
    JPEGLOG(__LINE__, szTestName, szStart);
    if (jpg.openFLASH((uint8_t *)corrupt1, sizeof(corrupt1), JPEGDraw)) {
        jpg.decode(0,0,0);
        jpg.close();
    }
    JPEGLOG(__LINE__, szTestName, " - PASSED");

    // Test 5 - open a corrupt image without crashing
    szTestName = (char *)"JPEG purposely corrupt image 2";
    JPEGLOG(__LINE__, szTestName, szStart);
    if (jpg.openFLASH((uint8_t *)corrupt2, sizeof(corrupt2), JPEGDraw)) {
        jpg.decode(0,0,0);
        jpg.close();
    }
    JPEGLOG(__LINE__, szTestName, " - PASSED");

    // Test 6 - open a corrupt image without crashing
    szTestName = (char *)"JPEG purposely corrupt image 3";
    JPEGLOG(__LINE__, szTestName, szStart);
    if (jpg.openFLASH((uint8_t *)corrupt3, sizeof(corrupt3), JPEGDraw)) {
        jpg.decode(0,0,0);
        jpg.close();
    }
    JPEGLOG(__LINE__, szTestName, " - PASSED");

    // Test 7 - open a corrupt image without crashing
    szTestName = (char *)"JPEG purposely corrupt image 4";
    JPEGLOG(__LINE__, szTestName, szStart);
    if (jpg.openFLASH((uint8_t *)FPE1, sizeof(FPE1), JPEGDraw)) {
        jpg.decode(0,0,0);
        jpg.close();
    }
    JPEGLOG(__LINE__, szTestName, " - PASSED");

    // Test 8 - open a corrupt image without crashing
    szTestName = (char *)"JPEG purposely corrupt image 5";
    JPEGLOG(__LINE__, szTestName, szStart);
    if (jpg.openFLASH((uint8_t *)FPE2, sizeof(FPE2), JPEGDraw)) {
        jpg.decode(0,0,0);
        jpg.close();
    }
    JPEGLOG(__LINE__, szTestName, " - PASSED");

    // Test 9 - confirm DMA option is properly providing a ping-pong buffer
    bDMAFailed = 0;
    pOldPixels = NULL;
    szTestName = (char *)"JPEG DMA ping-pong buffer";
    JPEGLOG(__LINE__, szTestName, szStart);
    if (jpg.openFLASH((uint8_t *)tulips, sizeof(tulips), JPEGDraw)) {
        jpg.decode(0,0,JPEG_USES_DMA);
        jpg.close();
    }
    if (!bDMAFailed) {
        JPEGLOG(__LINE__, szTestName, " - PASSED");
    } else {
        JPEGLOG(__LINE__, szTestName, " - FAILED");
    }
    // FUZZ testing
    // Randomize the input data (file header and compressed data) and confirm that the library returns an error code
    // and doesn't have an invalid pointer exception
    printf("Begin fuzz testing...\n");
    szTestName = (char *)"Single Byte Sequential Corruption Test";
    pFuzzData = (uint8_t *)malloc(sizeof(tulips));
    JPEGLOG(__LINE__, szTestName, szStart);
    // We don't need to corrupt the file all the way to the end because it will take a loooong time
    // The header is the main area where corruption can cause erratic behavior
    for (i=0; i<2000; i++) { // corrupt each byte one at a time by inverting it
        memcpy(pFuzzData, tulips, sizeof(tulips)); // start with the valid data
        pFuzzData[i] = ~pFuzzData[i]; // invert the bits of this byte
        if (jpg.openFLASH((uint8_t *)tulips, sizeof(tulips), JPEGDraw)) { // the JPEG header may be rejected
            rc = jpg.decode(0,0,0);
            jpg.close();
        }
    } // for each test
    JPEGLOG(__LINE__, szTestName, " - PASSED");
    szTestName = (char *)"Multi-Byte Random Corruption Test";
    JPEGLOG(__LINE__, szTestName, szStart);
    for (i=0; i<1000; i++) { // 1000 iterations of random spots in the file to corrupt with random values
        int iOffset;
        memcpy(pFuzzData, tulips, sizeof(tulips)); // start with the valid data
        iOffset = rand() % sizeof(tulips);
        pFuzzData[iOffset] = (uint8_t)rand();
        iOffset = rand() % sizeof(tulips); // corrupt 2 spots just for good measure
        pFuzzData[iOffset] = (uint8_t)rand();
        if (jpg.openFLASH((uint8_t *)tulips, sizeof(tulips), JPEGDraw)) { // the JPEG header may be rejected
            rc = jpg.decode(0,0,0);
            jpg.close();
        }
    } // for each test
    JPEGLOG(__LINE__, szTestName, " - PASSED");

    free(pFuzzData);
#ifdef FUTURE
    // Test 5
    // Test that the performance of requesting a partial decode is not the same as a full decode
    // In other words, see if asking for 1/2 of the image to be decoded takes about 1/2 the time
    // of asking for the full image decode
    //
#define LOOP_COUNT 1000
    iOldY = -1;
    iLineCount = 0;
    szTestName = (char *)"TIFF crop window perf test";
    JPEGLOG(__LINE__, szTestName, szStart);
    iTime1 = MilliTime(); // start time
    for (int i=0; i<LOOP_COUNT; i++) { // needs to run many times on the Mac to measure a few milliseconds :)
        if (g4.openTIFF((uint8_t *)weather_icons, (int)sizeof(weather_icons), TIFFDraw)) {
            iHeight = g4.getHeight();
            rc = g4.decode(); // decode the whole image
            g4.close();
            if (!(rc == TIFF_SUCCESS && iHeight == iLineCount)) {
                JPEGLOG(__LINE__, szTestName, " - FAILED");
                printf("iHeight = %d, lines = %d\n", iHeight, iLineCount);
                i = LOOP_COUNT; // stop running
            }
        } else {
            JPEGLOG(__LINE__, szTestName, " - open failed");
            i = LOOP_COUNT; // stop running immediately
        }
    } // for i
    iTime1 = MilliTime() - iTime1; // get the total time in milliseconds
    // Now ask the library to decode only the top half of the image
    iTime2 = MilliTime(); // start time 2
    iOldY = -1;
    iLineCount = 0;
    for (i=0; i<LOOP_COUNT; i++) { // needs to run many times on the Mac to measure a few milliseconds :)
        if (g4.openTIFF((uint8_t *)weather_icons, (int)sizeof(weather_icons), TIFFDraw)) {
            iHeight = g4.getHeight();
            g4.setDrawParameters(1.0f, TIFF_PIXEL_1BPP, 0, 0, g4.getWidth(), iHeight/2, NULL);
            rc = g4.decode(); // decode the top half of the image
            g4.close();
            if (!(rc == TIFF_SUCCESS && iHeight/2 == iLineCount)) {
                JPEGLOG(__LINE__, szTestName, " - FAILED");
                printf("iHeight = %d, lines = %d\n", iHeight, iLineCount);
                i = LOOP_COUNT; // stop running
            }
        } else {
            JPEGLOG(__LINE__, szTestName, " - open failed");
            i = LOOP_COUNT; // stop running immediately
        }
    } // for i
    iTime2 = MilliTime() - iTime2; // get the total time in milliseconds for decoding 1/2 of the image
    // The time may not be exactly half because of the time to parse the file header, but it should certainly be at least 25% less
    i = (iTime1 * 3)/4;
    if (iTime2 < i) {
        JPEGLOG(__LINE__, szTestName, " - PASSED\n");
        printf("Full decode time (%d iterations) = %d ms\n", LOOP_COUNT, iTime1);
        printf("Top half decode time (%d iterations) = %d ms\n", LOOP_COUNT, iTime2);
    } else {
        JPEGLOG(__LINE__, szTestName, " - FAILED");
    }
#endif // FUTURE
    return 0;
}
