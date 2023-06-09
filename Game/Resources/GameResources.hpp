#pragma once

#include <iostream>
#include <unordered_map>
#include <d3d11.h>
#include <SDL2/SDL.h>

typedef void* ESTHANDLE;

struct Boundary {
	int X;
	int Y;
	int Width;
	int Height;
};

struct OJSFrame : Boundary {
	OJSFrame(int X, int Y, int Width, int Height, short TransColor, int FrameOffset, int FrameSize);
	~OJSFrame();

	uint16_t TransparencyColor;
	int FrameOffset;
	int FrameSize;

	uint8_t* Buffer = nullptr;
};

struct BND {
	int Count;
	std::vector<Boundary> Coordinates;
};

struct OJS {
	char Name[128];
	int FrameCount;
	int RGBFormat;
	short TransparencyCode;
	std::vector<std::unique_ptr<OJSFrame>> Frames;
};

struct OPIFile {
	char Name[128];
	int FileOffset;
	int FileSize;
};

enum class NoteImageType {
	WHITE,
	BLUE,
	YELLOW,

	HOLD_WHITE,
	HOLD_BLUE,
	HOLD_YELLOW,
};

struct NoteImage {
	SDL_Texture* Texture;
	SDL_Surface* Surface;
	RECT TextureRect;
};

namespace GameInterfaceResource {
	bool Load();
	bool Dispose();

	OPIFile* GetFile(std::string name);
	ESTHANDLE* LoadFileData(OPIFile* file);
}

namespace GamePlayingResource {
	bool Load();
	bool Dispose();

	OPIFile* GetFile(std::string name);
	ESTHANDLE* LoadFileData(OPIFile* file);
}

namespace GameNoteResource {
	bool Load();
	bool Dispose();

	NoteImage* GetNoteTexture(NoteImageType noteType);
}