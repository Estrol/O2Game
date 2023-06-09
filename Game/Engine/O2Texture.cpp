#include "O2Texture.hpp"
#include "Lodepng.h"

struct rgba_t {
	uint8_t R;
	uint8_t G;
	uint8_t B;
	uint8_t A;

	rgba_t(uint16_t value) {
		B = value & 31;
		B <<= 3;

		value >>= 5;
		G = value & 31;
		G <<= 3;

		value >>= 5;
		R = value & 31;
		R <<= 3;

		A = 255;
	}

	int value() {
		return (A << 24) | (B << 16) | (G << 8) | R;
	}
};

O2Texture::O2Texture(OJSFrame* frame) {
	if (frame->FrameSize == 0) {
		throw std::invalid_argument("Invalid frame buffer size!");
	}
	
	if (frame->Buffer == nullptr) {
		throw std::invalid_argument("Invalid buffer pointer!");
	}

	int frameSize = frame->FrameSize;
	if (frameSize % 2 != 0) {
		frameSize += 1;
	}

	int biStride = (frame->Width * 32 + 31) / 32 * 4;
	int biSizeImage = abs(biStride) * frame->Height;

	uint8_t* raw_data = new uint8_t[frameSize];
	if (!raw_data) {
		throw std::runtime_error("Out of memory");
	}

	memcpy(raw_data, frame->Buffer, frame->FrameSize);

	uint8_t* m_pBuffer;
	size_t m_bufferSize;
	
	if (frame->TransparencyColor != 0) {
		std::vector<uint8_t> image_data(biSizeImage);
		rgba_t trans(frame->TransparencyColor);

		int offset16 = 0;
		int offset32 = 0;
		while (offset16 + 2 <= frameSize) {
			uint16_t tmp16 = *(uint16_t*)(raw_data + offset16);
			offset16 += 2;

			rgba_t pixel(tmp16);
			if (frame->TransparencyColor != 0
				&& pixel.R == trans.R
				&& pixel.G == trans.G
				&& pixel.B == trans.B) {

				pixel.R = 0;
				pixel.G = 0;
				pixel.B = 0;
				pixel.A = 0;
			}

			*(uint32_t*)(&image_data[0] + offset32) = pixel.value();
			offset32 += 4;
		}

		std::vector<uint8_t> result_buffer;
		uint32_t error = lodepng::encode(result_buffer, image_data, frame->Width, frame->Height, LCT_RGBA, 8);
		if (error != 0) {
			throw std::runtime_error("Failed to encode the image");
		}

		m_pBuffer = new uint8_t[result_buffer.size()];
		if (!m_pBuffer) {
			throw std::runtime_error("Out of memory");
		}

		memcpy(m_pBuffer, result_buffer.data(), result_buffer.size());
		m_bufferSize = result_buffer.size();
	}
	else {
		BITMAPFILEHEADER bmfHeader = { 0 };
		BITMAPINFOHEADER bi = { 0 };

		bi.biSize = sizeof(BITMAPINFOHEADER);
		bi.biWidth = frame->Width;
		bi.biHeight = frame->Height;
		bi.biPlanes = 1;
		bi.biBitCount = 16;
		bi.biCompression = BI_RGB;
		bi.biSizeImage = frame->Width * frame->Height * 2;
		bi.biXPelsPerMeter = 0;
		bi.biYPelsPerMeter = 0;
		bi.biClrUsed = 0;
		bi.biClrImportant = 0;

		bmfHeader.bfType = 0x4D42;
		bmfHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + bi.biSizeImage;
		bmfHeader.bfReserved1 = 0;
		bmfHeader.bfReserved2 = 0;

		int biStride = (frame->Width * bi.biBitCount + 31) / 32 * 4;
		int biSizeImage = abs(biStride) * frame->Height;

		bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);

		uint8_t* data;
		int totalSize = 0;

		int strideDiff = biStride / bi.biWidth * bi.biWidth;
		if (strideDiff == biStride) {
			data = new uint8_t[bi.biSizeImage];
			if (!data) {
				throw std::bad_alloc();
			}

			totalSize = bi.biSizeImage;

			memcpy(data, frame->Buffer, bi.biSizeImage);
		}
		else {
			data = new uint8_t[biSizeImage];
			if (!data) {
				throw std::bad_alloc();
			}

			totalSize = biSizeImage;
			memset(data, 0, biSizeImage);

			for (int y = 0; y < bi.biHeight; y++) {
				memcpy(data + y * biStride, frame->Buffer + y * strideDiff, strideDiff);
			}
		}

		uint8_t* tmp = new uint8_t[totalSize];
		if (!tmp) {
			throw std::bad_alloc();
		}

		if (biStride > 0) {
			for (int y = 0; y < bi.biHeight; y++) {
				memcpy(tmp + y * biStride, data + (bi.biHeight - y - 1) * biStride, biStride);
			}
		}
		else {
			memcpy(tmp, data, totalSize);
		}

		int size = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + totalSize;
		m_pBuffer = new uint8_t[size];
		if (!m_pBuffer) {
			throw std::bad_alloc();
		}

		memcpy(m_pBuffer, &bmfHeader, sizeof(BITMAPFILEHEADER));
		memcpy(m_pBuffer + sizeof(BITMAPFILEHEADER), &bi, sizeof(BITMAPINFOHEADER));
		memcpy(m_pBuffer + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER), tmp, totalSize);

		m_bufferSize = size;
	}

	delete[] raw_data;
	Texture2D::LoadImageResources(m_pBuffer, m_bufferSize);
	LoadImageResources(frame);

	m_bDisposeTexture = true;
}

void O2Texture::LoadImageResources(Boundary* position) {
	Position.X.Offset = position->X;
	Position.Y.Offset = position->Y;
}
