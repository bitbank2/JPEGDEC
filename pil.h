/* PIL - portable imaging library */
/* Copyright (c) 2000 BitBank Software, Inc. */
/* Written by Larry Bank */
/* Project started 12/7/2000 */
/* A high speed imaging library designed for */
/* low memory/cpu environments such as Windows CE */

#ifndef _PIL_H_
#define _PIL_H_

// Apple is all 64-bit
#if defined(__MACH__) || defined(__APPLE__)
#define _64BITS
#define USE_SIMD
#endif

#define PIL_BUFFER_SIZE    0x20000    // file buffer size
#define PIL_BUFFER_HIGHWATER 0x1f000   // 4K shy of end is a good place to reload
#define MAX_META_COUNT 256          // a reasonable number of metadata objects

#if defined(_X64) && !defined(_NO_ASM)
//#define JPEGDecodeMCU X64DECODEMCU
//#define JPEGDecodeMCUFast X64DECODEMCUFAST
#define PILEncodeLine X64EncodeLine
//#define PILInsertCode X64InsertCode
#endif // _X64

#if defined(_X86) && !defined(_X64) && !defined(_NO_ASM)
//#define JPEGDecodeMCU X86DECODEMCU
//#define JPEGDecodeMCUFast X86DECODEMCUFAST
#define PILEncodeLine X86EncodeLine
//#define PILInsertCode X86InsertCode
#endif

//#ifdef HAVE_NEON
#ifdef __arm__
#define PILDraw1Line ARMDraw1Line
#define PILDraw1Scaled ARMDraw1Scaled
#define PILEncodeLine ARMEncodeLine
#endif // HAVE_NEON

#ifdef USE_ARM_ASM
//#define JPEGIDCT ARMJPEGIDCT
//#define JPEGFDCT ARMJPEGFDCT
//#define JPEGDecodeMCU ARMDecodeMCU
//#define JPEGDecodeMCUFast ARMDecodeMCUFast
//#define JPEGEncodeMCU ARMEncodeMCU
#define JPEGGet16Bits ARMJPEGGet16Bits
#define PILDraw1Line ARMDraw1Line
#define PILDraw1Scaled ARMDraw1Scaled
#define PILEncodeLine ARMEncodeLine
#endif // USE_ARM_ASM

#ifndef uint32_t
#ifdef _WIN32
typedef __int16 int16_t;
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int16 uint16_t;
#define PILBSWAP64 _byteswap_uint64
#define PILBSWAP32 _byteswap_ulong
#ifdef _64BITS
#define PILBSWAP _byteswap_uint64
#define PILFIRST1(mask, index) _BitScanForward64(&index, mask)
#define PILCLZ(mask, index) _BitScanReverse64(&index, mask)
#else
#define PILFIRST1(mask, index) _BitScanForward(&index, mask)
#define PILCLZ(mask, index) _BitScanReverse(&index, mask)
#define PILBSWAP _byteswap_ulong
#endif
#else // _WIN32
typedef unsigned int uint32_t;
typedef int int32_t;
typedef short int16_t;
typedef unsigned short uint16_t;
#define PILBSWAP64 __builtin_bswap64
#define PILBSWAP32 __builtin_bswap32
#ifdef _64BITS
#define PILBSWAP(mask) __builtin_bswap64(mask)
#define PILFIRST1(mask, index) index = __builtin_ctzl(mask)
#define PILCLZ(mask, index) index = __builtin_clzl(mask)
#else
#define PILBSWAP(mask) __builtin_bswap32(mask)
#define PILFIRST1(mask, index) index = __builtin_ctz(mask)
#define PILCLZ(mask, index) index = __builtin_clz(mask)
#endif // 64-bits
#endif // GCC
#if defined(_64BITS) || defined(_X64)
#ifdef _WIN32
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
//#else
//typedef signed long int64_t;
//typedef unsigned long uint64_t;
#endif // _WIN32
#endif // _64BITS
#endif // uint32_t

// Write a run length to memory
// 1 byte for runs 0-127
// 2 bytes for runs 128-32766 (0x80 + high byte, low byte)
// 5 bytes for runs >= 32767 (FFFF followed by 3-byte length)
#define WRITERUN(p, l) if (l > 32766) {p[0] = 0xff; p[1] = 0xff; p[2] = (unsigned char)(l); p[3] = (unsigned char)(l >> 8); p[4] = (unsigned char)(l>>16); p+=5;} else if (l > 127) {p[0] = 0x80 | (unsigned char)(l >> 8); p[1] = l & 0xff; p += 2;} else {p[0] = (unsigned char)l; p++;}

    // Read a run length from memory
#define READRUN(run, ptr) run = *ptr++; if (run >= 128) {if (run == 255 && *ptr == 255) {run = (ptr[1])+(ptr[2]<<8)+(ptr[3]<<16); ptr += 4;} else {run = ((run & 0x7f)<< 8) + *ptr++; }}

// Needs to be #included prior to any other PIL #includes to avoid lack of
// macro expansion problems
#include "pil_io.h"

// #defined if audio decoding capability is desired
//#define	PIL_AUDIO_INCLUDED

// Header files required for forward reference to audio structure
#ifdef PIL_AUDIO_INCLUDED
#include "pil_audio.h"
#endif // PIL_AUDIO_INCLUDED

// During initialization of the decoder, various header information needs to be gathered
// these flags are used to mark which info has been
#define PIL_JPEGHDR_HUFF 1      // huffman tables defined
#define PIL_JPEGHDR_QUANT 2     // quantization tables defined
#define PIL_JPEGHDR_SOS 4       // start of scan defined
#define PIL_JPEGHDR_SOF 8       // start of frame defined
#define PIL_JPEGHDR_RST 16      // restart interval defined
#define PIL_JPEGHDR_READY 15    // minimum info to start image decode

// Progress indicator function
// returns 2 possible values:
#define PIL_PROGRESS_CONTINUE 0  // keep decoding
#define PIL_PROGRESS_CANCEL   1  // cancel current operation
typedef int (*PILPROGRESS)(uint32_t ulCurrent, uint32_t ulTotal);

// indicates what new information to display
typedef enum {
	FIV_EVENT_NONE=0,
	FIV_EVENT_FILE,
	FIV_EVENT_PAGE,
	FIV_EVENT_ZOOM,
	FIV_EVENT_ERROR
} fiv_event_types;

/* Supported compression types */
typedef enum  {
PIL_COMP_UNKNOWN=0,
PIL_COMP_NONE,	// uncompressed (flat bitmap)
PIL_COMP_G31D,	// CCITT Group 3 1-D
PIL_COMP_G32D,	// CCITT Group 3 2-D
PIL_COMP_G4,	// CCITT Group 4
PIL_COMP_MMR,  // IBM's stupid variation of G4
PIL_COMP_PCX,	// Paintshop PCX (run length)
PIL_COMP_TIFFPACKBITS,	// TIFF 4.0 Packbits (modified run length)
PIL_COMP_TIFFHUFFMAN,	// TIFF 4.0 Huffman (modified G3 1-D)
PIL_COMP_LZW,	// Unisys LZW
PIL_COMP_GIF,  // Unisys LZW with small differences
PIL_COMP_JPEG,	// JPEG baseline (DCT)
PIL_COMP_RLE,	// BitBank proprietary run-length encoding
PIL_COMP_PCL,	// HP PCL
PIL_COMP_WINRLE, // Windows BMP RLE
PIL_COMP_FLC,    // Aegis animator format
PIL_COMP_AVMFAX, // Packetized Modified Huffman data for AVM faxes
PIL_COMP_PNG,    // PNG flate compressed & filtered data
PIL_COMP_FLATE,  // PDF zlib flate compression
PIL_COMP_IPHONE_FLATE,  // Apple's hacked PNG format
PIL_COMP_MJPEG,  // Motion JPEG
PIL_COMP_MJPEG_AB, // Motion JPEG A/B (interlaced)
PIL_COMP_CINEPAK,  // Cinepak video compression
PIL_COMP_H263,     // H263 video conferencing codec
PIL_COMP_H264,     // H264 video
PIL_COMP_MPEG,     // MPEG video
PIL_COMP_MPEG4,    // MPEG-4 video
PIL_COMP_MSVC, // Microsoft Video-1
PIL_COMP_DICOMRLE, // DICOM Run Length encoded
PIL_COMP_DICOMRAW, // raw pixels from a DICOM image
PIL_COMP_THUNDERSCAN, // Thunderscan 4-bit RLE
PIL_COMP_JPEG2K, // JPEG2000
PIL_COMP_JBIG,  // JBIG1 (corresponds to ITU T.85 recommendation for single-plane bitonal images)
PIL_COMP_ETC1,
PIL_COMP_ETC2,
PIL_COMP_DXT1,
PIL_COMP_DXT2,
PIL_COMP_DXT3,
PIL_COMP_DXT4,
PIL_COMP_DXT5,
PIL_COMP_UNSUPPORTED,
PIL_COMP_COUNT
} pilcomps;

typedef enum
{
PIL_AUDIO_UNKNOWN=0,
PIL_AUDIO_PCM,
PIL_AUDIO_ADPCM,
PIL_AUDIO_ULAW,
PIL_AUDIO_ALAW,          // Microsoft ALAW
PIL_AUDIO_PCM_LE,        // uncompressed PCM in little-endian format
PIL_AUDIO_COUNT
} pilaudio;

/* Supported file types */
typedef enum
{
PIL_FILE_UNKNOWN=0,
PIL_FILE_WINBMP,	// Windows BMP
PIL_FILE_OS2BMP,	// OS/2 BMP
PIL_FILE_WINB2P,   // 2 bits per pel bitmap supported by CE
PIL_FILE_PCX,	// Paintshop PCX
PIL_FILE_DCX,	// multipage PCX = DCX
PIL_FILE_TIFF,	// TIFF
PIL_FILE_JFIF,	// JFIF JPEG
PIL_FILE_IOCA,	// IBM IOCA
PIL_FILE_AWD,	// Microsoft AWD Fax
PIL_FILE_TARGA,	// Truevision targa
PIL_FILE_PDF,	// Adobe PDF
PIL_FILE_GIF,	// Compuserve GIF
PIL_FILE_PNG,	// Portable Network Graphic
PIL_FILE_PSEG,	// IBM PSEG (page segment = advanced function printing - AFP)
PIL_FILE_WINFAX,	// Winfax undocumented file
PIL_FILE_BITFAX,	// another undocumented fax file
PIL_FILE_CALS,    // Government imaging format
PIL_FILE_QL2FAX,  // quicklinks fax
PIL_FILE_PPV,     // pocket powerpoint
PIL_FILE_FLC,     // FLI/FLC animation
PIL_FILE_AVI,     // Microsoft RIFF AVI
PIL_FILE_QT,      // Apple QuickTime
PIL_FILE_WF10,    // WinFAX version 10
PIL_FILE_AVMFAX,  // AVM Fritz-Fax
PIL_FILE_CANONRAW,     // Canon RAW format
PIL_FILE_MINOLTARAW,   // Minolta RAW format
PIL_FILE_OLYMPUSRAW,   // Olympus RAW format
PIL_FILE_FUJIRAW,      // Fuji RAW format
PIL_FILE_EFXFAX,       // Everex eFax
PIL_FILE_MPEG,         // MPEG-1
PIL_FILE_DICOM,        // DICOM medical images
PIL_FILE_FORTIS,       // *.MAG scanned image files
PIL_FILE_MODFAX,       // unknown fax
PIL_FILE_PPMAX,        // Paperport MAX
PIL_FILE_C4,           // JEDMICS C4
PIL_FILE_FAXIT,        // FAXIT
PIL_FILE_LASERFICHE,   // Laserfiche fax
PIL_FILE_PPM,          // portable pixmap (gray+color)
PIL_FILE_FITS,         // NASA FITS format
PIL_FILE_JP2,          // JPEG2000 JP2
PIL_FILE_SFFF,         // Structured fax file
PIL_FILE_DNG,          // Adobe Digital Negative (RAW image)
PIL_FILE_JBIG,          // JBIG "raw" file format
PIL_FILE_OPENEXR,	// ILM OpenEXR
PIL_FILE_DDS,		// Direct Draw Surface
PIL_FILE_KTX,		// Khronos texture
PIL_FILE_COUNT
} pilfiletypes;

#define PIL_PAGEFLAGS_BYTE			1	// word aligned rows
#define PIL_PAGEFLAGS_WORD			2	// word aligned rows
#define PIL_PAGEFLAGS_DWORD		4	// image has DWORD aligned rows
#define PIL_PAGEFLAGS_TOPDOWN		8	// image has row 0 starting at lowest address
#define PIL_PAGEFLAGS_BOTTOMUP	16	// image has row 0 starting at highest address
#define PIL_PAGEFLAGS_PREDICTOR  32 // lzw prediction
#define PIL_PAGEFLAGS_MOTOROLA   64 // Motorola byte order (TIFF files)
#define PIL_PAGEFLAGS_PLANAR     128 // TIFF image arranged in planes
//#define PIL_PAGEFLAGS_BWGRAY		64	// image has a black-to-white (0-ff) grayscale palette
//#define PIL_PAGEFLAGS_WBGRAY		128	// image has a white-to-black (0-ff) grayscale palette


#define PIL_BITDIR_MSB_FIRST     0
#define PIL_BITDIR_LSB_FIRST     1

#define PIL_READFLAGS_NONE          0

#define PIL_VIEWFLAGS_NONE		   	0	// no filtering
#define PIL_VIEWFLAGS_LIGHT			1	// bilevel density set to light
#define PIL_VIEWFLAGS_DARK		   	2	// bilevel density set to dark
#define PIL_VIEWFLAGS_SCALEGRAY		4	// scale bilevel to 4bpp gray
#define PIL_VIEWFLAGS_AVERAGE       8   // use pixel averaging when scaling down
#define PIL_VIEWFLAGS_ANNOTATIONS   16  // draw annotations (if available)
#define PIL_VIEWFLAGS_SIMD          32  // use SIMD to accelerate drawing

typedef enum
{
PIL_FILE_STATE_INVALID=0,			// image file state unknown/invalid
PIL_FILE_STATE_CLOSED,				// image file is closed
PIL_FILE_STATE_OPEN,  				// file is still open
PIL_FILE_STATE_LOADED,				// image data is loaded into pData of PIL_FILE
} pilfilestates;

#define PIL_PAGE_STATE_LOADED       1     // page is completely in memory
#define PIL_PAGE_STATE_OPEN         2     // currently open file

#define PIL_JPEG_FILTERED        1   // special flag indicating that the JPEG data has been pre-filtered
/* Conversion options */
#define PIL_CONVERT_QUALITY_MASK  0x0000000F   // bits reserved for specifying JPEG encode quality
#define PIL_CONVERT_16BPP         0x00000010   // convert to RGB565 in decoder
#define PIL_CONVERT_32BPP         0x00000020   // convert to ARGB in decoder
#define PIL_CONVERT_IGNORE_ERRORS 0x00000040   // Allow partial load of bad images
#define PIL_CONVERT_NOALLOC       0x00000080   // don't allocate output image buffer (for animation/video)
#define PIL_CONVERT_PROGRESSIVE   0x00000100   // progressive JPEG flag
#define PIL_CONVERT_HALFSIZE      0x00000200   // load JPEG as 1/2 size (really 1/4)
#define PIL_CONVERT_QUARTERSIZE   0x00000400   // load JPEG as 1/4 size (really 1/16)
#define PIL_CONVERT_EIGHTHSIZE    0x00000800   // load JPEG as 1/8 size (really 1/64)
#define PIL_CONVERT_THUMBNAIL     0x00001000   // load EXIF thumbnail or as 1/8 size if not present
#define PIL_CONVERT_ANIMATE       0x00002000   // retain original buffer for animation
#define PIL_CONVERT_LUMA_ONLY     0x00004000   // decode jpeg luma channel
#define PIL_CONVERT_JPEG_RGB      0x00008000   // Adobe can encode TIFF+JPEG images with a colorspace of RGB
#define PIL_CONVERT_JPEG_YCCK     0x00010000   // Adobe can encode TIFF+JPEG in YCCK colorspace (basically inverted YCbCr)
#define PIL_CONVERT_SIMD          0x00020000   // use SIMD instructions to accelerate the decode
#define PIL_CONVERT_SKIPEXIF      0x00040000   // don't bother reading EXIF info even if it's there
#define PIL_CONVERT_FOR_PDF       0x00080000   // the output will go to a PDF file, therefore will not have a palette, fix colors as needed
#define PIL_CONVERT_MULTITHREAD   0x00100000   // use multiple threads to encode/decode

#define PIL_READFLAGS_LOADALL     0x00200000   // load entire image into memory
#define PIL_READFLAGS_READMETA    0x00400000   // Read and collect unrecognized metadata

#define PIL_CONVERT_QUALITY_HIGHEST  0  // highest quality encoding
#define PIL_CONVERT_QUALITY_HIGH     1  // high quality encoding
#define PIL_CONVERT_QUALITY_MED      2  // medium quality encoding
#define PIL_CONVERT_QUALITY_LOW      3  // low quality encoding
#define PIL_CONVERT_QUALITY_RIDICULOUS 4 // Equivalent of Quality level 95. Keeps all high freq coefficients
#define PIL_CONVERT_QUALITY_SUBSAMPLE 8  // subsample the color values (4:2:0)
#define PIL_MAX_JPEG_QUALITY 15			// maximum valid value using all options
/* Photometric interpretation */
#define PIL_PHOTOMETRIC_WHITEISZERO  0
#define PIL_PHOTOMETRIC_BLACKISZERO  1
#define PIL_PHOTOMETRIC_RGB          2
#define PIL_PHOTOMETRIC_PALETTECOLOR 3
#define PIL_PHOTOMETRIC_TRANSPARENCY 4
#define PIL_PHOTOMETRIC_SEPARATED    5
#define PIL_PHOTOMETRIC_YCBCR        6
#define PIL_PHOTOMETRIC_YCCK         7
#define PIL_PHOTOMETRIC_CIELAB1      8
#define PIL_PHOTOMETRIC_CIELAB2      9
#define PIL_PHOTOMETRIC_CFA          10
#define PIL_PHOTOMETRIC_LINEARRAW    11
//#define PIL_PHOTOMETRIC_CFA			32803
//#define PIL_PHOTOMETRIC_LINEARRAW	34892
#define PIL_PHOTOMETRIC_NONE         99
/* Modification options */
typedef enum
{
PIL_MODIFY_INVALID=0,
PIL_MODIFY_ROTATE,
PIL_MODIFY_COLORS,
PIL_MODIFY_SIZE,
PIL_MODIFY_INVERT,
PIL_MODIFY_FLIPH,
PIL_MODIFY_FLIPV,
PIL_MODIFY_GRAY,
PIL_MODIFY_DESPECK,
PIL_MODIFY_LIGHTEN,
PIL_MODIFY_DARKEN,
PIL_MODIFY_FIXRES // fix resolution of fax images
} pilmodifyops;

typedef enum
{
	PIL_PDF_UNKNOWN = 0,
	PIL_PDF_PAGE,
	PIL_PDF_VALUE,      // simple numerical object
	PIL_PDF_COLORSPACE,
	PIL_PDF_DEVICERGB,
	PIL_PDF_DEVICEGRAY,
	PIL_PDF_BITSPERCOMPONENT,
	PIL_PDF_STREAM,
	PIL_PDF_THUMB,
	PIL_PDF_PROCSET,
	PIL_PDF_MEDIABOX,
	PIL_PDF_CROPBOX,
	PIL_PDF_CONTENTS,
	PIL_PDF_TYPE,
	PIL_PDF_KIDS,
	PIL_PDF_SMASK
} pdftypes;

// Multithreaded operations
#define PIL_THREAD_COMPLETE 0
#define PIL_THREAD_EXIT 1
#define PIL_THREAD_READ 2
#define PIL_THREAD_WRITE 3
#define PIL_THREAD_CONVERT 4
#define PIL_THREAD_BUSY 5

/* Color conversion options */
#define PIL_COLORS_BEST       1
#define PIL_COLORS_DITHER     2
#define PIL_COLORS_ERRORDIFF  4
#define PIL_COLORS_MEDIANCUT  8
#define PIL_COLORS_SIMD       16

#define PIL_OPTION_APPEND     1

// Structure for holding PDF objects
typedef struct pilpdf
{
int iType;           // PDF object type (e.g. PIL_PDF_IMAGE)
int iValue;          // integer value
} PIL_PDFObject;

#define PIL_STD_ERROR 0x10000000

#ifdef _64BITS
#define REGISTER_WIDTH 64
#define TOP_BIT 0x8000000000000000
#define MAX_VALUE 0xffffffffffffffff
#define LONGWHITECODEMASK 0x200000000000000
#define LONGBLACKCODEMASK 0x1000000000000000
#define GETMAXMOTOBITS MOTOEXTRALONG
#define BIGINT int64_t
#define BIGUINT uint64_t
typedef struct pil_buffered_bits
{
unsigned char *pBuf; // buffer pointer
uint64_t ulBits; // buffered bits
uint64_t ulBitOff; // current bit offset
uint64_t ulDataSize; // available data
} BUFFERED_BITS;
// structure for outputting variable length codes
typedef struct pil_code_tag
{
unsigned char *pOut; // current output pointer
int64_t iLen;  // length of data in accumulator
uint64_t ulAcc; // code accumulator (holds codes until at least 64-bits ready to write
} PIL_CODE;
#define STORECODE(pOut, iLen, ulCode, ulAcc, iNewLen) \
	if (iLen+iNewLen > 64) \
						{ uint64_t ul1, ul2 = ulAcc | (ulCode >> (iLen+iNewLen-64)); \
	ul1 = ul2 & (ul2 >> 4); ul1 &= (ul1 >> 2); ul1 &= (ul1 >> 1);  ul1 &= 0x0101010101010101; \
    if (ul1 == 0) \
	{*(uint64_t *)pOut = PILBSWAP64(ul2); pOut += 8; \
     iLen -= 64; ulAcc = 0;} else \
	{while (iLen >= 8) \
			{unsigned char c = (unsigned char)(ulAcc >> 56); *pOut++ = c; \
	if (c == 0xff) { *pOut++ = 0;} ulAcc <<= 8; iLen -= 8; }} \
	 iLen += iNewLen; ulAcc |= (ulCode << (64 - iLen));} \
		else { iLen += iNewLen; ulAcc |= (ulCode << (64-iLen)); }
#else // 32-bits
#define REGISTER_WIDTH 32
#define TOP_BIT 0x80000000
#define MAX_VALUE 0xffffffff
#define LONGWHITECODEMASK 0x2000000
#define LONGBLACKCODEMASK 0x10000000
#define GETMAXMOTOBITS MOTOLONG
#define BIGINT int32_t
#define BIGUINT uint32_t
// Structure for working with variable length codes
typedef struct pil_buffered_bits
{
unsigned char *pBuf; // buffer pointer
uint32_t ulBits; // buffered bits
uint32_t ulBitOff; // current bit offset
uint32_t ulDataSize; // available data
} BUFFERED_BITS;
// structure for outputting variable length codes
typedef struct pil_code_tag
{
unsigned char *pOut; // current output pointer
int32_t iLen;  // length of data in accumulator
uint32_t ulAcc; // code accumulator (holds codes until at least 32-bits ready to write
} PIL_CODE;
#define STORECODE(pOut, iLen, ulCode, ulAcc, iNewLen) \
	if (iLen+iNewLen > 32) { while (iLen >= 8) \
			{unsigned char c = (unsigned char)(ulAcc >> 24); *pOut++ = c; \
	if (c == 0xff) { *pOut++ = 0;} ulAcc <<= 8; iLen -= 8; }} \
	iLen += iNewLen; ulAcc |= (ulCode << (32-iLen));
#endif // _64BITS

#define CLIMBWHITE_NEW(pBuf, ulBitOff, ulBits, sCode) \
	{ BIGUINT ul; int iLen = 64; sCode = 0; while (iLen > 63) \
    { if (ulBitOff > (REGISTER_WIDTH-17)) \
	  { pBuf += (ulBitOff>>3); ulBitOff &= 7; ulBits = GETMAXMOTOBITS(pBuf); } \
	  if ((ulBits << ulBitOff) < LONGWHITECODEMASK) \
	    	  	{ ul = (ulBits >> ((REGISTER_WIDTH-14) - ulBitOff)) & 0x3fe; ulBitOff += black_l[ul]; iLen = black_l[ul+1];} \
				else {ul = (ulBits >> ((REGISTER_WIDTH - 10) - ulBitOff)) & 0x3fe; ulBitOff += white_s[ul]; iLen = white_s[ul+1];} \
     sCode += iLen; }}

#define CLIMBBLACK_NEW(pBuf, ulBitOff, ulBits, sCode) \
	{ BIGUINT ul; int iLen = 64; sCode = 0; while (iLen > 63) \
    { if (ulBitOff > (REGISTER_WIDTH-15)) \
	  { pBuf += (ulBitOff>>3); ulBitOff &= 7; ulBits = GETMAXMOTOBITS(pBuf); } \
	  if ((ulBits << ulBitOff) < LONGBLACKCODEMASK) \
	 	{ ul = (ulBits >> ((REGISTER_WIDTH-14) - ulBitOff)) & 0x3fe; ulBitOff += black_l[ul]; iLen = black_l[ul+1];} \
		else {ul = (ulBits >> ((REGISTER_WIDTH - 7) - ulBitOff)) & 0x7e; ulBitOff += black_s[ul]; iLen = black_s[ul+1];} \
     sCode += iLen; }}

// Current graphics state of the PDF renderer
typedef struct pil_pdf_state
{
uint32_t ulFillColor;
uint32_t ulStrokeColor;
float fLineWidth;
float fMiterLimit;
float x; // current point
float y;
float fFlatness;
int iLineJoin;
int iLineCap;
} PIL_PDF_STATE;

// Structure for scanning the contents of a JPEG image and holding
// the offsets and DC values of each MCU
typedef struct pil_jpeg_scan
{
uint32_t ulOffset; // byte offset of this MCU
uint32_t ulBitOff; // bit offset into the accumulated bits
uint32_t ulBits; // accumulated bits
signed int iYDC;	// Y DC value
signed int iCbDC;	// Cb DC value
signed int iCrDC;	// Cr DC value
} JPEG_SCAN;

typedef struct _piltables
{
unsigned short *pYUV16; // 16-bit conversion table (YCrCb->RGB565)
unsigned char *pYUVRed; // 8-bit conversion table for Y+Cr->Red
unsigned char *pYUVGreen; // 8-bit conversion table for Y+Cr+Cb->Green
unsigned char *pYUVBlue; // 8-bit conversion table for Y+Cb->Blue
} PILTABLES;

/* JPEG color component info */
typedef struct _jpegcompinfo
{
// These values are fixed over the whole image
// For compression, they must be supplied by the user interface
// for decompression, they are read from the SOF marker.
unsigned char component_needed;  /*  do we need the value of this component? */
unsigned char component_id;     /* identifier for this component (0..255) */
unsigned char component_index;  /* its index in SOF or cinfo->comp_info[] */
unsigned char h_samp_factor;    /* horizontal sampling factor (1..4) */
unsigned char v_samp_factor;    /* vertical sampling factor (1..4) */
unsigned char quant_tbl_no;     /* quantization table selector (0..3) */
// These values may vary between scans
// For compression, they must be supplied by the user interface
// for decompression, they are read from the SOS marker.
unsigned char dc_tbl_no;        /* DC entropy table selector (0..3) */
unsigned char ac_tbl_no;        /* AC entropy table selector (0..3) */
// These values are computed during compression or decompression startup
int true_comp_width;  /* component's image width in samples */
int true_comp_height; /* component's image height in samples */
// the above are the logical dimensions of the downsampled image
// These values are computed before starting a scan of the component
int MCU_width;        /* number of blocks per MCU, horizontally */
int MCU_height;       /* number of blocks per MCU, vertically */
int MCU_blocks;       /* MCU_width * MCU_height */
int downsampled_width; /* image width in samples, after expansion */
int downsampled_height; /* image height in samples, after expansion */
// the above are the true_comp_xxx values rounded up to multiples of
// the MCU dimensions; these are the working dimensions of the array
// as it is passed through the DCT or IDCT step.  NOTE: these values
// differ depending on whether the component is interleaved or not!!
// This flag is used only for decompression.  In cases where some of the
// components will be ignored (eg grayscale output from YCbCr image),
// we can skip IDCT etc. computations for the unused components.
} JPEGCOMPINFO;

#define DCTSIZE2  64
#define HUFF_TABLEN  273   // length of a Huffman table
#define MAX_COMPS_IN_SCAN  4 // maximum number of color components
#define MCU0 0
#define MCU1 64
#define MCU2 128
#define MCU3 192
#define MCU4 256
#define MCU5 320
//#define MCU6 384
//#define MCU7 448
//#define MCUTEMP 512

/* JPEG data structure */
typedef struct _jpegdata
{
int *pHuffAC, *pHuffDC;
int *pHuffACFast, *pHuffDCFast; // current huff table in use
unsigned char ucMaxACCol; // bits set indicate the column has non-zero coefficients
unsigned char ucMaxACRow; // bits set indicate the column has data in rows 4-7
unsigned char ucQuantTable; // current quantization table used by JPEGDecodeMCU()
unsigned char b11Bit; // indicates if the A/C Huffman decoding needs a max length of 11 bits (instead of the normal 10)
unsigned char ucDummy1[12]; // to keep pMCUs 16-byte aligned
int iOptions;
int *huffdc[2], *huffac[2];
int *huffdcFast[2], *huffacFast[2]; // faster table for codes <= 9 bits
short pMCUs[DCTSIZE2 * 6]; // MCU buffers - need up to 6 for 2:2 color subsampling
unsigned short sQuantTable[4*DCTSIZE2];
unsigned char cRangeTable[1024]; // for DCT
unsigned char cRangeTable2[1024]; // for YCC to RGB conversion
unsigned short usRangeTableR[1024]; // for direct 16bpp conversion
unsigned short usRangeTableG[1024];
unsigned short usRangeTableB[1024];
unsigned char ucHuffVals[HUFF_TABLEN*4];
unsigned char ucHuffTableUsed[4];
unsigned char ucHuffACDCBuf[2*0x100 + 2*0x1000]; // pre-allocate the maximum space for the Huffman decode tables
unsigned char *pPixel; // pointer to output pixels
uint32_t ulBits; // cached bits
int iResInterval;
int iResCount;
int iDataSize;
int iEXIF;        // Offset to JPEG EXIF info
int xdpi, ydpi;
int cx, cy;
uint32_t ulHeaderFlags; // flags indicating that all required header data is present
int bStrip;
int iScan, iScanStart, iScanEnd; // for progressive jpeg
int iOffset; // current offset into file
int iBit; // current bit position
int iMode; // JPEG SOF mode (C1,C2,C3,C4)
int iDataEnd; // Ending offset of data
int iScanOffset[32]; // offset to start of each scan (progressive)
int iHuffOffset[32]; // offset to start of huffman table for this scan
JPEGCOMPINFO JPCI[4]; /* Max color components */
//unsigned char ucAltHuff[3*0x4000]; // second table when we're working with "non-conformant" tables
char cApproxBitsLow, cApproxBitsHigh;
unsigned char ucNumComponents;
unsigned char ucComponentsInScan;
unsigned char ucBitsPerSample;
unsigned char jpegsample;
PILTABLES *pTables;
} JPEGDATA;
// Callback function for rendering a JPEG image
// returns the upper left corner coordinates, size and pixel data
typedef void (*PFNDRAWMCU)(int x, int y, int cx, int cy, uint16_t *pPixels);

// Structure for decoding a JPEG with multiple threads in parallel
// Created during the JPEGFilter() function when it finds restart markers
typedef struct pil_jpeg_slice
{
	volatile uint32_t uiFlag;	// thread completion flag
	int32_t iError;				// Error return code
	uint32_t uiResInterval;
	uint32_t uiResCount;
	unsigned char *pBitmap;		// Input/Output image buffer
	unsigned char *pData;		// Input/Output data pointer
	unsigned char *pPalette;	// for grayscale or 8-bit color images
	uint32_t uiLen;				// Input/Output data length
	uint32_t uiBitOffset;		// starting bit offset of this slice
	uint32_t uiPitch;			// output image pitch in bytes
	uint32_t uiWidth;			// Width of this slice in pixels
	uint32_t uiHeight;			// Height of this slice in pixels
	int32_t iDCPred0;			// DC prediction values to start this slice (also quality value for encoding)
	int32_t iDCPred1;
	int32_t iDCPred2;
    int32_t iDCPred3;
	JPEGDATA *pJPEG;
	int16_t pMCU[64 * 6];		// holds temp data for each minimum coded unit
} JPEG_SLICE;
//
// The amount of data read in one shot from the input file
// On embedded targets, we keep this small since we normally have very little RAM
//
#define FILE_BLOCK_SIZE 2048
/* Structure which holds a page of graphics data */
typedef struct _pilpage {
int iWidth, iHeight; // page size in pixels
unsigned char *szFile; // pointer to input file name (for reading from SD card)
unsigned char *pData;		// pointer to source image data
int iDataLoaded; // amount of filtered data available in the input buffer
int iHighWater;  // current high water mark for when to read more data
int iDataPos; // current position of input pointer relative to input data buffer
int iDataSize; // available filtered JPEG data
int iXres, iYres;	// Resolution in dots per inch
int iFileSize;    // Size of the input data or file
int iFilePos;     // current file read position (next read would start here)
File file;
JPEGDATA *pJPEG;           // jpeg decoder info
unsigned char ucFileBuf[3 * FILE_BLOCK_SIZE]; // 2 x block for filtered data and 1 x block for incoming file data
// Exif info
int iShutter;              // shutter speed
int iMetering;             // metering mode
int iFStop;                // F stop
int iExposure;             // exposure compensation
int iExposureProgram;      // aperture priority, full auto, etc.
int iISO;                  // ISO equivalent
int iFocalLength;
int iOriginalWidth;
int iOriginalHeight;
int iFlash;                // flash status bits
int iWhiteBalance;
int iOrientation;          // EXIF orientation: 1 = normal, 8 = 90 deg right, 3 = 180 deg, 6 = 270 deg
int  iAltitude;            // GPS altitude in meters
signed int iLatDeg;        // Latitude in degrees; negative = South, positive = North
int iLatMin;               // Latitude minutes * 100
int iLatSec;               // Latitude seconds * 1000
signed int iLongDeg;       // Longitude in degrees; negative = West, positive = East
int iLongMin;               // Longitude minutes * 100
int iLongSec;               // Longitude seconds * 1000
// DICOM info
int iWindowCenter;         // DICOM image contrast center value
int iWindowWidth;          // DICOM image contrast width value
int iPadding;              // DICOM padding pixel value
float fSlope;              // DICOM slope
float fIntercept;          // DICOM intercept
char szDateTime[32];       // ASCII date and time
char szComment[128];       // Comment
char szLatitude[16];       // GPS ASCII string of latitude (e.g. 47.23N)
char szLongitude[16];      // GPS ASCII string of longitude (e.g. 115.14W)
char cBitsperpixel;			// Supported values = {1,4,8,16,24,32}
char cFlags;			   	// includes alignment, top/bottom, etc.
char cState;				   // current state of this page
unsigned char cJPEGSubSample; // TIFF type 6 stores this info outside of the data block
unsigned char cJPEGMode; // 0xc0 = baseline, 0xc1 = extended, 0xc2 = progressive, 0xc3 = lossless
unsigned char cCompression;
unsigned char cBitDir;
unsigned char cPhotometric;
unsigned char bIsFF; // boolean indicating if the last byte filtered is an FF
int iOffset;
int iPitch;
int iFrameDelay;
} PIL_PAGE;

typedef struct tagPILRGBQUAD {
  unsigned char rgbBlue;
  unsigned char rgbGreen;
  unsigned char rgbRed;
  unsigned char rgbReserved;
} PILRGBQUAD;

typedef struct tagpilrect
{
	uint32_t Left;
	uint32_t Top;
	uint32_t Right;
	uint32_t Bottom;
} PILRECT;

#define PIL_LF_FACESIZE 32
typedef struct tagPILLOGFONT {
  signed long  lfHeight;
  signed long  lfWidth;
  signed long  lfEscapement;
  signed long  lfOrientation;
  signed long  lfWeight;
  unsigned char  lfItalic;
  unsigned char  lfUnderline;
  unsigned char  lfStrikeOut;
  unsigned char  lfCharSet;
  unsigned char  lfOutPrecision;
  unsigned char  lfClipPrecision;
  unsigned char  lfQuality;
  unsigned char  lfPitchAndFamily;
  unsigned char lfFaceName[32 /*PIL_LF_FACESIZE*/];
} PILLOGFONT;
// Wang/Eastman annotation mark attributes structure
typedef struct tagOIAN_MARK_ATTRIBUTES
{
unsigned int uType;                                   // The type of the mark.
                                                                // 1 = Image embedded
                                                                // 2 = Image reference
                                                                // 3 = Straight line
                                                                // 4 = Freehand line
                                                                // 5 = Hollow rectangle
                                                                // 6 = Filled rectangle
                                                                // 7 = Typed text
                                                                // 8 = Text from file
                                                                // 9 = Text stamp
                                                                //10 = Attach-a-Note
                                                                //12 = Form
                                                                //13 = OCR region
PILRECT lrBounds;                             // Rectangle in FULLSIZE units; equivalent to type RECT.
                                                                // Can be a rectangle or two points.
PILRGBQUAD rgbColor1;                             // The main color; for example, the color of all lines,
                                                                // all rectangles, and standalone text.
PILRGBQUAD rgbColor2;                             // The secondary color; for example, the color of the text of
                                                                // an Attach-a-Note.
int bHighlighting;                     // TRUE The mark is drawn highlighted. Highlighting
                                                                // performs the same function as a highlighting marker on a
                                                                // piece of paper. Valid only for lines, rectangles, and
                                                                // freehand.
int bTransparent;                      // TRUE The mark is drawn transparent. A transparent
                                                                // mark does not draw white pixels. That is, transparent
                                                                // replaces white pixels with whatever is behind those pixels.
                                                                // Available only for images.
unsigned int uLineSize;                             // The width of the line in pixels.
unsigned int uReserved1;                        // Reserved; must be set to 0.
unsigned int uReserved2;                        // Reserved; must be set to 0.
PILLOGFONT lfFont;                    // The font information for the text, consisting of standard
                                                                // font attributes of font size, name, style, effects, and
                                                                // background color.
uint32_t bReserved3;                        // Reserved; must be set to 0.
//time_t Time;                                    // The time that the mark was first saved, in seconds, from
                                                                // 00:00:00 1-1-1970 GMT. Every annotation mark has
                                                                // time as one of its attributes. If you do not set the time before
                                                                // the file is saved, the time is set to the date and time that the
                                                                // save was initiated. This time is in the form returned by the
                                                                // "time" C call, which is the number of seconds since
                                                                // midnight 00:00:00 on 1-1-1970 GMT. If necessary, refer
                                                                // to your C documentation for a more detailed description.
int bVisible;                                // TRUE The mark is currently set to be visible.
                                                                // Annotation marks can be visible or hidden.
uint32_t dwReserved4;                // Reserved; must be set to 0x0FF83F.
int32_t lReserved[10];                      // Must be set to 0.
}OIAN_MARK_ATTRIBUTES;


#define MAX_PAGES 4096  // maximum supported pages
/* Errors */
#define PIL_MAX_ERROR         16
#define PIL_ERROR_SUCCESS     0   // no error
#define PIL_ERROR_MEMORY      -1  // out of memory
#define PIL_ERROR_FILENF      -2  // file not found
#define PIL_ERROR_IO          -3  // I/O error
#define PIL_ERROR_DECOMP      -4  // decompression error
#define PIL_ERROR_BITDEPTH    -5  // unsupported pixel bit depth
#define PIL_ERROR_INVPARAM    -6  // invalid parameter
#define PIL_ERROR_UNSUPPORTED -7  // unsupported feature
#define PIL_ERROR_BADHEADER   -8  // bad data encountered in header
#define PIL_ERROR_UNKNOWN     -9  // unknown file type
#define PIL_ERROR_PAGENF      -10 // page not found
#define PIL_ERROR_CANCELED    -11 // operation was cancelled
#define PIL_ERROR_LICENSE     -12 // unlicensed operation
#define PIL_ERROR_EMPTY_PAGE  -13 // an empty (no data) page
#define PIL_ERROR_METANF      -14 // metadata tag not found
#define PIL_ERROR_ENCRYPTED   -15 // file contains encrypted data that cannot be read
#define PIL_ERROR_UNSUP_COMP  -16 // unsupported compression type
#define PIL_ERROR_AUDIO_CHAN_NOT_SUPPORTED -17	// # of audio channels not supported
#define PIL_ERROR_AUDIO_BITS_NOT_SUPPORTED -18	// Bit depth of audio stream not supported
#define	PIL_ERROR_AUDIO_HW_NOT_SET		   -19	// Audio hardware parameters not set
#define PIL_ERROR_AUDIO_TYPE_UNKNOWN	   -20	// Unknown audio stream in video file (no CODEC)

#ifndef MEMORY_MACROS
#define MEMORY_MACROS
#define INTELSHORT(p) ((*p) + (*(p+1)<<8))
#define INTELEXTRALONG(p) (*(uint64_t *)p)
#define MOTOSHORT(p) (((*(p))<<8) + (*(p+1)))
#ifdef _WIN32
#define MOTOEXTRALONG(p) (_byteswap_uint64(*(__int64 *)p))
#define MOTOLONG(p) (_byteswap_ulong(*(__int32 *)p))
#define INTELLONG(p) (*(unsigned __int32 *)p)
#ifdef _64BITS
//#pragma intrinsic(_byteswap_uint64)
//#pragma intrinsic(_BitScanForward64)
#else
#pragma intrinsic(_byteswap_ulong)
#pragma intrinsic(_BitScanForward)
#endif // _64BITS
#else // Linux
#define MOTOEXTRALONG(p) (__builtin_bswap64(*(uint64_t *)p))
#ifdef _X86
#define INTELLONG(p) (*(uint32_t *)p)
#define MOTOLONG(p) (__builtin_bswap32(*(uint32_t *)p))
#else // ARM
// On ARM 32-bit systems, we must assume that unaligned reads will cause an exception, so read these one byte at a time
#define INTELLONG(p) ((*(unsigned char *)p) + (*((unsigned char *)p+1)<<8) + (*((unsigned char *)p+2)<<16) + (*((unsigned char *)p+3)<<24))
#define MOTOLONG(p) (((*(unsigned char *)p)<<24) + ((*((unsigned char *)p+1))<<16) + ((*((unsigned char *)p+2))<<8) + (*((unsigned char *)p+3)))
#endif // _X86
#endif // _WIN32
#define MOTO24(p) ((*(p))<<16) + ((*(p+1))<<8) + (*(p+2))
#define WRITEPATTERN32(p, o, l) p[o] |= (unsigned char)(l >> 24); p[o+1] |= (unsigned char)(l >> 16); p[o+2] |= (unsigned char)(l >> 8); p[o+3] |= (unsigned char)l;
#define WRITEMOTO32(p, o, val) {uint32_t l = val; p[o] = (unsigned char)(l >> 24); p[o+1] = (unsigned char)(l >> 16); p[o+2] = (unsigned char)(l >> 8); p[o+3] = (unsigned char)l;}
#define WRITEMOTO16(p, o, val) {uint32_t l = val; p[o] = (unsigned char)(l >> 8); p[o+1] = (unsigned char)l;}
#endif


/* Structure defining a view of a page of graphics data */
typedef struct _pilview {
int iSize;                 // size of the PIL_VIEW structure for version checking
int iWinX, iWinY;		   	// offset of view in document pixels
int iScaleX, iScaleY;		// scaling factor in multiples of 1/256
int iWidth, iHeight;		   // destination bitmap size
int iPitch;                // destination bytes per line
int iOrientation;          // View angle = 0,90,180,270
unsigned char *pBitmap;		// destination bitmap pointer
char cFilter;			   	// filtering options (e.g. scale to gray)
} PIL_VIEW;


typedef struct _tifftag
    {
    unsigned short sTagID;     /* TIFF Tag */
    unsigned short sDataType;  /* Tag data type */
    uint32_t  lNumVals;   /* Number of values */
    uint32_t  lValue;     /* Tag value */
    } TIFFTAG;

/* IBM PSEG image cell descriptor */
typedef struct _imgcell
    {
    int curx, cury;     /* Cell position */
    int cellx, celly;   /* Cell size */
    int fillx, filly;   /* Fill rectangle size of current image block */
    } IMGCELL;

/* Structure which holds current file info */
typedef struct _pilfile {
	int iSize;                 // size of the PIL_FILE structure for version checking
	void *  lUser;            // user defined
	void *	iFile;	   		// file handle
	int      iFileSize;        // size of source file
	unsigned char *pData;		// pointer to memory mapped file
	int *pPageList;				// list of page offsets (e.g. for TIFF performance)
	int *pSoundList;           // list of sound chunk offsets (video and audio)
	int *pPageLens;            // Length of each page
	int *pSoundLens;           // Length of each sound chunk
	unsigned char *pKeyFlags;  // flags indicating key frames of video
   JPEGDATA *pJPEG;           // Precalc'd tables for JPEG + video files
	int iPage, iPageTotal;		// current page and total pages
	int iSoundTotal;           // number of sound chunks
	int iSampleFreq;           // sound sample frequency
	int iSoundLen;             // total size of sound data
	int iADPCMBlock;           // block size of ADPCM data
	int iFrameRate;            // Video frame rate in fps
	int iFrameDelay;           // Video frame delay in ms
	int iWindowCenter;         // DICOM image contrast center value
	int iWindowWidth;          // DICOM image contrast width value
    int iPadding;              // DICOM pixel padding value
	int iX, iY;                // Page size
	int iOrientation;		   // EXIF orientation
	int iThumbCX, iThumbCY;    // thumbnail size (if present)
    float fSlope;              // DICOM slope
    float fIntercept;          // DICOM intercept
	char cSoundBits;           // for video
	char cBpp;                 // bits per pixel written to the file
    char cBppStored;           // bits per pixel of actual data
    char cPixelRep;            // DICOM pixel representation (0=unsigned, 1=signed)
    char cPhotometric;			// photometric interpretation (or brightness for JPEG)
	unsigned char cJPEGMode;   // JPEG mode marker (e.g. 0xc0, 0xc1, 0xc2, 0xc3)
	unsigned char cCompression;     // for video
	pilaudio cAudioCodec;      // for video
   char cAudioChannels;       // for video
	pilfiletypes cFileType;		// file type
	pilfilestates cState;		// current state of this file

   // Audio segment (if applicable)
#ifdef PIL_AUDIO_INCLUDED
	PILHALError tLastError;		// Last filesystem HAL error encountered
	PILAUDIO AudioStream;		// Audio structure for this video file (if applicable)
#endif	// #ifdef PIL_AUDIO_INCLUDED
    int iAnnotationSize;
    int iAnnotationOffset;
} PIL_FILE;

#ifdef __cplusplus
extern "C" {
#endif
/*--- Functions of the PIL ---*/
int PILCalcSize(int x, int bpp);
int PILCalcBSize(int x, int bpp);
int PILThreshold(PIL_PAGE *pInPage, PIL_PAGE *pOutPage, int iOptions);
int PILDespeckle(PIL_PAGE *id, int iMaxWidth, int iColor);
int PILRenderDICOM(PIL_PAGE *pPage, int iWindowWidth, int iWindowCenter);
int PILDraw(PIL_PAGE *pPage, PIL_VIEW *pView, int bTopDown, void *pGammaBrightness);
int PILOpen(TCHAR *szFileName, PIL_FILE *pFile, int iOptions, TCHAR *szCompany, uint32_t ulKey);
int PILRead(PIL_FILE *pFile, PIL_PAGE *pPage, int iRequestedPage, int iOptions);
int PILClose(PIL_FILE *pFile);
int PILCreate(TCHAR *szFileName, PIL_FILE *pFile, int iOptions, int iFileType);
int PILWrite(PIL_FILE *pFile, PIL_PAGE *pPage, int iFlags);
int PILConvert(PIL_PAGE *pInPage, PIL_PAGE *pOutPage, int iOptions, PILPROGRESS, PILTABLES *pTables);
int PILFree(PIL_PAGE *pPage);
int PILResize(PIL_PAGE *pInPage, PIL_PAGE *pOutPage, int iNewX, int iNewY);
int PILCrop(PIL_PAGE *pPage, PIL_VIEW *pView);
int PILModify(PIL_PAGE *pPage, pilmodifyops iOperation, int iParam1, int iParam2);
int PILAnimateGIF(PIL_PAGE *pPage, PIL_PAGE *pAnimatePage);
int PILAnimatePNG(PIL_PAGE *pPage, PIL_PAGE *pAnimatePage);
int PILRotateJPEG(TCHAR *szSource, TCHAR *szDest, int iAngle);
int PILScanJPEG(JPEG_SCAN **pScanList, BUFFERED_BITS *bb, JPEGDATA *pJPEG);
int PILCreatePDF(TCHAR *fname, unsigned char *pOutput, int *iOutLen, PIL_PAGE **pPages, int iPageCount, char *szApplication, char *szProducer);
int PILCreateTIFF(TCHAR *fname, unsigned char *pOutput, int *iOutLen, PIL_PAGE **pPages, int iPageCount, char *szApplication, char *szProducer);
//void PILFreeMPEGStruct(MPEGDATA *pMPEG, int bMPEG);
int PILReadMeta(PIL_PAGE *pPage, int bByIndex, int iTagIndex, unsigned char *pucType, unsigned char *pOut, int *pLength);
int PILWriteMeta(PIL_PAGE *pPage, int iTag, unsigned char ucType, unsigned char *pData, int iLength);
int PILDeleteMeta(PIL_PAGE *pPage, int iTag);
int PILGray(PIL_PAGE *inpage);
unsigned char *PILGrayPalette(int iBpp);
unsigned char * PILHSVPalette();
int PILIsGray(PIL_PAGE *inpage);
void PILReverseRB(PIL_PAGE *pInPage);
int PILDeskew(PIL_PAGE *inpage);
int PILMedianCut(PIL_PAGE *pInPage, PIL_PAGE *pOutPage, int iDestBpp);
int PILCreateFileData(int iType, PIL_PAGE *pPage, unsigned char **pHeader, int *iHeaderLen, unsigned char **pBody, int *iBodyLen);
PILTABLES *PILInitTables(void);
void PILFreeTables(PILTABLES *);
int PILTest(TCHAR *szFileName);
int PILAutoCrop(PIL_PAGE *pPage, int bBlackBorder);
void PILFixTIFFRGB(unsigned char *pBuffer, PIL_PAGE *pPage);
JPEGDATA * PILPrepJPEGStruct(void);
int PILShearRotate(PIL_PAGE *pPage, double dAngle);
unsigned char * PILEncodeLine(int xsize, unsigned char *irlcptr, unsigned char *buf);
int PILReadJPEG(char *fname, PIL_PAGE *inpage, PFNDRAWMCU pfnDrawMCU, int iOptions);

#ifdef __cplusplus
}
#endif

#endif // #ifndef _PIL_H_
