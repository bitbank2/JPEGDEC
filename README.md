JPEGDEC<br>
-----------------------------------
Copyright (c) 2020 BitBank Software, Inc.<br>
Written by Larry Bank<br>
bitbank@pobox.com<br>
<br>
![JPEGDEC](/demo.jpg?raw=true "JPEGDEC")
<br>
I started working with image and video files around 1989 and soon turned my interest into a Document Imaging product based on my own imaging library. Over the years I added support for more and more formats until I had supported all of the standard ones, including DICOM. I sold my Document Imaging business in 1997, butstill found uses for my imaging code in other projects such as retro gaming. I recently went looking to see if there was a good JPEG viewer for Arduino and only found ones which sacrificed speed to work on MCUs with almost no RAM. I thought it would be possible to create an optimized JPEG decoder that could run on any MCU with at least 32K of RAM, so I started modifying my old JPEG code for this purpose. There are a lot of clever optimizations contained in the code that I kept as trade secrets for many years. I decided that it's time to share the code with the community.

<br>

Features:<br>
---------<br>
- Supports any MCU with at least 32K of RAM (Cortex-M0+ is the simplest I've tested)
- Optimized for speed; the main limitation will be how fast you can copy the pixels to the display. You can use DMA assisted SPI to help.
- GIF image data can come from memory (FLASH/RAM), SDCard or any media you provide.
- Simple class and callback design allows you to easily add GIF support to any application.
- The C code doing the heavy lifting is completely portable and has no external dependencies.
<br>
Instructions for use:<br>
---------------------<br>
Start by initializing the library. Either using hardware I2C, bit-banged I2C or SPI to talk to the display. For I2C, the
address of the display will be detected automatically (either 0x3c or 0x3d) or you can specify it. The typical MCU only allows setting the I2C speed up to 400Khz, but the SSD1306 displays can handle a much faster signal. With the bit-bang code, you can usually specify a stable 800Khz clock and with Cortex-M0 targets, the hardware I2C can be told to be almost any speed, but the displays I've tested tend to stop working beyond 1.6Mhz.<br>
<br>
After initializing the display you can begin drawing text or graphics on it. The final parameter of all of the drawing functions is a render flag. When true, the graphics will be sent to the internal backing buffer (when available) and sent to the display. You optionally pass the library a backing buffer (if your MCU has enough RAM) with the obdSetBackBuffer() function. When the render flag is false, the graphics will only be drawn into the internal buffer. Once you're ready to send the pixels to the display, call obdDumpBuffer(NULL) and it will copy the internal buffer in its entirety to the display.<br>
<br>
The text drawing function now has a scroll offset parameter. This tells it how many pixels of the text to skip before drawing the text at the given destination coordinates. For example, if you pass a value of 20 for the scroll offset and are using an 8-pixel wide font (FONT_NORMAL), the first two and a half characters will not be drawn; the second half of the third and subsequent characters will be drawn starting at the x/y you specified. This allows you to create a scrolling text effect by repeatedly calling the oledWriteString() function with progressively larger scroll offset values to make the text scroll from right to left.<br> 
<br>

If you find this code useful, please consider sending a donation.

[![paypal](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=SR4F44J2UR8S4)

