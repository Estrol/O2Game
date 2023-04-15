#include "GameplayScene.h"
#include "./Engine/NoteImageCacheManager.hpp"
#include <iostream>
#include <unordered_map>
#include "EnvironmentSetup.hpp"
#include <future>
#include "Data/Util/mINI.h"
#include "Resources/SkinConfig.hpp"

#define SAFE_DELETE(x) if (x) { delete x; x = nullptr; }

bool CheckSkinComponent(std::string x) {
	if (!std::filesystem::exists(x)) {
		auto absolutePath = std::filesystem::current_path().string() + x;
		bool result = std::filesystem::exists(absolutePath);
		return result;
	}

	return true;
}

GameplayScene::GameplayScene() : Scene::Scene() {
	m_keyLighting = {};
	m_keyButtons = {};
	m_keyState = {};
	m_game = nullptr;
	m_drawJam = false;
	m_wiggleAdd = 0;
	m_lnWiggleAdd = 0;
}

void GameplayScene::Update(double delta) {
	if (!m_starting) {
		m_starting = true;
		m_game->Start();
	}

	m_game->Update(delta);
}

void GameplayScene::Render(double delta) {
	m_playBG->Draw();

	for (auto& [lane, pressed] : m_keyState) {
		if (pressed) {
			m_keyLighting[lane]->Draw();
			m_keyButtons[lane]->Draw();
		}
	}

	m_game->Render(delta);

	auto scores = m_game->GetScoreManager()->GetScore();
	m_scoreNum->DrawNumber(std::get<0>(scores));

	if (m_drawCombo) {
		if (std::get<7>(scores) > 0) {
			m_comboNum->ManipPosition = UDim2::fromOffset(0, m_wiggleAdd);
			m_comboNum->DrawNumber(std::get<7>(scores));
		}

		m_comboTimer += delta;
		if (m_comboTimer > 2.0) {
			m_drawCombo = false;
		}
	}

	if (m_drawJudge) {
		m_judgement[m_judgeIndex]->Size = UDim2::fromScale(m_judgeSize, m_judgeSize);
		m_judgement[m_judgeIndex]->AnchorPoint = { 0.5, 0.5 };
		m_judgement[m_judgeIndex]->Draw();

		m_judgeSize = std::clamp(m_judgeSize + (delta * 3), 0.5, 1.0);
		if ((m_judgeTimer += delta) > 1) {
			m_drawJudge = false;
		}
	}

	if (m_drawJam) {
		if (std::get<5>(scores) > 0) {
			m_jamNum->DrawNumber(std::get<5>(scores));
			m_jamLogo->Draw(delta);
		}

		if ((m_jamTimer += delta) > 1) {
			m_drawJam = false;
		}
	}

	if (m_drawLN) {
		if (std::get<9>(scores) > 0) {
			m_lnComboNum->ManipPosition = UDim2::fromOffset(0, m_lnWiggleAdd);
			m_lnComboNum->DrawNumber(std::get<9>(scores));
			m_lnLogo->Draw(delta);
		}

		if ((m_lnTimer += delta) > 1) {
			m_drawLN = false;
		}
	}

	float gaugeVal = (float)m_game->GetScoreManager()->GetJamGauge() / kMaxJamGauge;
	m_jamGauge->Size = UDim2::fromScale(gaugeVal, 1);
	m_jamGauge->Draw();

	{
		auto curLifeTex = m_lifeBar->GetTexture();
		curLifeTex->CalculateSize();

		RECT rc = {};
		rc.left = curLifeTex->AbsolutePosition.X;
		rc.top = curLifeTex->AbsolutePosition.Y;
		rc.right = rc.left + curLifeTex->AbsoluteSize.X;
		rc.bottom = rc.top + curLifeTex->AbsoluteSize.Y;

		float alpha = (float)(kMaxLife - m_game->GetScoreManager()->GetLife()) / kMaxLife;
		LONG topCur = (1.0 - alpha) * rc.top + alpha * rc.bottom;
		rc.top = topCur;
		
		m_lifeBar->Draw(delta, &rc);
	}

	for (int i = 0; i < 7; i++) {
		if (m_drawHold[i]) {
			m_holdEffect[i]->Draw(delta);
		}

		m_hitEffect[i]->Draw(delta);
	}

	if (m_game->GetState() == GameState::PosGame) {
		SceneManager::GetInstance()->StopGame();
	}

	m_wiggleAdd = std::clamp(m_wiggleAdd + delta * -0.5, 0.0, 10.0);
	m_lnWiggleAdd = std::clamp(m_lnWiggleAdd + delta * -0.5, 0.0, 10.0);
}

void GameplayScene::Input(double delta) {
	
}

void GameplayScene::OnKeyDown(const KeyState& state) {
	m_game->OnKeyDown(state);
}

void GameplayScene::OnKeyUp(const KeyState& state) {
	m_game->OnKeyUp(state);
}

bool GameplayScene::Attach() {
	SkinConfig conf("/Skins/Default/Playing/Playing.ini");

	for (int i = 0; i < 7; i++) {
		m_keyState[i] = false;
		m_drawHold[i] = false;
		m_drawHit[i] = false;
	}

	m_playBG = new Texture2D("/Skins/Default/Playing/PlayingBG.png");
	for (int i = 0; i < 7; i++) {
		m_keyLighting[i] = new Texture2D(("/Skins/Default/Playing/KeyLighting" + std::to_string(i) + ".png"));
		m_keyButtons[i] = new Texture2D(("/Skins/Default/Playing/KeyButton" + std::to_string(i) + ".png"));

		auto conKeyLight = conf.GetPosition("KeyLighting" + std::to_string(i));
		auto conKeyButton = conf.GetPosition("KeyButton" + std::to_string(i));

		m_keyLighting[i]->Position = UDim2::fromOffset(conKeyLight.X, conKeyLight.Y);
		m_keyButtons[i]->Position = UDim2::fromOffset(conKeyButton.X, conKeyButton.Y);
	}

	std::vector<std::string> numComboPaths = {};
	for (int i = 0; i < 10; i++) {
		numComboPaths.push_back("/Skins/Default/Playing/ComboNum" + std::to_string(i) + ".png");

		if (!CheckSkinComponent(numComboPaths.back())) {
			MessageBoxA(NULL, "Failed to load Integer Images 0-9, please check your skin folder.", "Error", MB_OK);
			return false;
		}
	}

	m_comboNum = new NumericTexture(numComboPaths);
	auto numPos = conf.GetNumeric("Combo");

	m_comboNum->Position = UDim2::fromOffset(numPos.X, numPos.Y);
	m_comboNum->NumberPosition = IntToPos(numPos.Direction);
	m_comboNum->MaxDigits = numPos.MaxDigit;
	m_comboNum->FillWithZeros = numPos.FillWithZero;

	std::vector<std::string> numJamPaths = {};
	for (int i = 0; i < 10; i++) {
		numJamPaths.push_back("/Skins/Default/Playing/JamNum" + std::to_string(i) + ".png");

		if (!CheckSkinComponent(numJamPaths.back())) {
			MessageBoxA(NULL, "Failed to load Integer Images 0-9, please check your skin folder.", "Error", MB_OK);
			return false;
		}
	}

	m_jamNum = new NumericTexture(numJamPaths);
	numPos = conf.GetNumeric("Jam");

	m_jamNum->Position = UDim2::fromOffset(numPos.X, numPos.Y);
	m_jamNum->NumberPosition = IntToPos(numPos.Direction);
	m_jamNum->MaxDigits = numPos.MaxDigit;
	m_jamNum->FillWithZeros = numPos.FillWithZero;

	std::vector<std::string> numScorePaths = {};
	for (int i = 0; i < 10; i++) {
		numScorePaths.push_back("/Skins/Default/Playing/ScoreNum" + std::to_string(i) + ".png");

		if (!CheckSkinComponent(numScorePaths.back())) {
			MessageBoxA(NULL, "Failed to load Integer Images 0-9, please check your skin folder.", "Error", MB_OK);
			return false;
		}
	}

	m_scoreNum = new NumericTexture(numScorePaths);
	numPos = conf.GetNumeric("Score");
	
	m_scoreNum->Position = UDim2::fromOffset(numPos.X, numPos.Y);
	m_scoreNum->NumberPosition = IntToPos(numPos.Direction);
	m_scoreNum->MaxDigits = numPos.MaxDigit;
	m_scoreNum->FillWithZeros = numPos.FillWithZero;
	m_scoreNum->AlphaBlend = true;

	std::vector<std::string> judgeFileName = { "Miss", "Bad", "Good", "Cool" };
	for (int i = 0; i < 4; i++) {
		m_judgement[i] = new Texture2D("/Skins/Default/Playing/Judge" + judgeFileName[i] + ".png");

		if (!CheckSkinComponent(numScorePaths.back())) {
			MessageBoxA(NULL, "Failed to load Judge image!", "Error", MB_OK);
			return false;
		}

		auto judgePos = conf.GetPosition("Judge" + judgeFileName[i]);
		m_judgement[i]->Position = UDim2::fromOffset(judgePos.X, judgePos.Y);
		m_judgement[i]->AlphaBlend = true;
	}

	m_jamGauge = new Tile2D("/Skins/Default/Playing/JamGauge.png");
	auto gaugePos = conf.GetPosition("JamGauge");
	m_jamGauge->Position = UDim2::fromOffset(gaugePos.X, gaugePos.Y);
	m_jamGauge->AnchorPoint = { gaugePos.AnchorPointX, gaugePos.AnchorPointY };

	std::vector<std::string> jamLogoFileName = {};
	for (int i = 0; i < 8; i++) {
		jamLogoFileName.push_back("/Skins/Default/Playing/JamLogo" + std::to_string(i) + ".png");

		if (!CheckSkinComponent(jamLogoFileName.back())) {
			MessageBoxA(NULL, "Failed to load Jam Logo image!", "Error", MB_OK);
			return false;
		}
	}

	m_jamLogo = new Sprite2D(jamLogoFileName, 0.25);
	auto jamLogoPos = conf.GetSprite("JamLogo");
	m_jamLogo->Position = UDim2::fromOffset(jamLogoPos.X, jamLogoPos.Y);
	m_jamLogo->AnchorPoint = { jamLogoPos.AnchorPointX, jamLogoPos.AnchorPointY };

	std::vector<std::string> lifeBarFileName = {};
	for (int i = 0; i < 9; i++) {
		lifeBarFileName.push_back("/Skins/Default/Playing/LifeBar" + std::to_string(i) + ".png");

		if (!CheckSkinComponent(lifeBarFileName.back())) {
			MessageBoxA(NULL, "Failed to load Life Bar image!", "Error", MB_OK);
			return false;
		}
	}

	m_lifeBar = new Sprite2D(lifeBarFileName, 0.15);
	auto lifeBarPos = conf.GetSprite("LifeBar");
	m_lifeBar->Position = UDim2::fromOffset(lifeBarPos.X, lifeBarPos.Y);
	m_lifeBar->AnchorPoint = { lifeBarPos.AnchorPointX, lifeBarPos.AnchorPointY };

	std::vector<std::string> lnLogoFileName = {};
	for (int i = 0; i < 2; i++) {
		lnLogoFileName.push_back("/Skins/Default/Playing/LongNoteLogo" + std::to_string(i) + ".png");

		if (!CheckSkinComponent(lnLogoFileName.back())) {
			MessageBoxA(NULL, "Failed to load Long Note Logo image!", "Error", MB_OK);
			return false;
		}
	}

	m_lnLogo = new Sprite2D(lnLogoFileName, 0.25);
	auto lnLogoPos = conf.GetSprite("LongNoteLogo");
	m_lnLogo->Position = UDim2::fromOffset(lnLogoPos.X, lnLogoPos.Y);
	m_lnLogo->AnchorPoint = { lnLogoPos.AnchorPointX, lnLogoPos.AnchorPointY };

	std::vector<std::string> lnComboFileName = {};
	for (int i = 0; i < 10; i++) {
		lnComboFileName.push_back("/Skins/Default/Playing/LongNoteNum" + std::to_string(i) + ".png");

		if (!CheckSkinComponent(lnComboFileName.back())) {
			MessageBoxA(NULL, "Failed to load Long Note Combo image!", "Error", MB_OK);
			return false;
		}
	}

	m_lnComboNum = new NumericTexture(lnComboFileName);
	auto lnComboPos = conf.GetNumeric("LongNoteCombo");
	m_lnComboNum->Position = UDim2::fromOffset(lnComboPos.X, lnComboPos.Y);
	m_lnComboNum->NumberPosition = IntToPos(lnComboPos.Direction);
	m_lnComboNum->MaxDigits = lnComboPos.MaxDigit;
	m_lnComboNum->FillWithZeros = lnComboPos.FillWithZero;

	Chart* chart = (Chart*)EnvironmentSetup::GetObj("SONG");
	if (chart == nullptr) {
		MessageBoxA(NULL, "Fatal error: Chart is null", "Error", MB_OK);
		return false;
	}

	m_game = new RhythmEngine();

	if (!m_game) {
		MessageBoxA(NULL, "Failed to load game!", "EstEngine Error", MB_ICONERROR);
		return false;
	}
	
	m_game->ListenKeyEvent([&](GameTrackEvent e) {
		if (e.IsKeyEvent) {
			m_keyState[e.Lane] = e.State;
		}
		else {
			if (e.IsHitEvent) {
				if (e.IsHitLongEvent) {
					m_holdEffect[e.Lane]->ResetIndex();
					m_drawHold[e.Lane] = e.State;
					m_drawHit[e.Lane] = false;

					if (!e.State) {
						m_hitEffect[e.Lane]->ResetIndex();
						m_drawHit[e.Lane] = true;
					}
				}
				else {
					m_hitEffect[e.Lane]->ResetIndex();
					m_drawHold[e.Lane] = false;
					m_drawHit[e.Lane] = true;
				}
			}
		}
	});

	m_game->Load(chart);

	std::vector<std::string> HitEffect = {};
	for (int i = 0; i < 9; i++) {
		std::string path = "/Skins/Default/Playing/HitEffect" + std::to_string(i) + ".png";

		if (!CheckSkinComponent(path)) {
			break;
		}

		HitEffect.push_back(path);
	}

	std::vector<std::string> holdEffect = {};
	for (int i = 0; i < 9; i++) {
		std::string path = "/Skins/Default/Playing/HoldEffect" + std::to_string(i) + ".png";

		if (!CheckSkinComponent(path)) {
			break;
		}

		holdEffect.push_back(path);
	}

	auto lanePos = m_game->GetLanePos();
	auto laneSize = m_game->GetLaneSizes();
	for (int i = 0; i < 7; i++) {
		m_hitEffect[i] = new FrameTimer(HitEffect);
		m_holdEffect[i] = new FrameTimer(holdEffect);

		m_hitEffect[i]->SetFPS(30.0);
		m_holdEffect[i]->SetFPS(30.0);
		m_holdEffect[i]->Repeat = true;

		int pos = lanePos[i] + (laneSize[i] / 2);
		m_hitEffect[i]->Position = UDim2::fromOffset(pos, 480);
		m_holdEffect[i]->Position = UDim2::fromOffset(pos, 480);
		m_hitEffect[i]->AnchorPoint = { .5, .45 };
		m_holdEffect[i]->AnchorPoint = { .5, .45 };
	}

	m_game->GetScoreManager()->ListenHit([&](NoteHitInfo info) {
		m_scoreTimer = 0;
		m_judgeTimer = 0;
		m_comboTimer = 0;
		m_judgeSize = 0.5;

		m_drawCombo = true;
		m_drawJudge = true;
		m_wiggleAdd = 10;

		m_judgeIndex = (int)info.Result;
	});

	m_game->GetScoreManager()->ListenJam([&](int combo) {
		m_drawJam = true;
		m_jamTimer = 0;
	});

	m_game->GetScoreManager()->ListenLongNote([&] {
		m_lnTimer = 0;
		m_lnWiggleAdd = 10;
		m_drawLN = true;
	});

	m_starting = false;
	return true;
}

bool GameplayScene::Detach() {
	SAFE_DELETE(m_playBG);
	for (int i = 0; i < 7; i++) {
		SAFE_DELETE(m_keyLighting[i]);
		SAFE_DELETE(m_keyButtons[i]);
	}

	SAFE_DELETE(m_comboNum);
	SAFE_DELETE(m_jamNum);
	SAFE_DELETE(m_scoreNum);
	
	for (int i = 0; i < 4; i++) {
		SAFE_DELETE(m_judgement[i]);
	}

	return true;
}
