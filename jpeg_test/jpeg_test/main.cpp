//
//  main.cpp
//  jpeg_test
//
//  Created by Laurence Bank on 8/2/20.
//  Copyright © 2020 Laurence Bank. All rights reserved.
//
#include "JPEGDEC.h"

#include "../../test_images/f6t.h"

JPEGDEC jpeg;

void JPEGDraw(JPEGDRAW *pDraw)
{
} /* JPEGDraw() */

int main(int argc, const char * argv[]) {

    printf("Starting JPEG decoder...\n");
    if (jpeg.open((uint8_t *)f6t, sizeof(f6t), JPEGDraw))
    {
        printf("Successfully opened JPEG\n");
        printf("Image size: %d x %d, orientation: %d, bpp: %d\n", jpeg.getWidth(), jpeg.getHeight(), jpeg.getOrientation(), jpeg.getBpp());
        if (jpeg.decode(0))
        {
            printf("Successfully decoded image\n");
        }
        jpeg.close();
    }
    return 0;
}
