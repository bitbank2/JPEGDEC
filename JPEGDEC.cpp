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
// Expand the Huffman tables for fast decoding
//
int JPEGMakeHuffTables(JPEGIMAGE *pJPEG, int bThumbnail)
{
    int code, repeat, count, codestart;
    int j;
    int iLen, iTable;
    unsigned short *pTable, *pShort, *pLong;
    unsigned char *pucTable, *pucShort, *pucLong;
    uint32_t ul, *pLongTable;
    int iBitNum; // current code bit length
    int cc; // code
    unsigned char *p, *pBits, ucCode;
    int iMaxLength, iMaxMask;
    int iTablesUsed;
    
    iTablesUsed = 0;
    for (j=0; j<4; j++)
    {
        if (pJPEG->ucHuffTableUsed & (1 << j) != 0)
            iTablesUsed++;
    }
    // first do DC components (up to 4 tables of 12-bit codes)
    // we can save time and memory for the DC codes by knowing that there exist short codes (<= 6 bits)
    // and long codes (>6 bits, but the first 5 bits are 1's).  This allows us to create 2 tables: a 6-bit and 7 or 8-bit
    // to handle any DC codes
    iMaxLength = 12; // assume DC codes can be 12-bits
    iMaxMask = 0x7f; // lower 7 bits after truncate 5 leading 1's
    for (iTable = 0; iTable < 4; iTable++)
    {
        if (pJPEG->ucHuffTableUsed && (1 << iTable))
        {
            //         pJPEG->huffdcFast[iTable] = (int *)PILIOAlloc(0x180); // short table = 128 bytes, long table = 256 bytes
            pucShort = (unsigned char *)&pJPEG->ucHuffDC[iTable*2048];
            //         pJPEG->huffdc[iTable] = pJPEG->huffdcFast[iTable] + 0x20; // 0x20 longs = 128 bytes
            pucLong = (unsigned char *)&pJPEG->ucHuffDC[iTable*2048 + 1024];
            pBits = &pJPEG->ucHuffVals[iTable * HUFF_TABLEN];
            p = pBits;
            p += 16; // point to bit data
            cc = 0; // start with a code of 0
            for (iBitNum = 1; iBitNum <= 16; iBitNum++)
            {
                iLen = *pBits++; // get number of codes for this bit length
                if (iBitNum > iMaxLength && iLen > 0) // we can't handle codes longer a certain length
                {
                    return -1;
                }
                while (iLen)
                {
                    //               if (iBitNum > 6) // do long table
                    if ((cc >> (iBitNum-5)) == 0x1f) // first 5 bits are 1 - use long table
                    {
                        count = iMaxLength - iBitNum;
                        codestart = cc << count;
                        pucTable = &pucLong[codestart & iMaxMask]; // use lower 7/8 bits of code
                    }
                    else // do short table
                    {
                        count = 6 - iBitNum;
                        if (count < 0)
                            return -1; // DEBUG - something went wrong
                        codestart = cc << count;
                        pucTable = &pucShort[codestart];
                    }
                    ucCode = *p++;  // get actual huffman code
                    // does precalculating the DC value save time on ARM?
#ifndef USE_ARM_ASM
                    if (ucCode != 0 && (ucCode + iBitNum) <= 6 && pJPEG->ucMode != 0xc2) // we can fit the magnitude value in the code lookup (not for progressive)
                    {
                        int k, iLoop;
                        unsigned char ucCoeff;
                        unsigned char *d = &pucTable[512];
                        unsigned char ucMag = ucCode;
                        ucCode |= ((iBitNum+ucCode) << 4); // add magnitude bits to length
                        repeat = 1<<ucMag;
                        iLoop = 1<<(count-ucMag);
                        for (j=0; j<repeat; j++)
                        { // calcuate the magnitude coeff already
                            if (j & 1<<(ucMag-1)) // positive number
                                ucCoeff = (unsigned char)j;
                            else // negative number
                                ucCoeff = (unsigned char)(j - ((1<<ucMag)-1));
                            for (k=0; k<iLoop; k++)
                            {
                                *d++ = ucCoeff;
                            } // for k
                        } // for j
                    }
#endif
                    else
                    {
                        ucCode |= (iBitNum << 4);
                    }
                    if (count) // do it as dwords to save time
                    {
                        repeat = (1<<count);
                        memset(pucTable, ucCode, repeat);
                        //                  pLongTable = (uint32_t *)pTable;
                        //                  repeat = 1 << (count-2); // store as dwords (/4)
                        //                  ul = code | (code << 16);
                        //                  for (j=0; j<repeat; j++)
                        //                     *pLongTable++ = ul;
                    }
                    else
                    {
                        pucTable[0] = ucCode;
                    }
                    cc++;
                    iLen--;
                }
                cc <<= 1;
            }
        } // if table defined
    }
    // now do AC components (up to 4 tables of 16-bit codes)
    // We split the codes into a short table (9 bits or less) and a long table (first 5 bits are 1)
    for (iTable = 0; iTable < 4; iTable++)
    {
        if (pJPEG->ucHuffTableUsed & (1 << (iTable+4)))  // if this table is defined
        {
            pBits = &pJPEG->ucHuffVals[(iTable+4) * HUFF_TABLEN];
            p = pBits;
            p += 16; // point to bit data
            //         pJPEG->huffacFast[iTable] = (int *)PILIOAlloc(0x1400); // fast table = 1024 bytes, slow = 4096
            //         pJPEG->huffac[iTable] = pJPEG->huffacFast[iTable] + 0x100; // 0x100 longs = 1024 bytes
            pShort = (unsigned short *)&pJPEG->ucHuffAC[iTable*2048];
            pLong = (unsigned short *)&pJPEG->ucHuffAC[iTable*2048 + 1024];
            cc = 0; // start with a code of 0
            // construct the decode table
            for (iBitNum = 1; iBitNum <= 16; iBitNum++)
            {
                iLen = *pBits++; // get number of codes for this bit length
                while (iLen)
                {
                    if ((cc >> (iBitNum-6)) == 0x3f) // first 6 bits are 1 - use long table
                    {
                        count = 16 - iBitNum;
                        codestart = cc << count;
                        pTable = &pLong[codestart & 0x3ff]; // use lower 10 bits of code
                    }
                    else
                    {
                        count = 10 - iBitNum;
                        if (count < 0) // an 11/12-bit? code - that doesn't fit our optimized scheme, see if we can do a bigger table version
                        {
                            if (count == -1 && iTablesUsed <= 4) // we need to create "slow" tables
                            {
//                                j = JPEGMakeHuffTables_Slow(pJPEG, bThumbnail);
//                                pJPEG->huffacFast[1] = (int *) &pJPEG->ucAltHuff[0x0000];
//                                pJPEG->huffacFast[2] = (int *) &pJPEG->ucAltHuff[0x4000];
//                                pJPEG->huffacFast[3] = (int *) &pJPEG->ucAltHuff[0x8000];
                                return j;
                            }
                            else
                                return -1; // DEBUG - fatal error, more than 2 big tables we currently don't support
                        }
                        codestart = cc << count;
                        pTable = &pShort[codestart]; // 10 bits or shorter
                    }
                    code = *p++;  // get actual huffman code
                    if (bThumbnail && code != 0) // add "extra" bits to code length since we skip these codes
                    {
                        // get rid of extra bits in code and add increment (1) for AC index
                        code = ((iBitNum+(code & 0xf)) << 8) | ((code >> 4)+1);
                    }
#ifdef BOGUS // precalculating the AC coeff makes it run slightly slower
                    else if ((code & 0xf) != 0 && (code + iBitNum) <= 10) // we can fit the magnitude value + huffman code in a single read
                    {
                        int k, iLoop;
                        unsigned short usCoeff;
                        unsigned short *d = &pTable[4096]; // use unused table slots 2+3 for extra coeff data
                        unsigned char ucMag = (unsigned char)(code & 0xf);
                        code |= ((iBitNum + (code & 0xf)) << 8); // add magnitude bits to length
                        repeat = 1<<ucMag;
                        iLoop = 1<<(count-ucMag);
                        for (j=0; j<repeat; j++)
                        { // calcuate the magnitude coeff already
                            if (j & 1<<(ucMag-1)) // positive number
                                usCoeff = (unsigned short)j;
                            else // negative number
                                usCoeff = (unsigned short)(j - ((1<<ucMag)-1));
                            for (k=0; k<iLoop; k++)
                            {
                                *d++ = usCoeff;
                            } // for k
                        } // for j
                    }
#endif
                    else
                    {
                        code |= (iBitNum << 8);
                    }
                    if (count) // do it as dwords to save time
                    {
                        repeat = 1 << (count-1); // store as dwords (/2)
                        ul = code | (code << 16);
                        pLongTable = (uint32_t *)pTable;
                        for (j=0; j<repeat; j++)
                            *pLongTable++ = ul;
                    }
                    else
                    {
                        pTable[0] = (unsigned short)code;
                    }
                    cc++;
                    iLen--;
                }
                cc <<= 1;
            } // for each bit length
        } // if table defined
    }
    return 0;
} /* JPEGMakeHuffTables() */

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
                pPage->ucMode = (uint8_t)usMarker;
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
    {
        JPEGMakeHuffTables(pPage, 0); //int bThumbnail)
        return 1;
    }
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
