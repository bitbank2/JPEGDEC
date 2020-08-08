//
//  main.cpp
//  jpeg_test
//
//  Created by Laurence Bank on 8/2/20.
//  Copyright © 2020 Laurence Bank. All rights reserved.
//
#include "JPEGDEC.h"

#include "../../test_images/f6t.h"
#include "../../test_images/gray_road.h"

JPEGDEC jpeg;

void JPEGDraw(JPEGDRAW *pDraw)
{
    printf("x,y=%d,%d, p[0] = 0x%04x\n", pDraw->x, pDraw->y, pDraw->pPixels[0]);
} /* JPEGDraw() */

int main(int argc, const char * argv[]) {

#ifdef BOGUS
    int i;
    uint8_t ucRangeTable[1024];
    // DEBUG
    /* Create a range clipping table for results of multiplications */
    for (i=0; i<128; i++)
    {
        ucRangeTable[i] = (unsigned char)(0x80 + i);
        ucRangeTable[i+896] = (unsigned char)i;
    }
    for (i=0; i<384; i++)
    {
        ucRangeTable[i+128] = 0xff;
        ucRangeTable[i+512] = 0;
    }
    for (i=0; i<1024; i++)
    {
        printf("0x%02x,", ucRangeTable[i]);
        if ((i & 15) == 15)
            printf("\n");
    }
#endif // BOGUS
    
    printf("Starting JPEG decoder...\n");
//    if (jpeg.open((uint8_t *)f6t, sizeof(f6t), JPEGDraw))
    if (jpeg.open((uint8_t *)gray_road, sizeof(gray_road), JPEGDraw))
    {
//        printf("Successfully opened JPEG\n");
//        printf("Image size: %d x %d, orientation: %d, bpp: %d\n", jpeg.getWidth(), jpeg.getHeight(), jpeg.getOrientation(), jpeg.getBpp());
        if (jpeg.decode(0))
        {
//            printf("Successfully decoded image\n");
        }
        jpeg.close();
    }
    return 0;
}
