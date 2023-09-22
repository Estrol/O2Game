#pragma once
#include <vector>

#include "Scene.h"
#include "Texture/Text.h"
#include "Texture/Texture2D.h"

#include "../Engine/Button.hpp"
#include "../Engine/BGMPreview.hpp"
#include "../Resources/SkinConfig.hpp"

struct MouseState;
struct DB_MusicItem;

class SongSelectScene : public Scene {
public:
	SongSelectScene();
	~SongSelectScene() override;

	void Render(double delta) override;
	void Update(double delta) override;
	void Input(double delta) override;
	void OnMouseDown(const MouseState& state) override;

	bool Attach() override;
	bool Detach() override;

protected:
	void OnGameLoadMusic(double delta);
	void OnGameSelectMusic(double delta);

private:
	void SaveConfiguration();
	void LoadChartImage();

	int scene_index = 0;
	int index = -1;
	int page = 0;

	bool isWait = false;
	float waitTime = 0;

	float currentSpeed = 2.25;
	float currentRate = 1.0;
	
	float currentAlpha = 100.0;
	float nextAlpha = 100.0;

	bool is_departing = false;
	bool is_quit = false;
	bool is_update_bgm = false;
	bool imgui_modal_quit_confirm = false;

	bool bPlay = false;
    bool bExitPopup = false;
    bool bOptionPopup = false;
    bool bSelectNewSong = false;
    bool bOpenSongContext = false;
    bool bOpenEditor = false;
    bool bOpenRearrange = false;
    bool bScaleOutput = true;

	char lanePos[8] = {};

	std::unique_ptr<Texture2D> m_background;
	std::unique_ptr<Texture2D> m_songBackground;

	std::unique_ptr<BGMPreview> m_bgm;

	std::mutex m_imageLock;

	std::vector<std::string> m_resolutions;
	std::vector<std::string> m_fps;
	std::vector<UDim2> m_songListRect;
	std::vector<Button> m_buttons;
	std::vector<DB_MusicItem> m_musicList;

	SkinConfig m_config;
	std::vector<std::filesystem::path> m_tempMusicLists;
	std::filesystem::path file;
	int m_tempMusicIndex;
	float m_lastTime;
};