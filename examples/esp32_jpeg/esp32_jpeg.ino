//#include <SD.h>
#include <bb_spi_lcd.h>
#include "JPEGDEC.h"
//#include "test_images/gray_road.h"
//#include "test_images/f6t.h"
#include "test_images/thumb_test.h"

// Janzen Hub
#define CS_PIN 4
#define DC_PIN 12
#define LED_PIN 16
#define BUILTIN_SDCARD 0
#define RESET_PIN -1
#define MISO_PIN 19
#define MOSI_PIN 23
#define SCK_PIN 18

JPEGDEC jpeg;

//uint8_t ucTXBuf[1024];
static uint8_t *ucTXBuf;

//File file;
// pixel drawing callback
void drawMCU(JPEGDRAW *pDraw)
{
  int iCount = pDraw->iWidth * pDraw->iHeight;
//  Serial.printf("Draw pos = %d,%d. size = %d x %d\n", pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight);
  spilcdSetPosition(pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight, 1);
//  spilcdWriteDataBlock((uint8_t *)pDraw->pPixels, iCount*2, 1);
  spilcdWaitDMA();
  memcpy(ucTXBuf, pDraw->pPixels, iCount*sizeof(uint16_t));
  spilcdWriteDataDMA(iCount*2);  
} /* drawMCU() */

void setup()
{
char szTemp[128];

  Serial.begin(115200);
//  spilcdSetTXBuffer(ucTXBuf, sizeof(ucTXBuf));
  ucTXBuf = spilcdGetDMABuffer();
  spilcdInit(LCD_ILI9341, 0, 0, 0, 40000000, CS_PIN, DC_PIN, RESET_PIN, LED_PIN, MISO_PIN, MOSI_PIN, SCK_PIN);

//  if (!SD.begin(SDCARD_CS))
//  {
//    Serial.println("SD Init Fail!");
//  }
//  else
//  {
//    sprintf(szTemp, "SD Card Type: %d Size: %d MB", SD.cardType(), (int)(SD.cardSize() / 1024 / 1024));
//    Serial.println(szTemp);
//  }
} /* setup() */
void loop() {
long lTime;

  spilcdFill(0,1);

  if (jpeg.open((uint8_t *)thumb_test, sizeof(thumb_test), drawMCU))
  {
    Serial.println("Successfully opened JPEG image");
    Serial.printf("Image size: %d x %d, orientation: %d, bpp: %d\n", jpeg.getWidth(),
      jpeg.getHeight(), jpeg.getOrientation(), jpeg.getBpp());
    if (jpeg.hasThumb())
       Serial.printf("Thumbnail present: %d x %d\n", jpeg.getThumbWidth(), jpeg.getThumbHeight());
    jpeg.setPixelType(BIG_ENDIAN_PIXELS);
    lTime = micros();
    if (jpeg.decode(40,100,JPEG_SCALE_QUARTER | JPEG_EXIF_THUMBNAIL))
    {
      lTime = micros() - lTime;
      Serial.printf("Successfully decoded image in %d us\n", (int)lTime);
    }
    jpeg.close();
  }
  
  delay(10000);
}
