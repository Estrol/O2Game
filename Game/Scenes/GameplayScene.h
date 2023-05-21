#pragma once
#include "../../Engine/EstEngine.hpp"
#include "../../Engine/Text.hpp"

#include "../Engine/DrawableNote.hpp"
#include "../Engine/RhythmEngine.hpp"
#include "../Engine/FrameTimer.hpp"

#include "../ManiaKeys.h"
#include "../Engine/Button.hpp"

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
	void OnMouseDown(const MouseState& state) override;

	bool Attach() override;
	bool Detach() override;
	
private:
	void* CreateScreenshotWin32();

	std::unordered_map<int, Texture2D*> m_keyLighting;
	std::unordered_map<int, Texture2D*> m_keyButtons;
	std::unordered_map<int, Texture2D*> m_judgement;
	std::unordered_map<int, Texture2D*> m_pills;
	std::unordered_map<int, FrameTimer*> m_hitEffect;
	std::unordered_map<int, FrameTimer*> m_holdEffect;
	std::unordered_map<int, bool> m_keyState;
	std::unordered_map<int, UDim2> m_statsPos;

	Button* m_exitButtonFunc;
	Texture2D* m_exitBtn;
	Texture2D* m_playBG;

	Tile2D* m_jamGauge;
	Tile2D* m_waveGage;

	Sprite2D* m_jamLogo;
	Sprite2D* m_lifeBar;
	Sprite2D* m_lnLogo;
	Sprite2D* m_comboLogo;
	
	NumericTexture* m_statsNum;
	NumericTexture* m_lnComboNum;
	NumericTexture* m_jamNum;
	NumericTexture* m_scoreNum;
	NumericTexture* m_comboNum;

	Text* m_text;

	RhythmEngine* m_game;

	bool m_resourceFucked;
	bool m_starting;
	bool m_ended;
	int m_judgeIndex;

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

	/* Fixed Animation*/
	double m_wiggleTime;
	double m_wiggleOffset;

	/* button */
	bool m_drawExitButton;
	bool m_doExit;
};