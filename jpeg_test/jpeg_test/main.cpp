//
//  main.cpp
//  jpeg_test
//
//  Created by Laurence Bank on 8/2/20.
//  Copyright © 2020 Laurence Bank. All rights reserved.
//
#include "JPEGDEC.h"

#include "../../test_images/sciopero.h"

JPEGDEC jpeg;

void JPEGDraw(JPEGDRAW *pDraw)
{
} /* JPEGDraw() */

int main(int argc, const char * argv[]) {

    printf("Starting JPEG decoder...\n");
    jpeg.begin(BIG_ENDIAN_PIXELS);
    if (jpeg.open((uint8_t *)sciopero, sizeof(sciopero), JPEGDraw))
    {
        printf("Successfully opened JPEG\n");
        printf("Image size: %d x %d\n", jpeg.getWidth(), jpeg.getHeight());
        if (jpeg.decode())
        {
            printf("Successfully decoded image\n");
        }
        jpeg.close();
    }
    return 0;
}
