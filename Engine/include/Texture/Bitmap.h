#pragma once

// Complimentary to the BITMAPFILEHEADER and BITMAPINFOHEADER structs
// Also see: https://docs.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapfileheader

#if __GNUC__ && !defined(_WIN32)

// define WORD, DWORD, LONG for linux
#ifndef WORD
#define WORD uint16_t
#endif
#ifndef DWORD
#define DWORD uint32_t
#endif
#ifndef LONG
#define LONG int32_t
#endif

typedef struct tagBITMAPFILEHEADER
{
    WORD  bfType;      // 2  /* Magic identifier */
    DWORD bfSize;      // 4  /* File size in bytes */
    WORD  bfReserved1; // 2
    WORD  bfReserved2; // 2
    DWORD bfOffBits;   // 4 /* Offset to image data, bytes */
} __attribute__((packed)) BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER
{
    DWORD biSize;          // 4 /* Header size in bytes */
    LONG  biWidth;         // 4 /* Width of image */
    LONG  biHeight;        // 4 /* Height of image */
    WORD  biPlanes;        // 2 /* Number of colour planes */
    WORD  biBitCount;      // 2 /* Bits per pixel */
    DWORD biCompression;   // 4 /* Compression type */
    DWORD biSizeImage;     // 4 /* Image size in bytes */
    LONG  biXPelsPerMeter; // 4
    LONG  biYPelsPerMeter; // 4 /* Pixels per meter */
    DWORD biClrUsed;       // 4 /* Number of colours */
    DWORD biClrImportant;  // 4 /* Important colours */
} __attribute__((packed)) BITMAPINFOHEADER;

typedef enum {
    BI_RGB = 0,
    BI_RLE8 = 1,
    BI_RLE4 = 2,
    BI_BITFIELDS = 3,
    BI_JPEG = 4,
    BI_PNG = 5,
    BI_CMYK = 11,
    BI_CMYKRLE8 = 12,
    BI_CMYKRLE4 = 13
} CompressionType;

#endif