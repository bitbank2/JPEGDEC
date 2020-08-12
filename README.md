JPEGDEC<br>
-----------------------------------
Copyright (c) 2020 BitBank Software, Inc.<br>
Written by Larry Bank<br>
bitbank@pobox.com<br>
<br>
![JPEGDEC](/demo.jpg?raw=true "JPEGDEC")
<br>
I started working with image and video files around 1989 and soon turned my interest into a Document Imaging product based on my own imaging library. Over the years I added support for more and more formats until I had supported all of the standard ones, including DICOM. I sold my Document Imaging business in 1997. I started over with a new library design in 2000 and added even more formats. Over the years, I found uses for my imaging code in projects such as retro gaming. I recently went looking to see if there was a good JPEG viewer for Arduino and only found ones which sacrificed speed to work on MCUs with almost no RAM. I thought it would be possible to create an optimized JPEG decoder that could run on any MCU with at least 20K of RAM, so I started modifying my old JPEG code for this purpose. There are a lot of clever optimizations contained in the code that I kept as trade secrets for many years. I decided that it's time to share the code with the community. This code doesn't look like other JPEG libraries because I wrote it from a "blank sheet". It took many hundreds of hours of work to get it working correctly and find all of the optimizations. If you appreciate my work, please consider sponsoring me.

<br>

Features:<br>
---------<br>
- Supports any MCU with at least 20K of RAM (Cortex-M0+ is the simplest I've tested)
- Optimized for speed; the main limitation will be how fast you can copy the pixels to the display. You can use DMA assisted SPI to help.
- JPEG image data can come from memory (FLASH/RAM), SDCard or any media you provide.
- Simple class and callback design allows you to easily add JPEG support to any application.
- The C code doing the heavy lifting is completely portable and has no external dependencies.
- Includes fast downscaling options (1/2, 1/4, 1/8).
- Includes option to detect and decode the embedded Exif thumbnail
- Supports Baseline Huffman images (grayscale or YCbCr)
<br>

Acquiring JPEG files to play:
----------------------------
You'll notice that the images provided in the test_images folder have been turned into C code. Each byte is now in the form 0xAB so that it can be compiled into your program and stored in FLASH memory alongside your other code. You can use a command line tool called xxd to convert a binary file into this type of text. Make sure to add a const/PROGMEM modifier in front of the JPEG data array to ensure that it gets written to FLASH and not RAM by your build environment.

The Callback functions:
-----------------------
One of the ways to allow this code to run on any embedded platform was to define a set of callback functions. These isolate the JPEG decoding logic from the display and file I/O with callback functions. This allows the core code to run on any system, but you need to help it a little. At a minimum, your code must provide a function to draw (or store) each block of image pixels emitted by the library. If you're displaying a JPEG file from memory (RAM or FLASH), this is the only function you need to provide. In the examples folder there are multiple sketches to show how this is done on various display libraries. For reading from SD cards, 4 other functions must be provided: open, close, read, seek. There is an example for implementing these in the examples folder as well.
Note:
If you're using the ESP32 or ESP8266 (or another MCU which uses the Harvard Architecture) and decoding JPEG images stored in RAM or FLASH, you'll need to use the correct open function (openRAM or openFLASH). For MCUs based on the ARM Cortex-M, they are interchangeable.

The API:
--------
Please consult the Wiki for detailed info about each method exposed by the JPEGDEC class.


If you find this code useful, please consider becoming a sponsor or sending a donation.

[![paypal](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=SR4F44J2UR8S4)

