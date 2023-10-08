#pragma once
#include <string>

enum class SkinGroup {
	Main,
	MainMenu,
	Notes,
	Playing,
	SongSelect
};

enum class SkinDataType {
	Numeric,
	Position,
	Rect,
	Note,
	Sprite
};

struct LaneInfo {
    int HitPosition;
    int LaneOffset;
};

// Value Format: X, Y, AnchorPointX?, AnchorPointY?, TintColor?
struct PositionValue {
	double X, Y;
	float AnchorPointX, AnchorPointY;
	unsigned char RGB[3];
};

// Value format: X, Y, MaxDigits?, Direction?, FillWithZero?
struct NumericValue {
	double X, Y;
	int MaxDigit;
	int Direction;
	bool FillWithZero;
	unsigned char RGB[3];
};

// Value format: X, Y, AnchorPointX, AnchorPointY
struct SpriteValue {
	int numOfFrames;
	double X, Y;
	float AnchorPointX, AnchorPointY;
	float FrameTime;
	unsigned char RGB[3];
};

// Value format: NumOfFiles, FileName
struct NoteValue {
	int numOfFiles;
	std::string fileName;
	float FrameTime;
	unsigned char RGB[3];
};

struct RectInfo {
	int X, Y;
	int Width, Height;
};