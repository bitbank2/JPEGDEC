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

Feb 22, 2021 - LeRoy Miller, KD8BXP  
