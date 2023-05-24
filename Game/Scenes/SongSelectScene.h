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

	float currentSpeed = 2.25;
	float currentRate = 1.0;

	bool is_departing = false;
	bool is_quit = false;
	bool imgui_modal_quit_confirm = false;

	Text* m_text;
	Texture2D* m_songSelect;
	Texture2D* m_songSelectIndex;
	BGMPreview* m_bgm;

	UDim2 m_selectedIndex;
	std::vector<UDim2> m_songListRect;
	std::vector<Button> m_buttons;
};