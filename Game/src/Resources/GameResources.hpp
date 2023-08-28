#pragma once

#include <iostream>
#include <vector>
#include <unordered_map>
#include <SDL2/SDL.h>

#include "Rendering/WindowsTypes.h"
#include "Rendering/Vulkan/Texture2DVulkan.h"

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
	LANE_1,
	LANE_2,
	LANE_3,
	LANE_4,
	LANE_5,
	LANE_6,
	LANE_7,

	HOLD_LANE_1,
	HOLD_LANE_2,
	HOLD_LANE_3,
	HOLD_LANE_4,
	HOLD_LANE_5,
	HOLD_LANE_6,
	HOLD_LANE_7,

	TRAIL_UP,
	TRAIL_DOWN,
};

enum class O2ResourceType {
	O2_INTERFACE,
	O2_PLAYING,
	O2_AVATAR
};

struct NoteImage {
	std::vector<SDL_Texture*> Texture;
	std::vector<SDL_Surface*> Surface;

	std::vector<Texture2D_Vulkan*> VulkanTexture;
	Rect TextureRect;
};

namespace GameAvatarResource {
	bool Load();
	bool Dispose();

	OPIFile* GetFile(std::string filename);
	ESTHANDLE* LoadFileData(OPIFile* file);
}

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