#pragma once
#include <filesystem>
#include <directxtk/SpriteFont.h>
#include "UDim2.hpp"
#include "Color3.hpp"
#include "Vector2.hpp"

class Text {
public:
	Text() = default;
	Text(std::filesystem::path textPath);
	~Text();

	void Draw(std::wstring text);
	void Draw(std::wstring text, bool manualDraw);
	void Draw(std::wstring text, RECT* clipRect);
	void Draw(std::wstring text, RECT* cliprect, bool manualDraw);

	UDim2 Size;
	UDim2 Position;
	UDim2 Position2;
	Color3 TintColor;

	Vector2 AnchorPoint;
	Vector2 AbsoluteSize;
	Vector2 AbsolutePosition;

private:
	DirectX::SpriteFont* m_font;
};