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

Instructions for use:<br>
---------------------<br>

If you find this code useful, please consider becoming a sponsor or sending a donation.

[![paypal](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=SR4F44J2UR8S4)

