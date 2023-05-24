#pragma once
#include "framework.h"
#include <string>
#include <filesystem>

enum class AudioType {
	STREAM,
	SAMPLE
};

class Audio {
public:
	Audio(std::string id);
	~Audio();
	AudioType GetType() const;

	bool Create(std::filesystem::path fileName);
	bool Create(uint8_t* buffer, size_t size);

	bool Play(DWORD dwStartPosition = 0, BOOL bLoop = 0);
	bool Pause();
	bool Resume();
	bool Stop();
	bool IsPlaying();
	bool FadeIn();
	bool FadeOut();

	void Update();

	void SetVolume(int vol);
	void SetPan(int pan);
	void SetRate(float rate);
	void SetPitch(bool enabled);

	int GetDuration() const;

	std::string GetName() const;

	bool Release();

protected:
	bool CreateStream();
	AudioType m_type;
	std::string m_id;

	uint8_t* m_pBuffer;
	size_t m_dwSize;

	int volume = 0;
	int pan = 0;
	float rate = 1.0f;
	bool pitch = false;
	bool is_fade_rn = false;
	bool is_fade_out = false;
	
	DWORD fadeState = -1;
	DWORD fadeStartTime = -1;
	DWORD fadeEndTime = -1;

	DWORD m_hStream;
};