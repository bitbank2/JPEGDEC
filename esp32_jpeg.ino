#include <SD.h>
#include "pil_io.h"
#include "pil.h"
#include <bb_spi_lcd.h>

#define SDCARD_CS 0
#define PWDN_GPIO_NUM -1
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 4
#define SIOD_GPIO_NUM 18
#define SIOC_GPIO_NUM 23

File file;
// pixel drawing callback
void drawMCU(int x, int y, int cx, int cy, uint16_t *pPixels)
{
  
} /* drawMCU() */

void setup()
{
char szTemp[128];

  Serial.begin(115200);
  if (!SD.begin(SDCARD_CS))
  {
    Serial.println("SD Init Fail!");
  }
  else
  {
    sprintf(szTemp, "SD Card Type: %d Size: %d MB", SD.cardType(), (int)(SD.cardSize() / 1024 / 1024));
    Serial.println(szTemp);
  }
} /* setup() */
void loop() {
int rc;
PIL_PAGE inpage;

  rc = PILReadJPEG("/DCIM/100ESPDC/DSC00001.JPG", &inpage, drawMCU, PIL_CONVERT_16BPP);
  Serial.println(rc, DEC);
}
