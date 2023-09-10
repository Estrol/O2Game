#pragma once
#include <string>
#include <filesystem>
#include <mutex>
#include <chrono>

#include "../Rendering/WindowsTypes.h"

enum class AudioType {
	STREAM,
	SAMPLE
};

class Audio {
#if _DEBUG
	const char SIGNATURE[25] = "Audio";
#endif

public:
	Audio(std::string id);
	~Audio();
	AudioType GetType() const;

	bool Create(std::filesystem::path fileName);
	bool Create(uint8_t* buffer, size_t size);

	bool Play(uint32_t dwStartPosition = 0, bool bLoop = false);
	bool Pause();
	bool Resume();
	bool Stop();
	bool IsPlaying();
	bool IsFadeOut();

	bool FadeIn();
	bool FadeOut();

	void Update(double dt);

	void SetVolume(int vol);
	void SetPan(int pan);
	void SetRate(float rate);
	void SetPitch(bool enabled);

	int GetDuration() const;

	std::string GetName() const;

	bool Release();

protected:
	bool CreateStream();
	void FadeStream(int volume, bool state);

	AudioType m_type;
	std::string m_id;

	std::vector<uint8_t> m_vBuffer;
	size_t m_dwSize;

	int volume = 0;
	int pan = 0;
	float rate = 1.0f;
	bool pitch = false;
	bool is_fade_rn = false;
	
	uint32_t fadeState = -1;
	uint32_t fadeStartTime = -1;
	uint32_t fadeEndTime = -1;

	uint32_t m_hStream;
	float m_slideTargetVolume = 0;
	bool m_is_doing_fading = false;
	bool m_state = false;

	float m_minSlideVolume = 0;
	float m_maxSlideVolume = 0;

	std::chrono::steady_clock::time_point m_lastTime;
};