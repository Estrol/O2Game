#pragma once
#include <string>
#include <unordered_map>
#include <vector>

// if GCC then include experimental/filesystem, else filesytem
#include <filesystem>

// Value Format: X, Y, AnchorPointX?, AnchorPointY?, TintColor?
struct PositionValue {
	int X, Y;
	float AnchorPointX, AnchorPointY;
	unsigned char RGB[3];
};

// Value format: X, Y, MaxDigits?, Direction?, FillWithZero?
struct NumericValue {
	int X, Y;
	int MaxDigit;
	int Direction;
	bool FillWithZero;
};

// Value format: X, Y, AnchorPointX, AnchorPointY
struct SpriteValue {
	int numOfFrames;
	int X, Y;
	float AnchorPointX, AnchorPointY;
	float FrameTime;
};

// Value format: NumOfFiles, FileName
struct NoteValue {
	int numOfFiles;
	std::string fileName;
	float FrameTime;
};

struct RectInfo {
	int X, Y;
	int Width, Height;
};

class SkinConfig {
public:
	SkinConfig() = default;
	SkinConfig(std::string filePath, int keyCount);
	SkinConfig(std::filesystem::path path, int keyCount);
	~SkinConfig();

	std::vector<NumericValue>& GetNumeric(std::string key);
	std::vector<PositionValue>& GetPosition(std::string key);
	std::vector<RectInfo>& GetRect(std::string key);
	NoteValue& GetNote(std::string key);
	SpriteValue& GetSprite(std::string key);

private:
	void Load(std::filesystem::path path, int keyCount);

	std::unordered_map<std::string, std::vector<NumericValue>> m_numericValues;
	std::unordered_map<std::string, std::vector<PositionValue>> m_positionValues;
	std::unordered_map<std::string, SpriteValue> m_spriteValues;
	std::unordered_map<std::string, std::vector<RectInfo>> m_rectValues;
	std::unordered_map<std::string, NoteValue> m_noteValues;
};