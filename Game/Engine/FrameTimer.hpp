#pragma once
#include <vector>
#include <string>
#include "../../Engine/UDim2.hpp"
#include "../../Engine/Vector2.hpp"

class Texture2D;

class FrameTimer {
public:
	FrameTimer(std::vector<Texture2D*> frames);
	FrameTimer(std::vector<std::string> frames);
	~FrameTimer();

	bool Repeat;
	UDim2 Position;
	Vector2 AnchorPoint;

	void Draw(double delta);
	void SetFPS(float fps);
	void ResetIndex();

private:
	std::vector<Texture2D*> m_frames;
	int m_currentFrame;
	double m_frameTime;
	double m_currentTime;
};