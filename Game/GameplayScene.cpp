#include "GameplayScene.h"
#include "./Engine/NoteImageCacheManager.hpp"
#include <iostream>
#include <unordered_map>
#include "EnvironmentSetup.hpp"
#include <future>
#include "Data/Util/mINI.h"
#include "Resources/SkinConfig.hpp"
#include "Resources/Configuration.hpp"

#define SAFE_DELETE(x) if (x) { delete x; x = nullptr; }

bool CheckSkinComponent(std::filesystem::path x) {
	return std::filesystem::exists(x);
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
	
	// Draw stats
	{
		m_statsNum->Position = m_statsPos[0];
		m_statsNum->DrawNumber(std::get<1>(scores));
		m_statsNum->Position = m_statsPos[1];
		m_statsNum->DrawNumber(std::get<2>(scores));
		m_statsNum->Position = m_statsPos[2];
		m_statsNum->DrawNumber(std::get<3>(scores));
		m_statsNum->Position = m_statsPos[3];
		m_statsNum->DrawNumber(std::get<4>(scores));
		m_statsNum->Position = m_statsPos[4];
		m_statsNum->DrawNumber(std::get<8>(scores));
	}

	if (m_drawCombo) {
		if (std::get<7>(scores) > 0) {
			m_comboLogo->Position2 = UDim2::fromOffset(0, m_wiggleAdd);
			m_comboLogo->Draw(delta);

			m_comboNum->Position2 = UDim2::fromOffset(0, m_wiggleAdd);
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
			m_lnComboNum->Position2 = UDim2::fromOffset(0, m_lnWiggleAdd);
			m_lnComboNum->DrawNumber(std::get<9>(scores));
			m_lnLogo->Position2 = UDim2::fromOffset(0, m_lnWiggleAdd);
			m_lnLogo->Draw(delta);
		}

		if ((m_lnTimer += delta) > 1) {
			m_drawLN = false;
		}
	}

	float gaugeVal = (float)m_game->GetScoreManager()->GetJamGauge() / kMaxJamGauge;
	m_jamGauge->Size = UDim2::fromScale(gaugeVal, 1);
	m_jamGauge->Draw();

	float currentProgress = m_game->GetAudioPosition() / m_game->GetAudioLength();
	m_waveGage->Size = UDim2::fromScale(currentProgress, 1);
	m_waveGage->Draw();

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

	int numOfPills = m_game->GetScoreManager()->GetPills();
	for (int i = 0; i < numOfPills; i++) {
		m_pills[i]->Draw();
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
	auto SkinName = Configuration::Load("Game", "Skin");
	int LaneOffset = 3;
	int HitPos = 480;

	try {
		LaneOffset = std::atoi(Configuration::Skin_LoadValue(SkinName, "Game", "LaneOffset").c_str());
		HitPos = std::atoi(Configuration::Skin_LoadValue(SkinName, "Game", "HitPos").c_str());
	}
	catch (std::invalid_argument) {
		MessageBoxA(NULL, "Invalid parameter on Skin::Game::LaneOffset or Skin::Game::HitPos", "EstEngine Error", MB_ICONERROR);
	}

	auto skinPath = Configuration::Skin_GetPath(SkinName);
	auto playingPath = skinPath / "Playing";
	SkinConfig conf(playingPath / "Playing.ini", 7);

	for (int i = 0; i < 7; i++) {
		m_keyState[i] = false;
		m_drawHold[i] = false;
		m_drawHit[i] = false;
	}

	m_playBG = new Texture2D(playingPath / "PlayingBG.png");
	for (int i = 0; i < 7; i++) {
		m_keyLighting[i] = new Texture2D(playingPath / ("KeyLighting" + std::to_string(i) + ".png"));
		m_keyButtons[i] = new Texture2D(playingPath / ("KeyButton" + std::to_string(i) + ".png"));

		auto conKeyLight = conf.GetPosition("KeyLighting" + std::to_string(i));
		auto conKeyButton = conf.GetPosition("KeyButton" + std::to_string(i));

		m_keyLighting[i]->Position = UDim2::fromOffset(conKeyLight.X, conKeyLight.Y);
		m_keyButtons[i]->Position = UDim2::fromOffset(conKeyButton.X, conKeyButton.Y);
	}

	std::vector<std::filesystem::path> numComboPaths = {};
	for (int i = 0; i < 10; i++) {
		numComboPaths.push_back(playingPath / ("ComboNum" + std::to_string(i) + ".png"));

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
	m_comboNum->AlphaBlend = true;

	std::vector<std::filesystem::path> numJamPaths = {};
	for (int i = 0; i < 10; i++) {
		numJamPaths.push_back(playingPath / ("JamNum" + std::to_string(i) + ".png"));

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

	std::vector<std::filesystem::path> numScorePaths = {};
	for (int i = 0; i < 10; i++) {
		numScorePaths.push_back(playingPath / ("ScoreNum" + std::to_string(i) + ".png"));

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

	std::vector<std::string> judgeFileName = { "Miss", "Bad", "Good", "Cool" };
	for (int i = 0; i < 4; i++) {
		m_judgement[i] = new Texture2D(playingPath / ("Judge" + judgeFileName[i] + ".png"));

		if (!CheckSkinComponent(numScorePaths.back())) {
			MessageBoxA(NULL, "Failed to load Judge image!", "Error", MB_OK);
			return false;
		}

		auto judgePos = conf.GetPosition("Judge" + judgeFileName[i]);
		m_judgement[i]->Position = UDim2::fromOffset(judgePos.X, judgePos.Y);
		m_judgement[i]->AlphaBlend = true;
	}

	m_jamGauge = new Tile2D(playingPath / "JamGauge.png");
	auto gaugePos = conf.GetPosition("JamGauge");
	m_jamGauge->Position = UDim2::fromOffset(gaugePos.X, gaugePos.Y);
	m_jamGauge->AnchorPoint = { gaugePos.AnchorPointX, gaugePos.AnchorPointY };
	
	auto jamLogoPos = conf.GetSprite("JamLogo");
	std::vector<std::filesystem::path> jamLogoFileName = {};
	for (int i = 0; i < jamLogoPos.numOfFrames; i++) {
		jamLogoFileName.push_back(playingPath / ("JamLogo" + std::to_string(i) + ".png"));

		if (!CheckSkinComponent(jamLogoFileName.back())) {
			MessageBoxA(NULL, "Failed to load Jam Logo image!", "Error", MB_OK);
			return false;
		}
	}

	m_jamLogo = new Sprite2D(jamLogoFileName, 0.25);
	m_jamLogo->Position = UDim2::fromOffset(jamLogoPos.X, jamLogoPos.Y);
	m_jamLogo->AnchorPoint = { jamLogoPos.AnchorPointX, jamLogoPos.AnchorPointY };

	auto lifeBarPos = conf.GetSprite("LifeBar");
	std::vector<std::filesystem::path> lifeBarFileName = {};
	for (int i = 0; i < lifeBarPos.numOfFrames; i++) {
		lifeBarFileName.push_back(playingPath / ("LifeBar" + std::to_string(i) + ".png"));

		if (!CheckSkinComponent(lifeBarFileName.back())) {
			MessageBoxA(NULL, "Failed to load Life Bar image!", "Error", MB_OK);
			return false;
		}
	}

	m_lifeBar = new Sprite2D(lifeBarFileName, 0.15);
	m_lifeBar->Position = UDim2::fromOffset(lifeBarPos.X, lifeBarPos.Y);
	m_lifeBar->AnchorPoint = { lifeBarPos.AnchorPointX, lifeBarPos.AnchorPointY };

	auto lnLogoPos = conf.GetSprite("LongNoteLogo");
	std::vector<std::filesystem::path> lnLogoFileName = {};
	for (int i = 0; i < lnLogoPos.numOfFrames; i++) {
		lnLogoFileName.push_back(playingPath / ("LongNoteLogo" + std::to_string(i) + ".png"));

		if (!CheckSkinComponent(lnLogoFileName.back())) {
			MessageBoxA(NULL, "Failed to load Long Note Logo image!", "Error", MB_OK);
			return false;
		}
	}

	m_lnLogo = new Sprite2D(lnLogoFileName, 0.25);
	m_lnLogo->Position = UDim2::fromOffset(lnLogoPos.X, lnLogoPos.Y);
	m_lnLogo->AnchorPoint = { lnLogoPos.AnchorPointX, lnLogoPos.AnchorPointY };
	m_lnLogo->AlphaBlend = true;

	std::vector<std::filesystem::path> lnComboFileName = {};
	for (int i = 0; i < 10; i++) {
		lnComboFileName.push_back(playingPath / ("LongNoteNum" + std::to_string(i) + ".png"));

		if (!CheckSkinComponent(lnComboFileName.back())) {
			MessageBoxA(NULL, "Failed to load Long Note Combo image!", "Error", MB_OK);
			return false;
		}
	}

	std::vector<std::filesystem::path> statsNumFileName = {};
	for (int i = 0; i < 10; i++) {
		statsNumFileName.push_back(playingPath / ("StatsNum" + std::to_string(i) + ".png"));

		if (!CheckSkinComponent(statsNumFileName.back())) {
			MessageBoxA(NULL, "Failed to load Stats Number image!", "Error", MB_OK);
			return false;
		}
	}

	m_statsNum = new NumericTexture(statsNumFileName);
	auto statsNumPos = conf.GetNumeric("Stats0");
	auto statsNumPos1 = conf.GetNumeric("Stats1");
	auto statsNumPos2 = conf.GetNumeric("Stats2");
	auto statsNumPos3 = conf.GetNumeric("Stats3");
	auto statsNumPos4 = conf.GetNumeric("Stats4");

	m_statsNum->NumberPosition = IntToPos(statsNumPos.Direction);
	m_statsNum->MaxDigits = statsNumPos.MaxDigit;
	m_statsNum->FillWithZeros = statsNumPos.FillWithZero;
	m_statsNum->NumberPosition = IntToPos(statsNumPos.Direction);
	m_statsNum->AnchorPoint = { 0, .5 };

	m_statsPos[0] = UDim2::fromOffset(statsNumPos.X, statsNumPos.Y); // COOL
	m_statsPos[1] = UDim2::fromOffset(statsNumPos1.X, statsNumPos1.Y); // GOOD
	m_statsPos[2] = UDim2::fromOffset(statsNumPos2.X, statsNumPos2.Y); // BAD
	m_statsPos[3] = UDim2::fromOffset(statsNumPos3.X, statsNumPos3.Y); // MISS
	m_statsPos[4] = UDim2::fromOffset(statsNumPos4.X, statsNumPos4.Y); // MAXCOMBO

	m_lnComboNum = new NumericTexture(lnComboFileName);
	auto lnComboPos = conf.GetNumeric("LongNoteCombo");
	m_lnComboNum->Position = UDim2::fromOffset(lnComboPos.X, lnComboPos.Y);
	m_lnComboNum->NumberPosition = IntToPos(lnComboPos.Direction);
	m_lnComboNum->MaxDigits = lnComboPos.MaxDigit;
	m_lnComboNum->FillWithZeros = lnComboPos.FillWithZero;
	m_lnComboNum->AlphaBlend = true;

	auto comboLogoPos = conf.GetSprite("ComboLogo");
	std::vector<std::filesystem::path> comboFileName = {};
	for (int i = 0; i < comboLogoPos.numOfFrames; i++) {
		auto file = playingPath / ("ComboLogo" + std::to_string(i) + ".png");

		if (!CheckSkinComponent(file)) {
			break;
		}

		comboFileName.push_back(file);
	}

	m_comboLogo = new Sprite2D(comboFileName, 0.25);
	m_comboLogo->Position = UDim2::fromOffset(comboLogoPos.X, comboLogoPos.Y);
	m_comboLogo->AnchorPoint = { comboLogoPos.AnchorPointX, comboLogoPos.AnchorPointY };
	m_comboLogo->AlphaBlend = true;

	m_waveGage = new Tile2D(playingPath / "WaveGage.png");
	auto waveGagePos = conf.GetPosition("WaveGage");
	m_waveGage->Position = UDim2::fromOffset(waveGagePos.X, waveGagePos.Y);
	m_waveGage->AnchorPoint = { waveGagePos.AnchorPointX, waveGagePos.AnchorPointY };

	for (int i = 0; i < 5; i++) {
		auto file = playingPath / ("Pill" + std::to_string(i) + ".png");

		auto pos = conf.GetPosition("Pill" + std::to_string(i));
		m_pills[i] = new Texture2D(file);
		m_pills[i]->Position = UDim2::fromOffset(pos.X, pos.Y);
		m_pills[i]->AnchorPoint = { pos.AnchorPointX, pos.AnchorPointY };
	}

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

	m_game->SetHitPosition(HitPos);
	m_game->SetLaneOffset(LaneOffset);
	
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

	Keys keys[7] = {};
	for (int i = 0; i < 7; i++) {
		auto value = Configuration::Load("KeyMapping", "Lane" + std::to_string(i + 1));
		auto key = static_cast<Keys>(SDL_GetScancodeFromName(value.c_str()));

		if (key != Keys::INVALID_KEY) {
			keys[i] = key;
		}
		else {
			MessageBoxA(NULL, ("Unknown key: " + value).c_str(), "EstGame Error", MB_ICONERROR);
			return false;
		}
	}

	m_game->SetKeys(keys);

	std::vector<std::filesystem::path> HitEffect = {};
	for (int i = 0; i < 9; i++) {
		auto path = playingPath / ("HitEffect" + std::to_string(i) + ".png");

		if (!CheckSkinComponent(path)) {
			break;
		}

		HitEffect.push_back(path);
	}

	std::vector<std::filesystem::path> holdEffect = {};
	for (int i = 0; i < 9; i++) {
		auto path = playingPath / ("HoldEffect" + std::to_string(i) + ".png");

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
		m_hitEffect[i]->LastIndex();
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

		m_comboLogo->Reset();
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
	for (int i = 0; i < 7; i++) {
		SAFE_DELETE(m_keyLighting[i]);
		SAFE_DELETE(m_keyButtons[i]);
		SAFE_DELETE(m_hitEffect[i]);
		SAFE_DELETE(m_holdEffect[i]);
	}

	for (int i = 0; i < 5; i++) {
		SAFE_DELETE(m_pills[i]);
	}

	for (int i = 0; i < 4; i++) {
		SAFE_DELETE(m_judgement[i]);
	}

	SAFE_DELETE(m_playBG);

	SAFE_DELETE(m_jamGauge);
	SAFE_DELETE(m_waveGage);
	SAFE_DELETE(m_jamLogo);
	SAFE_DELETE(m_lifeBar);
	SAFE_DELETE(m_lnLogo);
	SAFE_DELETE(m_comboLogo);

	SAFE_DELETE(m_lnComboNum);
	SAFE_DELETE(m_jamNum);
	SAFE_DELETE(m_scoreNum);
	SAFE_DELETE(m_comboNum);

	SAFE_DELETE(m_game);

	Chart* chart = (Chart*)EnvironmentSetup::GetObj("SONG");
	SAFE_DELETE(chart);

	return true;
}

void* GameplayScene::CreateScreenshotWin32() {
	
	
	return nullptr;
}
