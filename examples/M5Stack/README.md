These are slightly modified versions of the adafruit_gfx_demo.  
Extra Images were included:  
The Star Ship ENTERPRISE "NCC-1701" from http://clipart-library.com/clip-art/uss-enterprise-silhouette-11.htm  
And an image of "Batman" from http://clipart-library.com/clipart/batman-clip-art_19.htm  

The JPEGDraw function was changed for use with the M5Stack devices.  

  M5.Lcd.drawBitmap((int16_t)pDraw->x, (int16_t)pDraw->y, (int16_t)pDraw->iWidth, (int16_t)pDraw->iHeight, pDraw->pPixels);  

Another small change was to add a check if the image had a thumbnail or not, and call the correct jpg decode routine.  
 if (jpeg.hasThumb()) {jpeg.decode(iCenterX[i],iCenterY[i],JPEG_EXIF_THUMBNAIL | iOption[i]); } else { jpeg.decode(iCenterX[i],iCenterY[i], iOption[i]); }  

To display the other images, uncomment the image you want to display, comment the others.  
Also uncomment the line with the name of the image you want to display, and comment the others.  
if (jpeg.openFLASH((uint8_t *)thumb_test, sizeof(thumb_test), JPEGDraw))  
    //if (jpeg.openFLASH((uint8_t *)ncc1701, sizeof(ncc1701), JPEGDraw))  
    //if (jpeg.openFLASH((uint8_t *)batman, sizeof(batman), JPEGDraw))  

M5Stack and M5Fire sketches are almost the same, the difference is how you start the screen on the Fire.  
M5StickC and M5StickCPlus are the same with the difference being which library is used (either M5StickC.h or M5StickkCPlus.h) and display size between the two devices.  

Tested on:
M5Stack Gray  
M5Stack Fire  
M5StickC  
M5StickC Plus  

Untested:
M5Core2  
M5Stack Basic (should work)  
M5Go Lite  
M5Go  

Feb 22, 2021 - LeRoy Miller, KD8BXP  
