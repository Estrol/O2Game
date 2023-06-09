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

	int index = -1;
	int page = 0;

	bool isWait = false;
	float waitTime = 0;

	float currentSpeed = 2.25;
	float currentRate = 1.0;
	int currentVolume = 100;
	int currentFPSIndex = 0;
	int currentOffset = 0;
	int currentResolutionIndex = 0;
	bool convertAutoSound = 0;

	bool is_departing = false;
	bool is_quit = false;
	bool is_update_bgm = false;
	bool imgui_modal_quit_confirm = false;

	std::unique_ptr<Texture2D> m_background;
	std::unique_ptr<BGMPreview> m_bgm;

	UDim2 m_selectedIndex;
	std::vector<std::string> m_resolutions;
	std::vector<std::string> m_fps;
	std::vector<UDim2> m_songListRect;
	std::vector<Button> m_buttons;
};