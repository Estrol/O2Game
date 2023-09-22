#include "GameplayScene.h"
#include <Exception/SDLException.h>

#include <random>
#include <future>
#include <iostream>
#include <unordered_map>
#include <limits.h>
#include <numeric>

#include "Game.h"
#include "MsgBox.h"
#include "SceneManager.h"
#include "Configuration.h"
#include "Rendering/Window.h"

#include "Imgui/ImguiUtil.h"
#include "Imgui/imgui.h"
#include "Texture/MathUtils.h"
#include "Texture/ImageGenerator.h"

#include "../GameScenes.h"
#include "../EnvironmentSetup.hpp"
#include "../Resources/SkinConfig.hpp"
#include "../Engine/NoteImageCacheManager.hpp"

struct MissInfo {
	int type;
	float beat;
	float hit_beat;
	float time;
};

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
			if (EnvironmentSetup::GetInt("Key") >= 0) {
				m_ended = true;
				SceneManager::ChangeScene(GameScene::SONGSELECT);
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
		SceneManager::DisplayFade(100, [] {
			SceneManager::ChangeScene(GameScene::RESULT);
		});
	}

	int difficulty = EnvironmentSetup::GetInt("Difficulty");
	if (difficulty >= 1 && m_starting) {
		float health = m_game->GetScoreManager()->GetLife();

		if (health <= 0) {
			m_game->Stop();
		}
	}

	if (m_doExit && !m_ended) {
		m_ended = true;

		auto scores = m_game->GetScoreManager()->GetScore();

		if (std::get<1>(scores) != 0 || std::get<2>(scores) != 0 || std::get<3>(scores) != 0 || std::get<4>(scores) != 0) {
			SceneManager::DisplayFade(100, [] {
				SceneManager::ChangeScene(GameScene::RESULT);
			});
		}
		else {
			SceneManager::DisplayFade(100, [] {
				SceneManager::ChangeScene(GameScene::SONGSELECT);
			});
		}
	}

	m_exitButtonFunc->Input(delta);
	m_game->Update(delta);
}

void GameplayScene::Render(double delta) {
	if (m_resourceFucked) {
		return;
	}

	ImguiUtil::NewFrame();

	bool is_flhd_enabled = m_laneHideImage.get() != nullptr;

	m_PlayBG->Draw();
	m_Playfield->Draw();
	if (!is_flhd_enabled) {
		m_targetBar->Draw(delta);
	}

	for (auto& [lane, pressed] : m_keyState) {
		if (pressed) {
			m_keyLighting[lane]->AlphaBlend = true;
			m_keyLighting[lane]->Draw();
			m_keyButtons[lane]->Draw();
		}
	}

	m_game->Render(delta);

	if (is_flhd_enabled) {
		m_laneHideImage->Draw();
		m_targetBar->Draw(delta);
	}

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

	Rect rc = {};
	rc.left = (int)curLifeTex->AbsolutePosition.X;
	rc.top = (int)curLifeTex->AbsolutePosition.Y;
	rc.right = rc.left + (int)curLifeTex->AbsoluteSize.X;
	rc.bottom = rc.top + (int)curLifeTex->AbsoluteSize.Y + 5; // Need to add + value because wiggle effect =w=
	float alpha = (float)(kMaxLife - m_game->GetScoreManager()->GetLife()) / kMaxLife;

	// Add wiggle effect
	float yOffset = 0.0f;
	// Wiggle effect after the first second
	yOffset = sinf((float)m_game->GetElapsedTime() * 75.0f) * 5.0f;

	int topCur = (int)::round((1.0f - alpha) * rc.top + alpha * rc.bottom);
	rc.top = topCur + static_cast<int>(::round(yOffset));
	if (rc.top >= rc.bottom) {
		rc.top = rc.bottom - 1;
	}

	m_lifeBar->Draw(delta, &rc);

	if (m_drawJudge && m_judgement[m_judgeIndex] != nullptr) {
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

	if (m_drawCombo && std::get<7>(scores) > 0) {
		m_amplitude = 30.0;
		m_wiggleTime = m_comboTimer * 60.0 * 2.0;
		m_wiggleOffset = std::sin(m_wiggleTime) * m_amplitude;

		constexpr double comboFrame = 3.0;

		double currentAmplitude;

		if (m_wiggleTime < comboFrame) {
			currentAmplitude = m_amplitude / 2 + (m_wiggleTime / comboFrame) * (m_amplitude / 2);
		}
		else {
			currentAmplitude = m_amplitude * std::pow(0.6, m_wiggleTime - comboFrame);
		}

		m_comboLogo->Position2 = UDim2::fromOffset(0, currentAmplitude / 3.0);
		m_comboLogo->Draw(delta);

		m_comboNum->Position2 = UDim2::fromOffset(0, currentAmplitude);
		m_comboNum->DrawNumber(std::get<7>(scores));

		m_comboTimer += delta;
		if (m_comboTimer > 1.0) {
			m_drawCombo = false;
		}
	}

	if (m_drawLN && std::get<9>(scores) > 0) {
		m_wiggleTime = m_lnTimer * 60.0;
		m_wiggleOffset = std::sin(m_wiggleTime) * 5.0;

		constexpr double comboFrameLN = 3.0;

		m_lnLogo->Position2 = UDim2::fromOffset(0, 0);
		m_lnComboNum->Position2 = UDim2::fromOffset(0, 0);

		if (m_wiggleTime < comboFrameLN) {
			m_lnLogo->Position2 = UDim2::fromOffset(0, m_wiggleOffset);
			m_lnComboNum->Position2 = UDim2::fromOffset(0, m_wiggleOffset);
		}

		m_lnLogo->Draw(delta);

		if (m_wiggleTime < comboFrameLN) {
			m_lnComboNum->Position2 = UDim2::fromOffset(0, m_wiggleOffset);
		}
		else {
			m_lnComboNum->Position2 = UDim2::fromOffset(0, 0);
		}

		m_lnComboNum->DrawNumber(std::get<9>(scores));

		m_lnTimer += delta;
		if (m_lnTimer > 1.0) {
			m_drawLN = false;
			m_lnLogo->Reset();
		}
	}


	float gaugeVal = (float)m_game->GetScoreManager()->GetJamGauge() / kMaxJamGauge;
	if (gaugeVal > 0) {
		m_jamGauge->CalculateSize();// Thanks estrol for make this easier to fix than before :pog: because moving RECT rc to GameplayScene :D

		int lerp;
		if (m_jamGauge->AbsoluteSize.Y > m_jamGauge->AbsoluteSize.X) {
			// Fill from bottom to top
			lerp = static_cast<int>(std::lerp(0.0, m_jamGauge->AbsoluteSize.Y, (double)gaugeVal));
			Rect rc = {
				(int)m_jamGauge->AbsolutePosition.X,
				(int)(m_jamGauge->AbsolutePosition.Y + m_jamGauge->AbsoluteSize.Y - lerp),
				(int)(m_jamGauge->AbsolutePosition.X + m_jamGauge->AbsoluteSize.X),
				(int)(m_jamGauge->AbsolutePosition.Y + m_jamGauge->AbsoluteSize.Y)
			};

			m_jamGauge->Draw(&rc);
		}
		else {
			// Fill from left to right
			lerp = static_cast<int>(std::lerp(0.0, m_jamGauge->AbsoluteSize.X, gaugeVal));
			Rect rc = {
				(int)m_jamGauge->AbsolutePosition.X,
				(int)m_jamGauge->AbsolutePosition.Y,
				(int)(m_jamGauge->AbsolutePosition.X + lerp),
				(int)(m_jamGauge->AbsolutePosition.Y + m_jamGauge->AbsoluteSize.Y)
			};

			m_jamGauge->Draw(&rc);
		}
	}

	float currentProgress = (float)m_game->GetAudioPosition() / (float)m_game->GetAudioLength();
	if (currentProgress > 0) {
		m_waveGage->CalculateSize();

		int min = 0, max = (int)m_waveGage->AbsoluteSize.X;
		int lerp = (int)std::lerp(min, max, currentProgress);

		Rect rc = {
			(int)m_waveGage->AbsolutePosition.X,
			(int)m_waveGage->AbsolutePosition.Y,
			(int)(m_waveGage->AbsolutePosition.X + lerp),
			(int)(m_waveGage->AbsolutePosition.Y + m_waveGage->AbsoluteSize.Y)
		};

		m_waveGage->Draw(&rc);
	}

	int PlayTime = std::clamp(m_game->GetPlayTime(), 0, INT_MAX);
	int currentMinutes = PlayTime / 60;
	int currentSeconds = PlayTime % 60;

	m_minuteNum->SetValue(currentMinutes);
	m_minuteNum->DrawNumber(currentMinutes);
	m_secondNum->SetValue(currentSeconds);
	m_secondNum->DrawNumber(currentSeconds);

	for (int i = 0; i < 7; i++) {
		m_hitEffect[i]->Draw(delta); 

		if (m_drawHold[i]) {
			m_holdEffect[i]->Draw(delta);
		}
	}

	if (m_drawExitButton) {
		m_exitBtn->Draw();
	}

	m_title->Draw(m_game->GetTitle());

	if (m_autoPlay) {
		m_autoText->Position = m_autoTextPos;
		m_autoText->Draw(u8"Game currently on Autoplay!");

		m_autoTextPos.X.Offset -= delta * 30.0;
		if (m_autoTextPos.X.Offset < (-m_autoTextSize + 20)) {
			m_autoTextPos = UDim2::fromOffset(GameWindow::GetInstance()->GetBufferWidth(), 50);
		}
	}
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
	m_drawJudge = false;
	m_autoPlay = EnvironmentSetup::GetInt("Autoplay") == 1;

	try {
		int LaneOffset = 5;
		int HitPos = 480;

		try {
			LaneOffset = std::atoi(Configuration::Skin_LoadValue("Game", "LaneOffset").c_str());
			HitPos = std::atoi(Configuration::Skin_LoadValue("Game", "HitPos").c_str());
		}
		catch (std::invalid_argument) {
			throw std::runtime_error("Invalid parameter on Skin::Game::LaneOffset or Skin::Game::HitPos");
		}

		int arena = EnvironmentSetup::GetInt("Arena");
		if (arena == 0) {
			std::random_device dev;
			std::mt19937 rng(dev());

			std::uniform_int_distribution<> dist(1, 12);

			arena = dist(rng);
		}

		auto skinPath = Configuration::Skin_GetPath();
		auto playingPath = skinPath / "Playing";
		SkinConfig conf(playingPath / "Playing.ini", 7);
		SkinConfig arena_conf(playingPath / "Arena" / std::to_string(arena) / "Arena.ini", 7);

		for (int i = 0; i < 7; i++) {
			m_keyState[i] = false;
			m_drawHit[i] = false;
			m_drawHold[i] = false;
		}

		m_title = std::make_unique<Text>(13);
		auto TitlePos = conf.GetPosition("Title");
		auto RectPos = conf.GetRect("Title");
		m_title->Position = UDim2::fromOffset(TitlePos[0].X, TitlePos[0].Y);
		m_title->AnchorPoint = { TitlePos[0].AnchorPointX, TitlePos[0].AnchorPointY };
		m_title->Clip = { RectPos[0].X, RectPos[0].Y, RectPos[0].Width, RectPos[0].Height };

		m_autoText = std::make_unique<Text>(13);
		m_autoTextSize = m_autoText->CalculateSize(u8"Game currently on autoplay!");

		m_autoTextPos = UDim2::fromOffset(GameWindow::GetInstance()->GetBufferWidth(), 50);

		m_PlayBG = std::make_unique<Texture2D>(playingPath / "Arena" / std::to_string(arena) / ("PlayingBG.png"));
		auto PlayBGPos = arena_conf.GetPosition("PlayingBG");
		m_PlayBG->Position = UDim2::fromOffset(PlayBGPos[0].X, PlayBGPos[0].Y);
		m_PlayBG->AnchorPoint = { PlayBGPos[0].AnchorPointX, PlayBGPos[0].AnchorPointY };

		auto conKeyLight = conf.GetPosition("KeyLighting");
		auto conKeyButton = conf.GetPosition("KeyButton");

		if (conKeyLight.size() < 7 || conKeyButton.size() < 7) {
			throw std::runtime_error("Playing.ini : Positions : KeyLighting#KeyButton : Not enough positions! (count < 7)");
		}

		m_Playfield = std::make_unique<Texture2D>(playingPath / "Playfield.png");
		for (int i = 0; i < 7; i++) {
			m_keyLighting[i] = std::move(std::make_unique<Texture2D>(playingPath / ("KeyLighting" + std::to_string(i) + ".png")));
			m_keyButtons[i] = std::move(std::make_unique<Texture2D>(playingPath / ("KeyButton" + std::to_string(i) + ".png")));

			m_keyLighting[i]->Position = UDim2::fromOffset(conKeyLight[i].X, conKeyLight[i].Y);
			m_keyButtons[i]->Position = UDim2::fromOffset(conKeyButton[i].X, conKeyButton[i].Y);
		}

		std::vector<std::filesystem::path> numComboPaths = {};
		for (int i = 0; i < 10; i++) {
			numComboPaths.emplace_back(playingPath / ("ComboNum" + std::to_string(i) + ".png"));

			if (!CheckSkinComponent(numComboPaths.back())) {
				throw std::runtime_error("Failed to load Integer Images 0-9, please check your skin folder.");
			}
		}

		m_comboNum = std::make_unique<NumericTexture>(numComboPaths);
		auto numPos = arena_conf.GetNumeric("Combo").front();

		m_comboNum->Position = UDim2::fromOffset(numPos.X, numPos.Y);
		m_comboNum->NumberPosition = IntToPos(numPos.Direction);
		m_comboNum->MaxDigits = numPos.MaxDigit;
		m_comboNum->FillWithZeros = numPos.FillWithZero;
		m_comboNum->AlphaBlend = true;

		std::vector<std::filesystem::path> numJamPaths = {};
		for (int i = 0; i < 10; i++) {
			numJamPaths.emplace_back(playingPath / ("JamNum" + std::to_string(i) + ".png"));

			if (!CheckSkinComponent(numJamPaths.back())) {
				throw std::runtime_error("Failed to load Integer Images 0-9, please check your skin folder.");
			}
		}

		m_jamNum = std::make_unique<NumericTexture>(numJamPaths);
		numPos = conf.GetNumeric("Jam").front();

		m_jamNum->Position = UDim2::fromOffset(numPos.X, numPos.Y);
		m_jamNum->NumberPosition = IntToPos(numPos.Direction);
		m_jamNum->MaxDigits = numPos.MaxDigit;
		m_jamNum->FillWithZeros = numPos.FillWithZero;

		std::vector<std::filesystem::path> numScorePaths = {};
		for (int i = 0; i < 10; i++) {
			numScorePaths.emplace_back(playingPath / ("ScoreNum" + std::to_string(i) + ".png"));

			if (!CheckSkinComponent(numScorePaths.back())) {
				throw std::runtime_error("Failed to load Integer Images 0-9, please check your skin folder.");
			}
		}

		m_scoreNum = std::make_unique<NumericTexture>(numScorePaths);
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
			m_judgement[i] = std::move(std::make_unique<Texture2D>(playingPath / ("Judge" + judgeFileName[i] + ".png")));

			if (!CheckSkinComponent(numScorePaths.back())) {
				throw std::runtime_error("Failed to load Judge image!");
			}

			m_judgement[i]->Position = UDim2::fromOffset(judgePos[i].X, judgePos[i].Y);
			m_judgement[i]->AlphaBlend = true;
		}

		m_jamGauge = std::make_unique<Texture2D>(playingPath / "JamGauge.png");
		auto gaugePos = conf.GetPosition("JamGauge");
		if (gaugePos.size() < 1) {
			throw std::runtime_error("Playing.ini : Positions : JamGauge : Position Not defined!");
		}

		m_jamGauge->Position = UDim2::fromOffset(gaugePos[0].X, gaugePos[0].Y);
		m_jamGauge->AnchorPoint = { gaugePos[0].AnchorPointX, gaugePos[0].AnchorPointY };

		auto jamLogoPos = conf.GetSprite("JamLogo");
		std::vector<std::filesystem::path> jamLogoFileName = {};
		for (int i = 0; i < jamLogoPos.numOfFrames; i++) {
			jamLogoFileName.emplace_back(playingPath / ("JamLogo" + std::to_string(i) + ".png"));

			if (!CheckSkinComponent(jamLogoFileName.back())) {
				throw std::runtime_error("Failed to load Jam Logo image!");
			}
		}

		m_jamLogo = std::make_unique<Sprite2D>(jamLogoFileName, 0.25f);
		m_jamLogo->Position = UDim2::fromOffset(jamLogoPos.X, jamLogoPos.Y);
		m_jamLogo->AnchorPoint = { (double)jamLogoPos.AnchorPointX, (double)jamLogoPos.AnchorPointY };

		auto lifeBarPos = conf.GetSprite("LifeBar");
		std::vector<std::filesystem::path> lifeBarFileName = {};
		for (int i = 0; i < lifeBarPos.numOfFrames; i++) {
			lifeBarFileName.emplace_back(playingPath / ("LifeBar" + std::to_string(i) + ".png"));

			if (!CheckSkinComponent(lifeBarFileName.back())) {
				throw std::runtime_error("Failed to load Life Bar image!");
			}
		}

		m_lifeBar = std::make_unique<Sprite2D>(lifeBarFileName, 0.15f);
		m_lifeBar->Position = UDim2::fromOffset(lifeBarPos.X, lifeBarPos.Y);
		m_lifeBar->AnchorPoint = { lifeBarPos.AnchorPointX, lifeBarPos.AnchorPointY };

		std::vector<std::filesystem::path> lnComboFileName = {};
		for (int i = 0; i < 10; i++) {
			lnComboFileName.emplace_back(playingPath / ("LongNoteNum" + std::to_string(i) + ".png"));

			if (!CheckSkinComponent(lnComboFileName.back())) {
				throw std::runtime_error("Failed to load Long Note Combo image!");
			}
		}

		std::vector<std::filesystem::path> statsNumFileName = {};
		for (int i = 0; i < 10; i++) {
			statsNumFileName.emplace_back(playingPath / ("StatsNum" + std::to_string(i) + ".png"));

			if (!CheckSkinComponent(statsNumFileName.back())) {
				throw std::runtime_error("Failed to load Stats Number image!");
			}
		}

		m_statsNum = std::make_unique<NumericTexture>(statsNumFileName);
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

		m_lnComboNum = std::make_unique<NumericTexture>(lnComboFileName);
		auto lnComboPos = conf.GetNumeric("LongNoteCombo");
		if (lnComboPos.size() < 1) {
			throw std::runtime_error("Playing.ini : Numerics : LongNoteCombo : Position Not defined!");
		}

		auto btnExitPos = conf.GetPosition("ExitButton");
		auto btnExitRect = conf.GetRect("Exit");

		if (btnExitPos.size() < 1 || btnExitRect.size() < 1) {
			throw std::runtime_error("Playing.ini : Positions|Rect : Exit : Not defined!");
		}

		m_exitBtn = std::make_unique<Texture2D>(playingPath / "Exit.png");
		m_exitBtn->Position = UDim2::fromOffset(btnExitRect[0].X, btnExitRect[0].Y); // Fix Exit not functional with Playing.ini
		m_exitBtn->AnchorPoint = { btnExitPos[0].AnchorPointX, btnExitPos[0].AnchorPointY };

		m_exitButtonFunc = std::make_unique<Button>(btnExitRect[0].X, btnExitRect[0].Y, btnExitRect[0].Width, btnExitRect[0].Height,
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

		auto lnLogoPos = conf.GetSprite("LongNoteLogo");
		std::vector<std::filesystem::path> lnLogoFileName = {};
		for (int i = 0; i < lnLogoPos.numOfFrames; i++) {
			lnLogoFileName.emplace_back(playingPath / ("LongNoteLogo" + std::to_string(i) + ".png"));

			if (!CheckSkinComponent(lnLogoFileName.back())) {
				throw std::runtime_error("Failed to load Long Note Logo image!");
			}
		}

		m_lnLogo = std::make_unique<Sprite2D>(lnLogoFileName, 0.25f);
		m_lnLogo->Position = UDim2::fromOffset(lnLogoPos.X, lnLogoPos.Y);
		m_lnLogo->AnchorPoint = { lnLogoPos.AnchorPointX, lnLogoPos.AnchorPointY };
		m_lnLogo->AlphaBlend = true;

		auto comboLogoPos = arena_conf.GetSprite("ComboLogo");
		std::vector<std::filesystem::path> comboFileName = {};
		for (int i = 0; i < comboLogoPos.numOfFrames; i++) {
			auto file = playingPath / ("ComboLogo" + std::to_string(i) + ".png");

			if (!CheckSkinComponent(file)) {
				break;
			}

			comboFileName.emplace_back(file);
		}

		m_comboLogo = std::make_unique<Sprite2D>(comboFileName, 0.25f);
		m_comboLogo->Position = UDim2::fromOffset(comboLogoPos.X, comboLogoPos.Y);
		m_comboLogo->AnchorPoint = { comboLogoPos.AnchorPointX, comboLogoPos.AnchorPointY };
		m_comboLogo->AlphaBlend = true;

		m_waveGage = std::make_unique<Texture2D>(playingPath / "WaveGage.png");
		auto waveGagePos = conf.GetPosition("WaveGage").front();
		m_waveGage->Position = UDim2::fromOffset(waveGagePos.X, waveGagePos.Y);
		m_waveGage->AnchorPoint = { waveGagePos.AnchorPointX, waveGagePos.AnchorPointY };

		std::vector<std::filesystem::path> numTimerPaths = {};
		for (int i = 0; i < 10; i++) {
			numTimerPaths.emplace_back(playingPath / ("PlayTimeNum" + std::to_string(i) + ".png"));

			if (!CheckSkinComponent(numTimerPaths.back())) {
				throw std::runtime_error("Failed to load Timer Images 0-9, please check your skin folder.");
			}
		}

		auto targetPos = conf.GetSprite("TargetBar");
		std::vector<std::filesystem::path> targetBarPaths = {};
		for (int i = 0; i < targetPos.numOfFrames; i++) {
			targetBarPaths.emplace_back(playingPath / ("TargetBar" + std::to_string(i) + ".png"));

			if (!CheckSkinComponent(targetBarPaths.back())) {
				throw std::runtime_error("Failed to load TargetBar Images, please check your skin folder.");
			}
		}

		m_targetBar = std::make_unique<Sprite2D>(targetBarPaths);
		m_targetBar->Position = UDim2::fromOffset(targetPos.X, targetPos.Y);
		m_targetBar->AnchorPoint = { targetPos.AnchorPointX, targetPos.AnchorPointY };
		m_targetBar->SetFPS(5);

		m_minuteNum = std::make_unique<NumericTexture>(numTimerPaths);
		auto minutePos = conf.GetNumeric("Minute");
		m_minuteNum->NumberPosition = IntToPos(minutePos[0].Direction);
		m_minuteNum->MaxDigits = minutePos[0].MaxDigit;
		m_minuteNum->FillWithZeros = minutePos[0].FillWithZero;
		m_minuteNum->Position = UDim2::fromOffset(minutePos[0].X, minutePos[0].Y);

		m_secondNum = std::make_unique<NumericTexture>(numTimerPaths);
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
			m_pills[i] = std::move(std::make_unique<Texture2D>(file));
			m_pills[i]->Position = UDim2::fromOffset(pos.X, pos.Y);
			m_pills[i]->AnchorPoint = { pos.AnchorPointX, pos.AnchorPointY };
		}

		Chart* chart = (Chart*)EnvironmentSetup::GetObj("SONG");
		if (chart == nullptr) {
			throw std::runtime_error("Fatal error: Chart is null");
		}

		m_game = std::make_unique<RhythmEngine>();

		if (!m_game) {
			throw std::runtime_error("Failed to load game!");
		}

		m_game->SetHitPosition(HitPos);
		m_game->SetLaneOffset(LaneOffset);

		int idx = 2;
		try {
			idx = std::atoi(Configuration::Load("Game", "GuideLine").c_str());
		}
		catch (std::invalid_argument) {
			idx = 2;
		}

		m_game->SetGuideLineIndex(idx);

		m_game->ListenKeyEvent([&](GameTrackEvent e) {
			if (e.IsKeyEvent) {
				m_keyState[e.Lane] = e.State;
			}
			else {
				if (e.IsHitEvent) {
					if (e.IsHitLongEvent) {
						m_holdEffect[e.Lane]->ResetIndex();
						m_drawHit[e.Lane] = false;
						m_drawHold[e.Lane] = e.State;

						if (!e.State) {
							m_hitEffect[e.Lane]->ResetIndex();
							m_drawHit[e.Lane] = true;
						}
					}
					else {
						m_hitEffect[e.Lane]->ResetIndex();
						m_drawHit[e.Lane] = true;
						m_drawHold[e.Lane] = false;
					}
				}
			}
			});

		m_game->Load(chart);

		std::map<int, std::vector<int>> mappedKeyIndex = {
			// 4: 1, 2, x, x, x, 3, 4
			{ 4, { 0, 1, 5, 6 } },
			// 5: x, 1, 2, 3, 4, 5, x
			{ 5, { 1, 2, 3, 4, 5 } },
			// 6: 1, 2, 3, 4, 5, 6, x
			{ 6, { 0, 1, 2, 3, 4, 5 } },
			// 7: 1, 2, 3, 4, 5, 6, 7
			{ 7, { 0, 1, 2, 3, 4, 5, 6 } },
		};

		Keys keys[7] = {};
		std::string keyName = "Lane";
		if (chart->m_keyCount != 7) {
			keyName = std::to_string(chart->m_keyCount) + "_" + keyName;
		}

		for (int i = 0; i < chart->m_keyCount; i++) {
			auto value = Configuration::Load("KeyMapping", keyName + std::to_string(i + 1));
			auto key = static_cast<Keys>(SDL_GetScancodeFromName(value.c_str()));
			if (key == Keys::INVALID_KEY) {
				throw std::runtime_error(("Unknown key: " + value + ", try check your keybind again!"));
			}

			keys[mappedKeyIndex[chart->m_keyCount][i]] = key;
		}

		m_game->SetKeys(keys);

		std::vector<std::filesystem::path> HitEffect = {};
		for (int i = 0; i < 9; i++) {
			auto path = playingPath / ("HitEffect" + std::to_string(i) + ".png");

			if (!CheckSkinComponent(path)) {
				break;
			}

			HitEffect.emplace_back(path);
		}

		std::vector<std::filesystem::path> holdEffect = {};
		for (int i = 0; i < 9; i++) {
			auto path = playingPath / ("HoldEffect" + std::to_string(i) + ".png");

			if (!CheckSkinComponent(path)) {
				break;
			}

			holdEffect.emplace_back(path);
		}

		auto lanePos = m_game->GetLanePos();
		auto laneSize = m_game->GetLaneSizes();
		for (int i = 0; i < 7; i++) {
			m_hitEffect[i] = std::move(std::make_unique<FrameTimer>(HitEffect));
			m_holdEffect[i] = std::move(std::make_unique<FrameTimer>(holdEffect));

			m_hitEffect[i]->SetFPS(30.0);
			m_holdEffect[i]->SetFPS(30.0);
			m_hitEffect[i]->LastIndex();
			m_holdEffect[i]->Repeat = true;
			m_hitEffect[i]->AlphaBlend = true;
			m_holdEffect[i]->AlphaBlend = true;

			int pos = (int)std::ceil(static_cast<double>(lanePos[i]) + (static_cast<double>(laneSize[i]) / 2.0));
			m_hitEffect[i]->Position = UDim2::fromOffset(pos, HitPos - 15); // -15 or it will too Deep
			m_holdEffect[i]->Position = UDim2::fromOffset(pos, HitPos - 15); // -15 or it will too Deep
			m_hitEffect[i]->AnchorPoint = { .5, .45 };
			m_holdEffect[i]->AnchorPoint = { .5, .45 };

			m_lifeBar->SetFPS(15);
		}

		bool IsHD = EnvironmentSetup::GetInt("Hidden") == 1;
		bool IsFL = EnvironmentSetup::GetInt("Flashlight") == 1;
		if (IsHD || IsFL) {
			std::vector<Segment> segments;

			if (IsFL) {
				segments = {
					{0.00f, 0.20f, {0, 0, 0, 255}, {0, 0, 0, 255}},
					{0.33f, 0.38f, {0, 0, 0, 255}, {0, 0, 0, 0	}},
					{0.38f, 0.61f, {0, 0, 0, 0	}, {0, 0, 0, 0	}},
					{0.61f, 0.67f, {0, 0, 0, 0	}, {0, 0, 0, 255}},
					{0.67f, 1.00f, {0, 0, 0, 255}, {0, 0, 0, 255}}
				};
			} else {
				segments = {
					{0.00f, 0.00f, {0, 0, 0, 255}, {0, 0, 0, 255}},
					{0.00f, 0.00f, {0, 0, 0, 255}, {0, 0, 0, 0	}},
					{0.00f, 0.45f, {0, 0, 0, 0	}, {0, 0, 0, 0	}},
					{0.45f, 0.50f, {0, 0, 0, 0	}, {0, 0, 0, 255}},
					{0.50f, 1.00f, {0, 0, 0, 255}, {0, 0, 0, 255}}
				};
			}

			int imageWidth = std::accumulate(laneSize, laneSize + 7, 0);
			int imageHeight = HitPos;
			int imagePos = lanePos[0];

			std::vector<uint8_t> imageBuffer = ImageGenerator::GenerateGradientImage(imageWidth, imageHeight, segments);

			m_laneHideImage = std::make_unique<Texture2D>(imageBuffer.data(), imageBuffer.size());
			m_laneHideImage->Position = UDim2::fromOffset(imagePos, 0);
			m_laneHideImage->Size = UDim2::fromOffset(imageWidth, imageHeight );
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
			m_jamLogo->SetFPS(13.33f);
			m_drawJam = true;
			m_jamTimer = 0;
			m_jamLogo->Reset();
		});

		m_game->GetScoreManager()->ListenLongNote([&] {
			m_lnLogo->SetFPS(60);
			m_lnTimer = 0;
			m_drawLN = true;
		});
	}
	catch (SDLException& e) {
		MsgBox::Show("GameplayError", "Image Error", e.what(), MsgBoxType::OK);
		m_resourceFucked = true;
		return true;
	}
	catch (std::exception& e) {
		MsgBox::Show("GameplayError", "Error", e.what(), MsgBoxType::OK);
		m_resourceFucked = true;
		return true;
	}

	SceneManager::GetInstance()->SetFrameLimitMode(FrameLimitMode::GAME);
	m_starting = false;
	return true;
}

bool GameplayScene::Detach() {
	for (int i = 0; i < 7; i++) {
		m_keyLighting[i].reset();
		m_keyButtons[i].reset();
		m_hitEffect[i].reset();
		m_holdEffect[i].reset();
	}

	for (int i = 0; i < 5; i++) {
		//SAFE_DELETE(m_pills[i]);
		m_pills[i].reset();
	}

	for (int i = 0; i < 4; i++) {
		m_judgement[i].reset();
	}

	m_PlayBG.reset();
	m_Playfield.reset();
	m_exitBtn.reset();

	m_jamGauge.reset();
	m_waveGage.reset();
	m_jamLogo.reset();
	m_lifeBar.reset();
	m_lnLogo.reset();
	m_comboLogo.reset();
	m_targetBar.reset();
	m_laneHideImage.reset();

	m_lnComboNum.reset();
	m_jamNum.reset();
	m_scoreNum.reset();
	m_comboNum.reset();
	m_minuteNum.reset();
	m_secondNum.reset();

	if (m_game) {
		auto score = m_game->GetScoreManager()->GetScore();

		EnvironmentSetup::SetInt("Score", std::get<0>(score));
		EnvironmentSetup::SetInt("Cool", std::get<1>(score));
		EnvironmentSetup::SetInt("Good", std::get<2>(score));
		EnvironmentSetup::SetInt("Bad", std::get<3>(score));
		EnvironmentSetup::SetInt("Miss", std::get<4>(score));
		EnvironmentSetup::SetInt("JamCombo", std::get<5>(score));
		EnvironmentSetup::SetInt("MaxJamCombo", std::get<6>(score));
		EnvironmentSetup::SetInt("Combo", std::get<7>(score));
		EnvironmentSetup::SetInt("MaxCombo", std::get<8>(score));
		EnvironmentSetup::SetInt("LNCombo", std::get<9>(score));
		EnvironmentSetup::SetInt("LNMaxCombo", std::get<10>(score));
	}
	
	m_game.reset();
	m_title.reset();
	m_exitButtonFunc.reset();

	SceneManager::GetInstance()->SetFrameLimitMode(FrameLimitMode::MENU);
	return true;
}

void* GameplayScene::CreateScreenshotWin32() {
	return nullptr;
}
