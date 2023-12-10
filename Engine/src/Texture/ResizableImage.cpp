#include "Texture/ResizableImage.h"
#include "Rendering/Renderer.h"
#include "Texture/Bitmap.h"

ResizableImage::ResizableImage() : ResizableImage(1, 1, (char)0xFF) {}

ResizableImage::ResizableImage(int X, int Y, char color) : Texture2D::Texture2D()
{
    BITMAPFILEHEADER bmfHeader = { 0 };
    BITMAPINFOHEADER bi = { 0 };

    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = X;
    bi.biHeight = Y;
    bi.biPlanes = 1;
    bi.biBitCount = 16;
    bi.biCompression = BI_RGB;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    int biStride = (X * bi.biBitCount + 31) / 32 * 4;
    bi.biSizeImage = biStride * Y;

    bmfHeader.bfType = 0x4D42;
    bmfHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + bi.biSizeImage;
    bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);
    bmfHeader.bfReserved1 = 0;
    bmfHeader.bfReserved2 = 0;

    uint8_t *pixels = new uint8_t[bi.biSizeImage];
    memset(pixels, 0, bi.biSizeImage); // initialize to black or some other color

    for (int y = 0; y < Y; y++) {
        uint8_t *row = pixels + y * biStride;
        for (int x = 0; x < X; x++) {
            row[x * 2] = color & 0xFF;
            row[x * 2 + 1] = (color >> 8) & 0xFF;
        }
    }

    int      size = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + bi.biSizeImage;
    uint8_t *buffer = new uint8_t[size];
    memcpy(buffer, &bmfHeader, sizeof(BITMAPFILEHEADER));
    memcpy(buffer + sizeof(BITMAPFILEHEADER), &bi, sizeof(BITMAPINFOHEADER));
    memcpy(buffer + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER), pixels, bi.biSizeImage);

    delete[] pixels;

    Texture2D::LoadImageResources(buffer, size);
    Size = UDim2::fromOffset(X, Y);
    m_actualSize = { 0, 0, X, Y };
    AnchorPoint = { 0, 1 };
}
