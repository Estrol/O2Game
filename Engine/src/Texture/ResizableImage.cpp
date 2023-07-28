#include "Texture/ResizableImage.h"
#include <windows.h>

#include "Rendering/Renderer.h"

ResizableImage::ResizableImage() : ResizableImage(1, 1, (char)0xFF) {}

ResizableImage::ResizableImage(int X, int Y, char color) : Texture2D::Texture2D() {
	BITMAPFILEHEADER bmfHeader = { 0 };
	BITMAPINFOHEADER bi = { 0 };

	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = 1;
	bi.biHeight = 1;
	bi.biPlanes = 1;
	bi.biBitCount = 16;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = X * Y * 2;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	bmfHeader.bfType = 0x4D42;
	bmfHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + bi.biSizeImage;
	bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);
	bmfHeader.bfReserved1 = 0;
	bmfHeader.bfReserved2 = 0;

	int biStride = (X * bi.biBitCount + 31) / 32 * 4;
	int biSizeImage = abs(biStride) * Y;

	uint8_t* pixels = new uint8_t[biSizeImage];
	memset(pixels, color, biSizeImage);

	int size = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + biSizeImage;
	uint8_t* buffer = new uint8_t[size];
	memcpy(buffer, &bmfHeader, sizeof(BITMAPFILEHEADER));
	memcpy(buffer + sizeof(BITMAPFILEHEADER), &bi, sizeof(BITMAPINFOHEADER));
	memcpy(buffer + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER), pixels, biSizeImage);


	delete[] pixels;

	Texture2D::LoadImageResources(buffer, size);
	Size = UDim2::fromOffset(X, Y);
	m_actualSize = { 0, 0, X, Y };
	AnchorPoint = { 0, 1 };
}
