#pragma once

#include "../../Engine/Texture2D.hpp"
#include "../../Engine/Scene.hpp"
#include "../../Engine/Text.hpp"
#include "../Engine/Button.hpp"
#include "../Engine/BGMPreview.hpp"

struct MouseState;

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

private:
	void SaveConfiguration();
	void LoadChartImage();

	int index = -1;
	int page = 0;

	bool isWait = false;
	float waitTime = 0;

	float currentSpeed = 2.25;
	float currentRate = 1.0;
	
	float currentAlpha = 100.0;
	float nextAlpha = 100.0;
	
	int startOffset = 0;
	int currentVolume = 100;
	int currentFPSIndex = 0;
	int currentOffset = 0;
	int currentResolutionIndex = 0;
	int currentGuideLineIndex = 0;
	bool LongNoteLighting = false;
	bool LongNoteOnHitPos = false;
	bool convertAutoSound = false;

	bool is_departing = false;
	bool is_quit = false;
	bool is_update_bgm = false;
	bool imgui_modal_quit_confirm = false;

	char lanePos[8];

	std::unique_ptr<Texture2D> m_background;
	std::unique_ptr<Texture2D> m_songBackground;

	std::unique_ptr<BGMPreview> m_bgm;

	std::mutex m_imageLock;

	std::vector<std::string> m_resolutions;
	std::vector<std::string> m_fps;
	std::vector<UDim2> m_songListRect;
	std::vector<Button> m_buttons;
};