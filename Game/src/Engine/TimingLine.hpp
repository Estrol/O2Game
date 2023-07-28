#pragma once
class ResizableImage;
class RhythmEngine;

struct TimingLineDesc {
	double StartTime;
	double Offset;
	int ImagePos;
	int ImageSize;

	RhythmEngine* Engine;
};

class TimingLine {
public:
	TimingLine();
	~TimingLine();

	void Load(TimingLineDesc* timing);

	void Update(double delta);
	void Render(double delta);

	double GetOffset() const;
	double GetStartTime() const;
	double GetTrackPosition() const;

	void Release();
private:
	double m_startTime, m_offset, m_currentTrackPosition;
	int m_imagePos, m_imageSize;

	ResizableImage* m_line;
	RhythmEngine* m_engine;
};