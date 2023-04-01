#include "DrawableLine.hpp"
#include <windows.h>

DrawableLine::DrawableLine() : Tile2D::Tile2D() {
	BITMAPFILEHEADER bmfHeader = { 0 };
	BITMAPINFOHEADER bi = { 0 };

	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = 1;
	bi.biHeight = 1;
	bi.biPlanes = 1;
	bi.biBitCount = 16;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 1 * 1 * 2;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	bmfHeader.bfType = 0x4D42;
	bmfHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + bi.biSizeImage;
	bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);
	bmfHeader.bfReserved1 = 0;
	bmfHeader.bfReserved2 = 0;

	int biStride = (1 * bi.biBitCount + 31) / 32 * 4;
	int biSizeImage = abs(biStride) * 1;

	uint8_t* pixels = new uint8_t[biSizeImage];
	memset(pixels, 0xFF, biSizeImage); 

	int size = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + biSizeImage;
	uint8_t* buffer = new uint8_t[size];
	memcpy(buffer, &bmfHeader, sizeof(BITMAPFILEHEADER));
	memcpy(buffer + sizeof(BITMAPFILEHEADER), &bi, sizeof(BITMAPINFOHEADER));
	memcpy(buffer + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER), pixels, biSizeImage);

	Tile2D::LoadImageResources(buffer, size);
	delete[] pixels;

	m_pSpriteBatch = Renderer::GetInstance()->GetSpriteBatch(1);
	Size = UDim2::fromOffset(198, 1);
	AnchorPoint = { 0, 1 };
}
