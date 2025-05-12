//
// Display a JPEG image on a SPI LCD
// written by Larry Bank 5/12/2025
//
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "../src/JPEGDEC.h"
#include "../src/jpeg.inl"
#include <bb_spi_lcd.h> // SPI LCD library
SPILCD lcd;
JPEGIMAGE jpg;

// Pin definitions for Adafruit PiTFT HAT
// GPIO 25 = Pin 22
#define DC_PIN 49
// GPIO 27 = Pin 13
#define RESET_PIN 72
// GPIO 8 = Pin 24
#define CS_PIN 76
// GPIO 24 = Pin 18
#define LED_PIN 92
// SPI device is sent as SCK pin
#define SCK_PIN 3
// SPI device CS line is sent as MOSI pin (spidev3.0 in this case)
#define MOSI_PIN 0
#define LCD_TYPE LCD_ILI9341

int JPEGDraw(JPEGDRAW *pDraw)
{
     if (pDraw->y + pDraw->iHeight > lcd.iHeight) return 0; // beyond bottom
     spilcdSetPosition(&lcd, pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight, DRAW_TO_LCD);
     spilcdWriteDataBlock(&lcd, (uint8_t *)pDraw->pPixels, pDraw->iWidth * pDraw->iHeight * 2, DRAW_TO_LCD);
     return 1;
} /* JPEGDraw() */

int main(int argc, const char *argv[])
{
int i;

    if (argc != 2) { // show help
       printf("showimg - display a JPEG image on a SPI LCD\n");
       printf("usage: showimg <input_file>\n");
       return 0;
    }
	// Initialize the display
	// int spilcdInit(int iLCDType, int bFlipRGB, int bInvert, int bFlipped, int32_t iSPIFreq, int iCSPin, int iDCPin, int iResetPin, int iLEDPin, int iMISOPin, int iMOSIPin, int iCLKPin);
        i = spilcdInit(&lcd, LCD_TYPE, FLAGS_NONE, 60000000, CS_PIN, DC_PIN, RESET_PIN, LED_PIN, -1,MOSI_PIN,SCK_PIN,0);
	spilcdSetOrientation(&lcd, LCD_ORIENTATION_90);
        spilcdFill(&lcd, TFT_BLACK, DRAW_TO_LCD);
	spilcdWriteString(&lcd, 0, 0, "Linux JPEG Decoder Example", TFT_GREEN, 0,  FONT_12x16, DRAW_TO_LCD);
	if (JPEG_openFile(&jpg, argv[1], JPEGDraw)) {
                jpg.ucPixelType = RGB565_BIG_ENDIAN;
    		JPEG_decode(&jpg, 0, 16, 0); // leave the first line of text showing
	}
	return 0;
} /* main() */

