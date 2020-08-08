//#include <SD.h>
#include <bb_spi_lcd.h>
#include "JPEGDEC.h"
#include "test_images/gray_road.h"
//#include "test_images/f6t.h"

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
  spilcdSetPosition(pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight, 1);
  spilcdWaitDMA();
  memcpy(ucTXBuf, pDraw->pPixels, 64*sizeof(uint16_t));
  spilcdWriteDataDMA(64*2);  
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
//  if (jpeg.open((uint8_t *)f6t, sizeof(f6t), drawMCU))
  if (jpeg.open((uint8_t *)gray_road, sizeof(gray_road), drawMCU))
  {
    Serial.println("Successfully opened JPEG image");
    Serial.printf("Image size: %d x %d, orientation: %d, bpp: %d\n", jpeg.getWidth(), jpeg.getHeight(), jpeg.getOrientation(), jpeg.getBpp());
    lTime = micros();
    if (jpeg.decode(0))
    {
      lTime = micros() - lTime;
      Serial.printf("Successfully decoded image in %d us\n", (int)lTime);
    }
    jpeg.close();
  }
  delay(10000);
}
