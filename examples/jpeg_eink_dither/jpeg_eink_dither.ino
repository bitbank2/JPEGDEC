#include "epaper42.h"
#include <JPEGDEC.h>
#include "../test_images/squirrel.h"

JPEGDEC jpeg;
static uint8_t image[400*8];
Epd epd;
int iWidth, iHeight;

void JPEGDraw(JPEGDRAW *pDraw)
{
  int iPitch;
  iPitch = (pDraw->iWidth+3)/4;
  epd.SetPartialWindow4Gray((uint8_t *)pDraw->pPixels, pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight, iPitch);
} /* JPEGDraw() */

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  if (epd.Init() != 0) {
    Serial.print("e-Paper init failed");
    return;
  }
  epd.Init_4Gray();
  epd.ClearFrame();

} /* setup() */

void loop() {

    if (jpeg.openFLASH((uint8_t *)squirrel, sizeof(squirrel), JPEGDraw)) {
      Serial.println("JPEG opened successfully");
      jpeg.setPixelType(TWO_BIT_DITHERED);
      if (jpeg.decodeDither(image, JPEG_SCALE_HALF)) {
        Serial.println("JPEG decoded successfully");
      }
      jpeg.close();
      epd.DisplayFrame4Gray();
    }
    
  while (1)
  {};

}
