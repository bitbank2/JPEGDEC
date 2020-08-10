//
//  main.cpp
//  jpeg_test
//
//  Created by Laurence Bank on 8/2/20.
//  Copyright © 2020 Laurence Bank. All rights reserved.
//
#include "JPEGDEC.h"

//#include "../../test_images/f6t.h"
//#include "../../test_images/gray_road.h"
//#include "../../test_images/st_peters.h"
//#include "../../test_images/640x480.h"
#include "../../test_images/thumb_test.h"
JPEGDEC jpeg;

void JPEGDraw(JPEGDRAW *pDraw)
{
//    printf("x,y=%d,%d, p[0] = 0x%04x\n", pDraw->x, pDraw->y, pDraw->pPixels[0]);
} /* JPEGDraw() */

int main(int argc, const char * argv[]) {

    printf("jpeg structure size = %d\n", sizeof(JPEGIMAGE));
    
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
    for (int i=0; i<1; i++)
    {
        if (jpeg.open((uint8_t *)thumb_test, sizeof(thumb_test), JPEGDraw))
        {
            printf("Successfully opened JPEG\n");
            printf("Image size: %d x %d, orientation: %d, bpp: %d, subsample: 0x%02x\n", jpeg.getWidth(), jpeg.getHeight(), jpeg.getOrientation(), jpeg.getBpp(), jpeg.getSubSample());
            if (jpeg.hasThumb())
                printf("Thumbnail present: %d x %d\n", jpeg.getThumbWidth(), jpeg.getThumbHeight());
            if (jpeg.decode(0,0,JPEG_SCALE_QUARTER | JPEG_EXIF_THUMBNAIL))
            {
                printf("Successfully decoded image\n");
            }
            jpeg.close();
        }
    } // for i
    return 0;
}
