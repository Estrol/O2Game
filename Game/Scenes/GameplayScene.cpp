#include "GameplayScene.h"
#include <unordered_map>
#include <iostream>
#include <future>

#include "../../Engine/Imgui/ImguiUtil.hpp"
#include "../Engine/NoteImageCacheManager.hpp"

#include "../Resources/Configuration.hpp"
#include "../Resources/SkinConfig.hpp"
#include "../Data/Util/mINI.h"

#include "../EnvironmentSetup.hpp"
#include "../GameScenes.h"
#include "../../Engine/MsgBox.hpp"

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
	m_wiggleTime = 0;
	m_wiggleOffset = 0;
}

void GameplayScene::Update(double delta) {
	if (m_resourceFucked) {
		if (!m_ended) {
			if (EnvironmentSetup::Get("Key").size() > 0) {
				m_ended = true;
				SceneManager::ChangeScene(GameScene::MAINMENU);
			}
			else {
				if (MsgBox::GetResult("GameplayError") == 4) {
					m_ended = true;
					SceneManager::GetInstance()->StopGame();
				}
			}
		}
		
		return;
	}

	if (!m_starting) {
		m_starting = true;
		m_game->Start();
	}

	if (m_game->GetState() == GameState::PosGame && !m_ended) {
		m_ended = true;
		SceneManager::ChangeScene(GameScene::RESULT);
	}

	if (m_doExit && !m_ended) {
		m_ended = true;
		
		auto scores = m_game->GetScoreManager()->GetScore();

		if (std::get<1>(scores) != 0 || std::get<2>(scores) != 0 || std::get<3>(scores) != 0 || std::get<4>(scores) != 0) {
			SceneManager::ChangeScene(GameScene::RESULT);
		}
		else {
			SceneManager::ChangeScene(GameScene::MAINMENU);
		}
	}

	m_exitButtonFunc->Input(delta);
	m_game->Update(delta);
}

void GameplayScene::Render(double delta) {
	if (m_resourceFucked) {
		return;
	}

	m_PlayBG->Draw();
	m_Playfield->Draw();

	for (auto& [lane, pressed] : m_keyState) {
		if (pressed) {
			m_keyLighting[lane]->AlphaBlend = true;
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

	int numOfPills = m_game->GetScoreManager()->GetPills();
	for (int i = 0; i < numOfPills; i++) {
		m_pills[i]->Draw();
	}

	auto curLifeTex = m_lifeBar->GetTexture(); // Move lifebar to here so it will not overlapping
	curLifeTex->CalculateSize();

	RECT rc = {};
	rc.left = curLifeTex->AbsolutePosition.X;
	rc.top = curLifeTex->AbsolutePosition.Y;
	rc.right = rc.left + curLifeTex->AbsoluteSize.X;
	rc.bottom = rc.top + curLifeTex->AbsoluteSize.Y + 5; // Need to add + value because wiggle effect =w=
	float alpha = (float)(kMaxLife - m_game->GetScoreManager()->GetLife()) / kMaxLife;
	
	// Add wiggle effect
	float yOffset = 0.0f;
	// Wiggle effect after the first second
	yOffset = sinf(m_game->GetElapsedTime() * 75.0f) * 5.0f;

	LONG topCur = (1.0f - alpha) * rc.top + alpha * rc.bottom;
	rc.top = topCur + static_cast<LONG>(yOffset);

	m_lifeBar->Draw(delta, &rc);
	
	if (m_drawCombo) {
		if (std::get<7>(scores) > 0) {
			m_wiggleTime = m_comboTimer * 120; // Combo animated by Frames per second
			m_amplitude = 30; // Maximum amplitude
			double halfAmplitude = (m_amplitude) / 2; // Half of the maximum amplitude
			double comboLogoReduceAmplitude = 3;
			double dampingFactor = 0.6; // Damping factor to reduce amplitude over time
			m_wiggleOffset = std::sin(m_wiggleTime) * (m_amplitude);

			if (m_wiggleTime < M_PI) {
				double currentAmplitude = halfAmplitude + (m_wiggleTime / M_PI) * ((m_amplitude)-halfAmplitude); // Gradually increase amplitude

				m_comboLogo->Position2 = UDim2::fromOffset(0, currentAmplitude / comboLogoReduceAmplitude);
				m_comboLogo->Draw(delta);

				m_comboNum->Position2 = UDim2::fromOffset(0, currentAmplitude);
				m_comboNum->DrawNumber(std::get<7>(scores));
			}
			else {
				double elapsedWiggleTime = m_wiggleTime - M_PI; // Time after the initial wiggle phase
				double dampingAmplitude = m_amplitude * std::pow(dampingFactor, elapsedWiggleTime); // Reduce amplitude using damping factor

				m_comboLogo->Position2 = UDim2::fromOffset(0, dampingAmplitude / comboLogoReduceAmplitude);
				m_comboLogo->Draw(delta);

				m_comboNum->Position2 = UDim2::fromOffset(0, dampingAmplitude);
				m_comboNum->DrawNumber(std::get<7>(scores));
			}
		}

		m_comboTimer += delta;
		if (m_comboTimer > 1) {
			m_drawCombo = false;
		}
	}

	if (m_drawJudge) {
		m_judgement[m_judgeIndex]->Size = UDim2::fromScale(m_judgeSize, m_judgeSize);
		m_judgement[m_judgeIndex]->AnchorPoint = { 0.5, 0.5 };
		m_judgement[m_judgeIndex]->Draw();

		m_judgeSize = std::clamp(m_judgeSize + (delta * 3), 0.5, 1.0);
		if ((m_judgeTimer += delta) > 0.60) {
			m_drawJudge = false;
		}
	}

	if (m_drawJam) {
		if (std::get<5>(scores) > 0) {
			m_jamNum->DrawNumber(std::get<5>(scores));
			m_jamLogo->Draw(delta);
		}

		if ((m_jamTimer += delta) > 0.60) {
			m_drawJam = false;
		}
	}
	
	if (m_drawLN) {
		if (std::get<9>(scores) > 0) {
			m_wiggleTime = m_lnTimer * 60; // LNCombo animated by Frame per second
			m_wiggleOffset = std::sin(m_wiggleTime) * 12; // Amplitude 

			if (m_wiggleTime < M_PI) {
				m_lnLogo->Position2 = UDim2::fromOffset(0, m_wiggleOffset);
				m_lnLogo->Draw(delta);

				m_lnComboNum->Position2 = UDim2::fromOffset(0, m_wiggleOffset);
				m_lnComboNum->DrawNumber(std::get<9>(scores));
			}
			else {
				m_lnLogo->Position2 = UDim2::fromOffset(0, 0);
				m_lnLogo->Draw(delta);

				m_lnComboNum->Position2 = UDim2::fromOffset(0, 0);
				m_lnComboNum->DrawNumber(std::get<9>(scores));
			}
		}

		m_lnTimer += delta;
		if (m_lnTimer > 1) {
			m_drawLN = false;
		}
	}

	float gaugeVal = (float)m_game->GetScoreManager()->GetJamGauge() / kMaxJamGauge;
	m_jamGauge->Size = UDim2::fromScale(gaugeVal, 1);
	m_jamGauge->Draw();

	float currentProgress = m_game->GetAudioPosition() / m_game->GetAudioLength();
	m_waveGage->Size = UDim2::fromScale(currentProgress, 1);
	m_waveGage->Draw();

	int PlayTime = m_game->GetPlayTime();
	int currentMinutes = PlayTime / 60;
	int currentSeconds = PlayTime % 60;

	m_minuteNum->SetValue(currentMinutes);
	m_minuteNum->DrawNumber(currentMinutes);
	m_secondNum->SetValue(currentSeconds);
	m_secondNum->DrawNumber(currentSeconds);

	for (int i = 0; i < 7; i++) {
		if (m_drawHold[i]) {
			m_holdEffect[i]->Draw(delta);
		}

		m_hitEffect[i]->Draw(delta);
	}

	if (m_drawExitButton) {
		m_exitBtn->Draw();
	}

	m_title->Draw(m_game->GetTitle());
}

void GameplayScene::Input(double delta) {
	if (m_resourceFucked) return;

	m_game->Input(delta);
}

void GameplayScene::OnKeyDown(const KeyState& state) {
	if (m_resourceFucked) return;

	m_game->OnKeyDown(state);
}

void GameplayScene::OnKeyUp(const KeyState& state) {
	if (m_resourceFucked) return;

	m_game->OnKeyUp(state);
}

void GameplayScene::OnMouseDown(const MouseState& state) {
	if (m_resourceFucked) return;

	m_exitButtonFunc->OnKeyDown();
}

bool GameplayScene::Attach() {
	m_ended = false;
	m_starting = false;
	m_doExit = false;
	m_drawExitButton = false;
	m_resourceFucked = false;

	try {
		auto SkinName = Configuration::Load("Game", "Skin");
		int LaneOffset = 5;
		int HitPos = 480;

		try {
			LaneOffset = std::atoi(Configuration::Skin_LoadValue(SkinName, "Game", "LaneOffset").c_str());
			HitPos = std::atoi(Configuration::Skin_LoadValue(SkinName, "Game", "HitPos").c_str());
		}
		catch (std::invalid_argument) {
			throw std::runtime_error("Invalid parameter on Skin::Game::LaneOffset or Skin::Game::HitPos");
		}

		auto skinPath = Configuration::Skin_GetPath(SkinName);
		auto playingPath = skinPath / "Playing";
		SkinConfig conf(playingPath / "Playing.ini", 7);

		for (int i = 0; i < 7; i++) {
			m_keyState[i] = false;
			m_drawHold[i] = false;
			m_drawHit[i] = false;
		}

		m_title = new Text("Arial", 13);
		auto TitlePos = conf.GetPosition("Title");
		m_title->Position = UDim2::fromOffset(TitlePos[0].X, TitlePos[0].Y);
		m_title->AnchorPoint = { TitlePos[0].AnchorPointX, TitlePos[0].AnchorPointY };

		m_PlayBG = new Texture2D(playingPath / "PlayingBG.png");
		auto PlayBGPos = conf.GetPosition("PlayingBG");
		m_PlayBG->Position = UDim2::fromOffset(PlayBGPos[0].X, PlayBGPos[0].Y);
		m_PlayBG->AnchorPoint = { PlayBGPos[0].AnchorPointX, PlayBGPos[0].AnchorPointY };

		auto conKeyLight = conf.GetPosition("KeyLighting");
		auto conKeyButton = conf.GetPosition("KeyButton");

		if (conKeyLight.size() < 7 || conKeyButton.size() < 7) {
			throw std::runtime_error("Playing.ini : Positions : KeyLighting#KeyButton : Not enough positions! (count < 7)");
		}

		m_Playfield = new Texture2D(playingPath / "Playfield.png");
		for (int i = 0; i < 7; i++) {
			m_keyLighting[i] = new Texture2D(playingPath / ("KeyLighting" + std::to_string(i) + ".png"));
			m_keyButtons[i] = new Texture2D(playingPath / ("KeyButton" + std::to_string(i) + ".png"));

			m_keyLighting[i]->Position = UDim2::fromOffset(conKeyLight[i].X, conKeyLight[i].Y);
			m_keyButtons[i]->Position = UDim2::fromOffset(conKeyButton[i].X, conKeyButton[i].Y);
		}

		std::vector<std::filesystem::path> numComboPaths = {};
		for (int i = 0; i < 10; i++) {
			numComboPaths.push_back(playingPath / ("ComboNum" + std::to_string(i) + ".png"));

			if (!CheckSkinComponent(numComboPaths.back())) {
				throw std::runtime_error("Failed to load Integer Images 0-9, please check your skin folder.");
			}
		}

		m_comboNum = new NumericTexture(numComboPaths);
		auto numPos = conf.GetNumeric("Combo").front();

		m_comboNum->Position = UDim2::fromOffset(numPos.X, numPos.Y);
		m_comboNum->NumberPosition = IntToPos(numPos.Direction);
		m_comboNum->MaxDigits = numPos.MaxDigit;
		m_comboNum->FillWithZeros = numPos.FillWithZero;
		m_comboNum->AlphaBlend = true;

		std::vector<std::filesystem::path> numJamPaths = {};
		for (int i = 0; i < 10; i++) {
			numJamPaths.push_back(playingPath / ("JamNum" + std::to_string(i) + ".png"));

			if (!CheckSkinComponent(numJamPaths.back())) {
				throw std::runtime_error("Failed to load Integer Images 0-9, please check your skin folder.");
			}
		}

		m_jamNum = new NumericTexture(numJamPaths);
		numPos = conf.GetNumeric("Jam").front();

		m_jamNum->Position = UDim2::fromOffset(numPos.X, numPos.Y);
		m_jamNum->NumberPosition = IntToPos(numPos.Direction);
		m_jamNum->MaxDigits = numPos.MaxDigit;
		m_jamNum->FillWithZeros = numPos.FillWithZero;

		std::vector<std::filesystem::path> numScorePaths = {};
		for (int i = 0; i < 10; i++) {
			numScorePaths.push_back(playingPath / ("ScoreNum" + std::to_string(i) + ".png"));

			if (!CheckSkinComponent(numScorePaths.back())) {
				throw std::runtime_error("Failed to load Integer Images 0-9, please check your skin folder.");
			}
		}

		m_scoreNum = new NumericTexture(numScorePaths);
		numPos = conf.GetNumeric("Score").front();

		m_scoreNum->Position = UDim2::fromOffset(numPos.X, numPos.Y);
		m_scoreNum->NumberPosition = IntToPos(numPos.Direction);
		m_scoreNum->MaxDigits = numPos.MaxDigit;
		m_scoreNum->FillWithZeros = numPos.FillWithZero;

		std::vector<std::string> judgeFileName = { "Miss", "Bad", "Good", "Cool" };
		auto judgePos = conf.GetPosition("Judge");
		if (judgePos.size() < 4) {
			throw std::runtime_error("Playing.ini : Positions : Judge : Not enough positions! (count < 4)");
		}

		for (int i = 0; i < 4; i++) {
			m_judgement[i] = new Texture2D(playingPath / ("Judge" + judgeFileName[i] + ".png"));

			if (!CheckSkinComponent(numScorePaths.back())) {
				throw std::runtime_error("Failed to load Judge image!");
			}

			m_judgement[i]->Position = UDim2::fromOffset(judgePos[i].X, judgePos[i].Y);
			m_judgement[i]->AlphaBlend = true;
		}

		m_jamGauge = new Tile2D(playingPath / "JamGauge.png");
		auto gaugePos = conf.GetPosition("JamGauge");
		if (gaugePos.size() < 1) {
			throw std::runtime_error("Playing.ini : Positions : JamGauge : Position Not defined!");
		}

		m_jamGauge->Position = UDim2::fromOffset(gaugePos[0].X, gaugePos[0].Y);
		m_jamGauge->AnchorPoint = { gaugePos[0].AnchorPointX, gaugePos[0].AnchorPointY };

		auto jamLogoPos = conf.GetSprite("JamLogo");
		std::vector<std::filesystem::path> jamLogoFileName = {};
		for (int i = 0; i < jamLogoPos.numOfFrames; i++) {
			jamLogoFileName.push_back(playingPath / ("JamLogo" + std::to_string(i) + ".png"));

			if (!CheckSkinComponent(jamLogoFileName.back())) {
				throw std::runtime_error("Failed to load Jam Logo image!");
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
				throw std::runtime_error("Failed to load Life Bar image!");
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
				throw std::runtime_error("Failed to load Long Note Logo image!");
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
				throw std::runtime_error("Failed to load Long Note Combo image!");
			}
		}

		std::vector<std::filesystem::path> statsNumFileName = {};
		for (int i = 0; i < 10; i++) {
			statsNumFileName.push_back(playingPath / ("StatsNum" + std::to_string(i) + ".png"));

			if (!CheckSkinComponent(statsNumFileName.back())) {
				throw std::runtime_error("Failed to load Stats Number image!");
			}
		}

		m_statsNum = new NumericTexture(statsNumFileName);
		auto statsNumPos = conf.GetNumeric("Stats");
		if (statsNumPos.size() < 5) {
			throw std::runtime_error("Playing.ini : Numerics : Stats : Not enough positions! (count < 5)");
		}

		m_statsNum->NumberPosition = IntToPos(statsNumPos[0].Direction);
		m_statsNum->MaxDigits = statsNumPos[0].MaxDigit;
		m_statsNum->FillWithZeros = statsNumPos[0].FillWithZero;
		m_statsNum->NumberPosition = IntToPos(statsNumPos[0].Direction);
		m_statsNum->AnchorPoint = { 0, .5 };

		m_statsPos[0] = UDim2::fromOffset(statsNumPos[0].X, statsNumPos[0].Y); // COOL
		m_statsPos[1] = UDim2::fromOffset(statsNumPos[1].X, statsNumPos[1].Y); // GOOD
		m_statsPos[2] = UDim2::fromOffset(statsNumPos[2].X, statsNumPos[2].Y); // BAD
		m_statsPos[3] = UDim2::fromOffset(statsNumPos[3].X, statsNumPos[3].Y); // MISS
		m_statsPos[4] = UDim2::fromOffset(statsNumPos[4].X, statsNumPos[4].Y); // MAXCOMBO

		m_lnComboNum = new NumericTexture(lnComboFileName);
		auto lnComboPos = conf.GetNumeric("LongNoteCombo");
		if (lnComboPos.size() < 1) {
			throw std::runtime_error("Playing.ini : Numerics : LongNoteCombo : Position Not defined!");
		}

		auto btnExitPos = conf.GetPosition("ExitButton");
		auto btnExitRect = conf.GetRect("Exit");

		if (btnExitPos.size() < 1 || btnExitRect.size() < 1) {
			throw std::runtime_error("Playing.ini : Positions|Rect : Exit : Not defined!");
		}

		m_exitBtn = new Texture2D(playingPath / "Exit.png");
		m_exitBtn->Position = UDim2::fromOffset(btnExitRect[0].X, btnExitRect[0].Y); // Fix Exit not functional with Playing.ini
		m_exitBtn->AnchorPoint = { btnExitPos[0].AnchorPointX, btnExitPos[0].AnchorPointY };

		m_exitButtonFunc = new Button(btnExitRect[0].X, btnExitRect[0].Y, btnExitRect[0].Width, btnExitRect[0].Height,
			[&](int state) {
				m_drawExitButton = state;
			},
			[&]() {
				m_doExit = true;
			}
		);

		m_lnComboNum->Position = UDim2::fromOffset(lnComboPos[0].X, lnComboPos[0].Y);
		m_lnComboNum->NumberPosition = IntToPos(lnComboPos[0].Direction);
		m_lnComboNum->MaxDigits = lnComboPos[0].MaxDigit;
		m_lnComboNum->FillWithZeros = lnComboPos[0].FillWithZero;
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
		auto waveGagePos = conf.GetPosition("WaveGage").front();
		m_waveGage->Position = UDim2::fromOffset(waveGagePos.X, waveGagePos.Y);
		m_waveGage->AnchorPoint = { waveGagePos.AnchorPointX, waveGagePos.AnchorPointY };

		std::vector<std::filesystem::path> numTimerPaths = {};
		for (int i = 0; i < 10; i++) {
			numTimerPaths.emplace_back(playingPath / ("PlayTimeNum" + std::to_string(i) + ".png"));

			if (!CheckSkinComponent(numTimerPaths.back())) {
				MessageBoxA(NULL, "Failed to load Timer Images 0-9, please check your skin folder.", "Error", MB_OK);
				return false;
			}
		}

		m_minuteNum = new NumericTexture(numTimerPaths);
		auto minutePos = conf.GetNumeric("Minute");
		m_minuteNum->NumberPosition = IntToPos(minutePos[0].Direction);
		m_minuteNum->MaxDigits = minutePos[0].MaxDigit;
		m_minuteNum->FillWithZeros = minutePos[0].FillWithZero;
		m_minuteNum->Position = UDim2::fromOffset(minutePos[0].X, minutePos[0].Y);

		m_secondNum = new NumericTexture(numTimerPaths);
		auto secondPos = conf.GetNumeric("Second");
		m_secondNum->NumberPosition = IntToPos(secondPos[0].Direction);
		m_secondNum->MaxDigits = secondPos[0].MaxDigit;
		m_secondNum->FillWithZeros = secondPos[0].FillWithZero;
		m_secondNum->Position = UDim2::fromOffset(secondPos[0].X, secondPos[0].Y);

		auto pillsPosition = conf.GetPosition("Pill");
		if (pillsPosition.size() < 5) {
			throw std::runtime_error("Playing.ini : Positions : Pill : Not enough positions! (count < 5)");
		}

		for (int i = 0; i < 5; i++) {
			auto file = playingPath / ("Pill" + std::to_string(i) + ".png");

			auto pos = pillsPosition[i];
			m_pills[i] = new Texture2D(file);
			m_pills[i]->Position = UDim2::fromOffset(pos.X, pos.Y);
			m_pills[i]->AnchorPoint = { pos.AnchorPointX, pos.AnchorPointY };
		}

		Chart* chart = (Chart*)EnvironmentSetup::GetObj("SONG");
		if (chart == nullptr) {
			throw std::runtime_error("Fatal error: Chart is null");
		}

		m_game = new RhythmEngine();

		if (!m_game) {
			throw std::runtime_error("Failed to load game!");
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
				throw std::runtime_error(("Unknown key: " + value + ", try check your keybind again!"));
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

			int pos = std::ceil(static_cast<double>(lanePos[i]) + (static_cast<double>(laneSize[i]) / 2.0));
			m_hitEffect[i]->Position = UDim2::fromOffset(pos, 465);
			m_holdEffect[i]->Position = UDim2::fromOffset(pos, 465);
			m_hitEffect[i]->AnchorPoint = { .5, .45 };
			m_holdEffect[i]->AnchorPoint = { .5, .45 };

			m_lifeBar->SetFPS(15);
		}

		m_game->GetScoreManager()->ListenHit([&](NoteHitInfo info) {
			m_scoreTimer = 0;
			m_judgeTimer = 0;
			m_comboTimer = 0;
			m_judgeSize = 0.5;

			m_drawCombo = true;
			m_drawJudge = true;

			m_comboLogo->SetFPS(18);
			m_comboTimer = 0; // Fix crash :troll:
			m_comboLogo->Reset();
			m_judgeIndex = (int)info.Result;
		});

		m_game->GetScoreManager()->ListenJam([&](int combo) {
			m_jamLogo->SetFPS(13.33);
			m_drawJam = true;
			m_jamTimer = 0;
			m_jamLogo->Reset();
		});

		m_game->GetScoreManager()->ListenLongNote([&] {
			m_lnLogo->SetFPS(15);
			m_lnTimer = 0;
			m_drawLN = true;
		});
	}
	catch (std::exception& e) {
		MsgBox::Show("GameplayError", "Error", e.what(), MsgBoxType::OK);
		m_resourceFucked = true;
		return true;
	}

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

	SAFE_DELETE(m_PlayBG);
	SAFE_DELETE(m_Playfield);

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
	SAFE_DELETE(m_minuteNum);
	SAFE_DELETE(m_secondNum);
	SAFE_DELETE(m_exitBtn);

	if (m_game) {
		auto score = m_game->GetScoreManager()->GetScore();

		EnvironmentSetup::Set("Score", std::to_string(std::get<0>(score)));
		EnvironmentSetup::Set("Cool", std::to_string(std::get<1>(score)));
		EnvironmentSetup::Set("Good", std::to_string(std::get<2>(score)));
		EnvironmentSetup::Set("Bad", std::to_string(std::get<3>(score)));
		EnvironmentSetup::Set("Miss", std::to_string(std::get<4>(score)));
		EnvironmentSetup::Set("JamCombo", std::to_string(std::get<5>(score)));
		EnvironmentSetup::Set("MaxJamCombo", std::to_string(std::get<6>(score)));
		EnvironmentSetup::Set("Combo", std::to_string(std::get<7>(score)));
		EnvironmentSetup::Set("MaxCombo", std::to_string(std::get<8>(score)));
		EnvironmentSetup::Set("LNCombo", std::to_string(std::get<9>(score)));
		EnvironmentSetup::Set("LNMaxCombo", std::to_string(std::get<10>(score)));
	}

	SAFE_DELETE(m_game);
	SAFE_DELETE(m_title);
	SAFE_DELETE(m_exitButtonFunc);

	Chart* chart = (Chart*)EnvironmentSetup::GetObj("SONG");
	EnvironmentSetup::SetObj("SONG", nullptr);
	SAFE_DELETE(chart);

	return true;
}

void* GameplayScene::CreateScreenshotWin32() {
	
	
	return nullptr;
}
