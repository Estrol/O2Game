#pragma once
#include "../Engine/EstEngine.hpp"
#include "ManiaKeys.h"
#include "Engine/O2Texture.hpp"
#include "Engine/DrawableNote.hpp"
#include "Engine/RhythmEngine.hpp"

struct ManiaKeyState {
	Keys key;
	bool isPressed;
};

class GameplayScene : public Scene {
public:
	GameplayScene();

	void Update(double delta) override;
	void Render(double delta) override;
	void Input(double delta) override;

	void OnKeyDown(const KeyState& state) override;
	void OnKeyUp(const KeyState& state) override;

	bool Attach() override;
	bool Detach() override;
	
private:
	std::unordered_map<int, O2Texture*> m_keyLighting;
	std::unordered_map<int, O2Texture*> m_keyButtons;
	std::unordered_map<int, bool> m_keyState;

	O2Texture* m_playfieldBG;
	RhythmEngine* m_game;
};