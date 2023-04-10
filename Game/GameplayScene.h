#pragma once
#include "../Engine/EstEngine.hpp"
#include "ManiaKeys.h"
#include "Engine/O2Texture.hpp"
#include "Engine/DrawableNote.hpp"
#include "Engine/RhythmEngine.hpp"
#include "Engine/O2NumericTexture.hpp"
#include "Engine/FrameTimer.hpp"

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
	std::unordered_map<int, Texture2D*> m_keyLighting;
	std::unordered_map<int, Texture2D*> m_keyButtons;
	std::unordered_map<int, Texture2D*> m_judgement;
	std::unordered_map<int, FrameTimer*> m_hitEffect;
	std::unordered_map<int, FrameTimer*> m_holdEffect;
	std::unordered_map<int, bool> m_keyState;

	Texture2D* m_playBG;
	Texture2D* m_playfieldBG;

	Tile2D* m_jamGauge;

	Sprite2D* m_jamLogo;
	Sprite2D* m_lifeBar;
	Sprite2D* m_lnLogo;
	
	NumericTexture* m_lnComboNum;
	NumericTexture* m_jamNum;
	NumericTexture* m_scoreNum;
	NumericTexture* m_comboNum;
	RhythmEngine* m_game;

	bool m_starting;
	int m_judgeIndex;
	int m_wiggleAdd;
	int m_lnWiggleAdd;

	/* Scoring */
	bool m_drawJam;
	bool m_drawScore;
	bool m_drawJudge;
	bool m_drawCombo;
	bool m_drawLN;

	/* Timer */
	double m_jamTimer;
	double m_scoreTimer;
	double m_judgeTimer;
	double m_comboTimer;
	double m_lnTimer;
	
	/* other stuff */
	double m_judgeSize;

	/* Hit/Hold Effect */
	bool m_drawHold[7];
	bool m_drawHit[7];
};