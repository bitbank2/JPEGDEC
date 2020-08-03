//
// JPEG Decoder
// written by Larry Bank
// bitbank@pobox.com
// Arduino port started 8/2/2020
// Original JPEG code written 26+ years ago :)
// The goal of this code is to decode baseline JPEG images
// using no more than 22K of RAM (if sent directly to an LCD display)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#include "JPEGDEC.h"

// forward references
static int JPEGInit(JPEGIMAGE *pJPEG);
static int JPEGParseInfo(JPEGIMAGE *pPage);
static void JPEGGetMoreData(JPEGIMAGE *pPage);
static int DecodeJPEG(JPEGIMAGE *pImage, int iOptions);

//
// Helper functions for memory based images
//
static int32_t readMem(JPEGFILE *pFile, uint8_t *pBuf, int32_t iLen)
{
    int32_t iBytesRead;

    iBytesRead = iLen;
    if ((pFile->iSize - pFile->iPos) < iLen)
       iBytesRead = pFile->iSize - pFile->iPos;
    if (iBytesRead <= 0)
       return 0;
    memcpy_P(pBuf, &pFile->pData[pFile->iPos], iBytesRead);
    pFile->iPos += iBytesRead;
    return iBytesRead;
} /* readMem() */

static int32_t seekMem(JPEGFILE *pFile, int32_t iPosition)
{
    if (iPosition < 0) iPosition = 0;
    else if (iPosition >= pFile->iSize) iPosition = pFile->iSize-1;
    pFile->iPos = iPosition;
    return iPosition;
} /* seekMem() */

//
// Memory initialization
//
int JPEGDEC::open(uint8_t *pData, int iDataSize, JPEG_DRAW_CALLBACK *pfnDraw)
{
    _jpeg.pfnRead = readMem;
    _jpeg.pfnSeek = seekMem;
    _jpeg.pfnDraw = pfnDraw;
    _jpeg.pfnOpen = NULL;
    _jpeg.pfnClose = NULL;
    _jpeg.JPEGFile.iSize = iDataSize;
    _jpeg.JPEGFile.pData = pData;
    return JPEGInit(&_jpeg);
} /* open() */

int JPEGDEC::getWidth()
{
    return _jpeg.iWidth;
} /* getWidth() */

int JPEGDEC::getHeight()
{
    return _jpeg.iHeight;
} /* getHeight() */

int JPEGDEC::getBpp()
{
    return (int)_jpeg.ucBpp;
} /* getBpp() */

int JPEGDEC::getSubSample()
{
    return (int)_jpeg.ucSubSample;
} /* getSubSample() */

//
// File (SD/MMC) based initialization
//
int JPEGDEC::open(char *szFilename, JPEG_OPEN_CALLBACK *pfnOpen, JPEG_CLOSE_CALLBACK *pfnClose, JPEG_READ_CALLBACK *pfnRead, JPEG_SEEK_CALLBACK *pfnSeek, JPEG_DRAW_CALLBACK *pfnDraw)
{
    _jpeg.pfnRead = pfnRead;
    _jpeg.pfnSeek = pfnSeek;
    _jpeg.pfnDraw = pfnDraw;
    _jpeg.pfnOpen = pfnOpen;
    _jpeg.pfnClose = pfnClose;
    _jpeg.JPEGFile.fHandle = (*pfnOpen)(szFilename, &_jpeg.JPEGFile.iSize);
    if (_jpeg.JPEGFile.fHandle == NULL)
       return 0;
    return JPEGInit(&_jpeg);

} /* open() */

void JPEGDEC::close()
{
    if (_jpeg.pfnClose)
        (*_jpeg.pfnClose)(_jpeg.JPEGFile.fHandle);
} /* close() */

void JPEGDEC::begin(int iEndian)
{
    memset(&_jpeg, 0, sizeof(_jpeg));
    _jpeg.ucLittleEndian = (iEndian == LITTLE_ENDIAN_PIXELS);
}
//
// Play a single frame
// returns:
// 1 = good result and more frames exist
// 0 = good result and no more frames exist
// -1 = error
int JPEGDEC::decode()
{
    return DecodeJPEG(&_jpeg, 0);
} /* decode() */
//
// The following functions are written in plain C and have no
// 3rd party dependencies, not even the C runtime library
//
//
// Initialize a GIF file and callback access from a file on SD or memory
// returns 1 for success, 0 for failure
// Fills in the canvas size of the GIFIMAGE structure
//
static int JPEGInit(JPEGIMAGE *pJPEG)
{
    pJPEG->JPEGFile.iPos = 0; // start at beginning of file
    return JPEGParseInfo(pJPEG); // gather info for image
} /* GIFInit() */
//
// Unpack the Huffman tables
//
static int JPEGGetHuffTables(uint8_t *pBuf, int iLen, JPEGIMAGE *pJPEG)
{
    int i, j, iOffset, iTableOffset;
    unsigned char ucTable;
    
    iOffset = 0;
    while (iLen > 17)  // while there are tables to copy (we may have combined more than 1 table together)
    {
        ucTable = pBuf[iOffset++]; // get table index
        if (ucTable & 0x10) // convert AC offset of 0x10 into offset of 4
            ucTable ^= 0x14;
        pJPEG->ucHuffTableUsed |= (1 << ucTable); // mark this table as being defined
        if (ucTable <= 7) // tables are 0-3, AC+DC
        {
            iTableOffset = ucTable * HUFF_TABLEN;
            j = 0; // total bits
            for (i=0; i<16; i++)
            {
                j += pBuf[iOffset];
                pJPEG->ucHuffVals[iTableOffset+i] = pBuf[iOffset++];
            }
            iLen -= 17; // subtract length of bit lengths
            if (j == 0 || j > 256 || j > iLen) // bogus bit lengths
            {
                return -1;
            }
            iTableOffset += 16;
            for (i=0; i<j; i++)
            {  // copy huffman table
                pJPEG->ucHuffVals[iTableOffset+i] = pBuf[iOffset++];
            }
            iLen -= j;
        }
    }
    return 0;
} /* JPEGGetHuffTables() */

//
// Parse the JPEG header, gather necessary info to decode the image
// Returns 1 for success, 0 for failure
//
static int JPEGParseInfo(JPEGIMAGE *pPage)
{
    int iBytesRead, iNumComponents;
    int i, iOffset, iTableOffset;
    uint8_t ucTable, *s = pPage->ucFileBuf;
    uint16_t usMarker, usLen;
    int iFilePos = 0;
    
    iBytesRead = (*pPage->pfnRead)(&pPage->JPEGFile, s, FILE_BUF_SIZE);
    if (iBytesRead < FILE_BUF_SIZE) // a JPEG file this tiny? probably bad
        return 0;
    iFilePos += iBytesRead;
    if (MOTOSHORT(pPage->ucFileBuf) != 0xffd8)
        return 0; // not a JPEG file
    iOffset = 2; /* Start at offset of first marker */
    usMarker = 0; /* Search for SOFx (start of frame) marker */
    while (usMarker != 0xffda && iOffset < pPage->JPEGFile.iSize)
    {
        if (iOffset >= FILE_BUF_SIZE/2) // too close to the end, read more data
        {
            // Do we need to seek first?
            if (iOffset >= FILE_BUF_SIZE)
            {
                iFilePos += iOffset;
                iOffset = 0;
                (*pPage->pfnSeek)(&pPage->JPEGFile, iFilePos);
                iBytesRead = 0; // throw away any old data
            }
            // move existing bytes down
            if (iOffset)
            {
                memcpy(pPage->ucFileBuf, &pPage->ucFileBuf[iOffset], iBytesRead - iOffset);
                iBytesRead -= iOffset;
                iOffset = 0;
            }
            i = (*pPage->pfnRead)(&pPage->JPEGFile, &pPage->ucFileBuf[iBytesRead], FILE_BUF_SIZE-iBytesRead);
            iFilePos += i;
            iBytesRead += i;
        }
        usMarker = MOTOSHORT(&s[iOffset]);
        iOffset += 2;
        usLen = MOTOSHORT(&s[iOffset]); // marker length
        if (usMarker < 0xffc0 || usMarker == 0xffff) // invalid marker, could be generated by "Arles Image Web Page Creator" or Accusoft
        {
            iOffset++;
            continue; // skip 1 byte and try to resync
        }
        switch (usMarker)
        {
            case 0xffc1:
            case 0xffc2:
            case 0xffc3:
                return 0; // currently unsupported modes
                
            case 0xffc0: // SOFx - start of frame
                pPage->ucBpp = s[iOffset+2]; // bits per sample
                pPage->iHeight = MOTOSHORT(&s[iOffset+3]);
                pPage->iWidth = MOTOSHORT(&s[iOffset+5]);
                iNumComponents = s[iOffset+7];
                pPage->ucBpp = pPage->ucBpp * iNumComponents; /* Bpp = number of components * bits per sample */
                pPage->ucSubSample = s[iOffset+9]; // subsampling option for the second color component
                pPage->ucSubSample = (pPage->ucSubSample & 0xf) | (pPage->ucSubSample >> 2);
                break;
            case 0xffdd: // Restart Interval
                if (usLen == 4)
                    pPage->iResInterval = MOTOSHORT(&s[iOffset+2]);
                break;
            case 0xffc4: /* M_DHT */ // get Huffman tables
                iOffset += 2; // skip length
                usLen -= 2; // subtract length length
                if (JPEGGetHuffTables(&s[iOffset], usLen, pPage) != 0) // bad tables?
                    return 0; // error
                break;
            case 0xffdb: /* M_DQT */
                /* Get the quantization tables */
                /* first byte has PPPPNNNN where P = precision and N = table number 0-3 */
                iOffset += 2; // skip length
                usLen -= 2; // subtract length length
                while (usLen > 0)
                {
                    ucTable = s[iOffset++]; // table number
                    if ((ucTable & 0xf) > 3) // invalid table number
                        return 0;
                    iTableOffset = (ucTable & 0xf) * DCTSIZE;
                    if (ucTable & 0xf0) // if word precision
                    {
                        for (i=0; i<DCTSIZE; i++)
                        {
                            pPage->sQuantTable[i+iTableOffset] = MOTOSHORT(&s[iOffset]);
                            iOffset += 2;
                        }
                        usLen -= (DCTSIZE*2 + 1);
                    }
                    else // byte precision
                    {
                        for (i=0; i<DCTSIZE; i++)
                        {
                            pPage->sQuantTable[i+iTableOffset] = (unsigned short)s[iOffset++];
                        }
                        usLen -= (DCTSIZE + 1);
                    }
                }
                break;

        }
        iOffset += usLen;
    } // while
    if (usMarker == 0xffda) // start of image
        return 1;
    return 0;
} /* JPEGParseInfo() */
//
// Filter more VLC data for decoding
// returns 1 to signify more data available for this image
// 0 indicates there is no more data
//
static void JPEGGetMoreData(JPEGIMAGE *pPage)
{
    unsigned char c = 1;
    // move any existing data down
    if ((pPage->iVLCSize - pPage->iVLCOff) >= VLC_HIGHWATER)
        return; // buffer is already full; no need to read more data
    if (pPage->iVLCOff != 0)
    {
      memcpy(pPage->ucVLC, &pPage->ucVLC[pPage->iVLCOff], pPage->iVLCSize - pPage->iVLCOff);
      pPage->iVLCSize -= pPage->iVLCOff;
      pPage->iVLCOff = 0;
    }
    while (c && pPage->JPEGFile.iPos < pPage->JPEGFile.iSize && pPage->iVLCSize < VLC_HIGHWATER)
    {
        (*pPage->pfnRead)(&pPage->JPEGFile, &c, 1); // current length
        (*pPage->pfnRead)(&pPage->JPEGFile, &pPage->ucVLC[pPage->iVLCSize], c);
        pPage->iVLCSize += c;
    }
} /* JPEGGetMoreData() */

//
// Decode the image
//
static int DecodeJPEG(JPEGIMAGE *pImage, int iOptions)
{
    return 0;
} /* DecodeJPEG() */
