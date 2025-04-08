#include "JPEGDisplay.h"
#include <SD.h>

static int JPEGDraw(JPEGDRAW *pDraw)
{
BB_SPI_LCD *pLCD = (BB_SPI_LCD *)pDraw->pUser;

    pLCD->setAddrWindow(pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight);
    pLCD->pushPixels((uint16_t *)pDraw->pPixels, pDraw->iWidth * pDraw->iHeight);
    return 1;
} /* JPEGDraw() */

// Functions to access a file on the SD card
static File myfile;

static void * myOpen(const char *filename, int32_t *size) {
  myfile = SD.open(filename);
  *size = myfile.size();
  return &myfile;
}
static void myClose(void *handle) {
  if (myfile) myfile.close();
}
static int32_t myRead(JPEGFILE *handle, uint8_t *buffer, int32_t length) {
  if (!myfile) return 0;
  return myfile.read(buffer, length);
}
static int32_t mySeek(JPEGFILE *handle, int32_t position) {
  if (!myfile) return 0;
  return myfile.seek(position);
}

int JPEGDisplay::loadJPEG(BB_SPI_LCD *pLCD, int x, int y, const void *pData, int iDataSize)
{
JPEGDEC *jpeg;
int w, h, rc;

    jpeg = (JPEGDEC *)malloc(sizeof(JPEGDEC));
    if (!jpeg) return 0;
    if (jpeg->openRAM((uint8_t *)pData, iDataSize, JPEGDraw)) {
        jpeg->setPixelType(RGB565_BIG_ENDIAN);
        w = jpeg->getWidth();
        h = jpeg->getHeight();
        if (x == JPEGDISPLAY_CENTER) {
            x = (pLCD->width() - w)/2;
            if (x < 0) x = 0;
        } else if (x < 0 || w + x > pLCD->width()) {
            return 0; // clipping not supported
        }
        if (y == JPEGDISPLAY_CENTER) {
            y = (pLCD->height() - h)/2;
            if (y < 0) y = 0;
        } else if (y < 0 || y + h > pLCD->height()) {
        // clipping is not supported
            return 0;
        }
        jpeg->setUserPointer((void *)pLCD);
        jpeg->decode(x, y, 0); // simple decode, no options
        jpeg->close();
        free(jpeg);
        return 1;
   }
   free(jpeg);
   return 0;
} /* loadJPEG() */

int JPEGDisplay::loadJPEG(BB_SPI_LCD *pLCD, int x, int y, const char *fname)
{
    JPEGDEC *jpeg;
    int w, h, rc;

    jpeg = (JPEGDEC *)malloc(sizeof(JPEGDEC));
    if (!jpeg) return 0;
    if (jpeg->open(fname, myOpen, myClose, myRead, mySeek, JPEGDraw)) {
        jpeg->setPixelType(RGB565_BIG_ENDIAN);
        w = jpeg->getWidth();
        h = jpeg->getHeight();
        if (x == JPEGDISPLAY_CENTER) {
            x = (pLCD->width() - w)/2;
            if (x < 0) x = 0;
        } else if (x < 0 || w + x > pLCD->width()) {
            return 0; // clipping not supported
        }
        if (y == JPEGDISPLAY_CENTER) {
            y = (pLCD->height() - h)/2;
            if (y < 0) y = 0;
        } else if (y < 0 || y + h > pLCD->height()) {
        // clipping is not supported
            return 0;
        }
        jpeg->setUserPointer((void *)pLCD);
        jpeg->decode(x, y, 0); // simple decode, no options
        jpeg->close();
        free(jpeg);
        return 1;
    }
    free(jpeg);
    return 0;
} /* loadJPEG() */

int JPEGDisplay::getJPEGInfo(int *width, int *height, int *bpp, const void *pData, int iDataSize)
{
    JPEGDEC *jpeg;
    int rc;

    if (!width || !height || !bpp || !pData || iDataSize < 32) return 0;
    
    jpeg = (JPEGDEC *)malloc(sizeof(JPEGDEC));
    if (!jpeg) return 0;
    if (jpeg->openRAM((uint8_t *)pData, iDataSize, JPEGDraw)) {
        *width = jpeg->getWidth();
        *height = jpeg->getHeight();
        *bpp = jpeg->getBpp();
        free(jpeg);
        return 1;
    }
    free(jpeg);
    return 0;
} /* getJPEGInfo() */

int JPEGDisplay::getJPEGInfo(int *width, int *height, int *bpp, const char *fname)
{
    JPEGDEC *jpeg;
    int rc;

    if (!width || !height || !bpp || !fname) return 0;
    jpeg = (JPEGDEC *)malloc(sizeof(JPEGDEC));
    if (!jpeg) return 0;
    if (jpeg->open(fname, myOpen, myClose, myRead, mySeek, JPEGDraw)) {
        *width = jpeg->getWidth();
        *height = jpeg->getHeight();
        *bpp = jpeg->getBpp();
        jpeg->close();
        free(jpeg);
        return 1;
    }
    free(jpeg);
    return 0;
} /* getJPEGInfo() */
