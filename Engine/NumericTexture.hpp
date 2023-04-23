#pragma once
#include <d3d11.h>
#include <vector>
#include <string>
#include <map>
#include "Texture2D.hpp"

enum class NumericPosition {
	LEFT,
	MID,
	RIGHT
};

NumericPosition IntToPos(int i);

class NumericTexture {
public:
	NumericTexture() = default;
	NumericTexture(std::vector<ID3D11ShaderResourceView*>& numericsTexture);
	NumericTexture(std::vector<std::string> numericsFiles);
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
		
protected:
	std::vector<Texture2D*> m_numericsTexture;
	std::map<int, RECT> m_numbericsWidth;
};