#pragma once
#include <vector>
#include <string>
#include <map>
#include "Texture2D.h"
#include "../Rendering/WindowsTypes.h"

enum class NumericPosition {
	LEFT,
	MID,
	RIGHT
};

NumericPosition IntToPos(int i);

class NumericTexture {
public:
	NumericTexture() = default;
	NumericTexture(std::vector<std::string> numericsFiles);
	NumericTexture(std::vector<std::filesystem::path> numericsPath);
	~NumericTexture();

	UDim2 Position;
	UDim2 Position2;
	Vector2 AnchorPoint;
	NumericPosition NumberPosition = NumericPosition::MID;
	bool FillWithZeros = false;
	bool AlphaBlend = false;
	int MaxDigits = 0;
	int Offset = 0;

	void DrawNumber(int number);
	void SetValue(int value);
		
protected:
	std::vector<Texture2D*> m_numericsTexture;
	std::map<int, Rect> m_numbericsWidth;
};