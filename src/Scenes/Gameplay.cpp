/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include "Gameplay.h"
#include <Exceptions/EstException.h>
#include <Graphics/NativeWindow.h>
#include <Graphics/Renderer.h>
#include <Imgui/imgui.h>

#include <random>

#include "../Game/Data/chart.hpp"
#include "../Game/Data/ojn.h"

#include "../Game/Core/Skinning/SkinManager.h"
#include "../Game/Env.h"

#include "../Game/Core/Drawable/ImageGenerators.h"
#include "../Game/Data/Util/Util.hpp"

#include "../Game/MsgBoxEx.h"
#include <Configuration.h>
#include <Logs.h>
#include <MsgBox.h>

#include <Audio/AudioEngine.h>
#include <Audio/AudioSample.h>

#include <Exceptions/EstException.h>

#include "../Game/Core/Audio/SampleManager.h"

#define AUTOPLAY_TEXT (const char *)u8"Game currently on autoplay!"

void Gameplay::Update(double delta)
{
    m_Engine->Update(delta);

    static float a = 0;
    a += static_cast<float>(delta);

    static bool started = false;
    if (a >= 1.5 && !started) {
        started = true;
        m_Engine->Start();
    }
}

void Gameplay::Draw(double delta)
{
    // TODO: find better way
    bool is_flhd_enabled = m_laneHideImage.get() != nullptr;

    m_PlayBG->Draw();
    m_Playfooter->Draw();
    m_Playfield->Draw();
    if (!is_flhd_enabled) {
        m_targetBar->Draw(delta);
    }

    for (auto &[lane, pressed] : m_keyState) {
        if (pressed) {
            m_keyLighting[lane]->AlphaBlend = true;
            m_keyLighting[lane]->Draw();
            m_keyButtons[lane]->Draw();
        }
    }

    if (!is_flhd_enabled) {
        m_Engine->DrawTimingLine(delta);
        m_Engine->Draw(delta);
    } else {
        m_Engine->Draw(delta);
        m_Engine->DrawTimingLine(delta);
    }

    if (is_flhd_enabled) {
        m_laneHideImage->Draw();
        m_targetBar->Draw(delta);
    }

    auto &scores = m_Engine->GetScoreManager()->GetScore();
    m_scoreNum->Draw(scores.Score);

    // Draw stats
    {
        m_statsNum->Position = m_statsPos[0];
        m_statsNum->Draw(scores.Cool);
        m_statsNum->Position = m_statsPos[1];
        m_statsNum->Draw(scores.Good);
        m_statsNum->Position = m_statsPos[2];
        m_statsNum->Draw(scores.Bad);
        m_statsNum->Position = m_statsPos[3];
        m_statsNum->Draw(scores.Miss);
        m_statsNum->Position = m_statsPos[4];
        m_statsNum->Draw(scores.MaxCombo);
    }

    int numOfPills = m_Engine->GetScoreManager()->GetPills();
    for (int i = 0; i < numOfPills; i++) {
        m_pills[i]->Draw();
    }

    auto curLifeTex = m_lifeBar->GetImages()[0]; // Move lifebar to here so it will not overlapping
    curLifeTex->CalculateSize();

    Rect rc = {};
    rc.X = (int)curLifeTex->AbsolutePosition.X;
    rc.Y = (int)curLifeTex->AbsolutePosition.Y;
    rc.Width = rc.X + (int)curLifeTex->AbsoluteSize.X;
    rc.Height = rc.Y + (int)curLifeTex->AbsoluteSize.Y + 5; // Need to add + value because wiggle effect =w=
    float alpha = (float)(kMaxLife - m_Engine->GetScoreManager()->GetLife()) / kMaxLife;

    // Add wiggle effect
    float yOffset = 0.0f;
    // Wiggle effect after the first second
    yOffset = sinf((float)m_Engine->GetElapsedTime() * 75.0f) * 5.0f;

    int topCur = (int)::round((1.0f - alpha) * rc.Y + alpha * rc.Height);
    rc.Y = topCur + static_cast<int>(::round(yOffset));
    if (rc.Y >= rc.Height) {
        rc.Y = rc.Height - 1;
    }

    m_lifeBar->Draw(delta, rc);

    if (m_drawJudge && m_judgement[m_judgeIndex] != nullptr) {
        m_judgement[m_judgeIndex]->Size = UDim2::fromScale(m_judgeSize, m_judgeSize);
        m_judgement[m_judgeIndex]->AnchorPoint = { 0.5, 0.5 };
        m_judgement[m_judgeIndex]->Draw();

        m_judgeSize = std::clamp(m_judgeSize + (delta * 6), 0.5, 1.0); // Nice
        if ((m_judgeTimer += delta) > 0.60) {
            m_drawJudge = false;
        }
    }

    if (m_drawJam) {
        if (scores.JamCombo > 0) {
            m_jamNum->Draw(scores.JamCombo);
            m_jamLogo->Draw(delta);
        }

        if ((m_jamTimer += delta) > 0.60) {
            m_drawJam = false;
        }
    }

    if (m_drawCombo && scores.Combo > 0) {
        m_amplitude = 30.0;
        m_wiggleTime = 60.0 * m_comboTimer; // Do not edit

        double currentAmplitude = m_amplitude * std::pow(0.60, m_wiggleTime); // std::pow 0.60 = o2jam increment
        currentAmplitude = std::max(currentAmplitude, 1.0);

        m_comboLogo->Position2 = UDim2::fromOffset(0, currentAmplitude / 3.0);
        m_comboLogo->Draw(delta);

        m_comboNum->Position2 = UDim2::fromOffset(0, currentAmplitude);
        m_comboNum->Draw(scores.Combo);

        m_comboTimer += delta;
        if (m_comboTimer > 1.0) {
            m_drawCombo = false;
        }
    }

    if (m_drawLN && scores.LnCombo > 0) {
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
        } else {
            m_lnComboNum->Position2 = UDim2::fromOffset(0, 0);
        }

        m_lnComboNum->Draw(scores.LnCombo);

        m_lnTimer += delta;
        if (m_lnTimer > 1.0) {
            m_drawLN = false;
            m_lnLogo->Reset();
        }
    }

    float gaugeVal = (float)m_Engine->GetScoreManager()->GetJamGauge() / kMaxJamGauge;
    if (gaugeVal > 0) {
        m_jamGauge->CalculateSize();

        int _lerp;
        if (m_jamGauge->AbsoluteSize.Y > m_jamGauge->AbsoluteSize.X) {
            // Fill from bottom to top
            _lerp = static_cast<int>(lerp(0.0f, (float)m_jamGauge->AbsoluteSize.Y, gaugeVal));
            Rect rc = {
                (int)m_jamGauge->AbsolutePosition.X,
                (int)(m_jamGauge->AbsolutePosition.Y + m_jamGauge->AbsoluteSize.Y - _lerp),
                (int)(m_jamGauge->AbsolutePosition.X + m_jamGauge->AbsoluteSize.X),
                (int)(m_jamGauge->AbsolutePosition.Y + m_jamGauge->AbsoluteSize.Y)
            };

            m_jamGauge->Draw(rc);
        } else {
            // Fill from left to right
            _lerp = static_cast<int>(lerp(0.0f, (float)m_jamGauge->AbsoluteSize.X, gaugeVal));
            Rect rc = {
                (int)m_jamGauge->AbsolutePosition.X,
                (int)m_jamGauge->AbsolutePosition.Y,
                (int)(m_jamGauge->AbsolutePosition.X + _lerp),
                (int)(m_jamGauge->AbsolutePosition.Y + m_jamGauge->AbsoluteSize.Y)
            };

            m_jamGauge->Draw(rc);
        }
    }

    float currentProgress = (float)m_Engine->GetAudioPosition() / (float)m_Engine->GetAudioLength();
    if (currentProgress > 0) {
        m_waveGage->CalculateSize();

        int min = 0, max = (int)m_waveGage->AbsoluteSize.X;
        int _lerp = (int)lerp(min, max, currentProgress);

        Rect rc = {
            (int)m_waveGage->AbsolutePosition.X,
            (int)m_waveGage->AbsolutePosition.Y,
            (int)(m_waveGage->AbsolutePosition.X + _lerp),
            (int)(m_waveGage->AbsolutePosition.Y + m_waveGage->AbsoluteSize.Y)
        };

        m_waveGage->Draw(rc);
    }

    int PlayTime = std::clamp(m_Engine->GetPlayTime(), 0, INT_MAX);
    int currentMinutes = PlayTime / 60;
    int currentSeconds = PlayTime % 60;

    m_minuteNum->Draw(currentMinutes);
    m_secondNum->Draw(currentSeconds);

    for (int i = 0; i < 7; i++) {
        m_hitEffect[i]->Draw(delta);

        if (m_drawHold[i]) {
            m_holdEffect[i]->Draw(delta);
        }
    }

    if (m_drawExitButton) {
        m_exitBtn->Draw();
    }

    m_title->DrawString(m_Engine->GetTitle());

    if (m_autoPlay) {
        m_autoText->Position = m_autoTextPos;
        m_autoText->DrawString(AUTOPLAY_TEXT);

        m_autoTextPos.X.Offset -= delta * 30.0;
        if (m_autoTextPos.X.Offset < (-m_autoTextSize + 20)) {
            auto rect = Graphics::NativeWindow::Get()->GetBufferSize();

            m_autoTextPos = UDim2::fromOffset(rect.Width, 50);
        }
    }
}

void Gameplay::Input(double delta)
{
}

void Gameplay::OnKeyDown(const Inputs::State &state)
{
    m_Engine->OnKeyDown(state);
}

void Gameplay::OnKeyUp(const Inputs::State &state)
{
    m_Engine->OnKeyUp(state);
}

bool Gameplay::Attach()
{
    m_ended = false;
    m_starting = false;
    m_doExit = false;
    m_drawExitButton = false;
    m_resourceFucked = false;
    m_drawJudge = false;

    try {
        auto bufferRect = Graphics::NativeWindow::Get()->GetBufferSize();

        auto manager = SkinManager::Get();
        manager->LoadSkin("Default");
        manager->SetKeyCount(7);
        manager->Arena_SetIndex(1);

        int LaneOffset = 5;
        int HitPos = 480;

        try {
            LaneOffset = std::stoi(manager->GetSkinProp("Game", "LaneOffset", "5"));
            HitPos = std::stoi(manager->GetSkinProp("Game", "HitPos", "480"));
        } catch (const std::invalid_argument &) {
            throw Exceptions::EstException("Invalid parameter on Skin::Game::LaneOffset or Skin::Game::HitPos");
        }

        int  arena = Env::GetInt("Arena");
        auto skinPath = manager->GetPath(); // Move to above, make it easiest
        auto playingPath = skinPath / "Playing";
        auto arenaPath = playingPath / "Arena";

        if (arena == 0) {
            std::random_device dev;
            std::mt19937       rng(dev());

            std::uniform_int_distribution<> dist(1, 12);

            arena = dist(rng);
        }

        arenaPath /= std::to_string(arena);
        Env::SetInt("CurrentArena", arena);

        if (!std::filesystem::exists(arenaPath)) {
            throw Exceptions::EstException("Arena %d is missing from folder: %s", arena, arenaPath.string().c_str());
        }

        manager->SetKeyCount(7);
        manager->Arena_SetIndex(arena);

        for (int i = 0; i < 7; i++) {
            m_keyState[i] = false;
            m_drawHit[i] = false;
            m_drawHold[i] = false;
        }

        m_title = std::make_unique<UI::Text>("arial.ttf", 13.0f);
        auto TitlePos = manager->GetPosition(SkinGroup::Playing, "Title"); // conf.GetPosition("Title");
        auto RectPos = manager->GetRect(SkinGroup::Playing, "Title");      // conf.GetRect("Title");
        m_title->Position = UDim2::fromOffset(TitlePos[0].X, TitlePos[0].Y);
        m_title->AnchorPoint = { TitlePos[0].AnchorPointX, TitlePos[0].AnchorPointY };
        m_title->TextClipping = { RectPos[0].X, RectPos[0].Y, RectPos[0].Width, RectPos[0].Height };

        m_autoText = std::make_unique<UI::Text>("arial.ttf", 13.0f);
        m_autoTextSize = static_cast<int>(m_autoText->MeasureString(AUTOPLAY_TEXT).y);

        m_autoTextPos = UDim2::fromOffset(bufferRect.Width, 50);

        m_PlayBG = std::make_unique<Image>(arenaPath / ("PlayingBG.png"));
        auto PlayBGPos = manager->Arena_GetPosition("PlayingBG"); // arena_conf.GetPosition("PlayingBG");
        m_PlayBG->Position = UDim2::fromOffset(PlayBGPos[0].X, PlayBGPos[0].Y);
        m_PlayBG->AnchorPoint = { PlayBGPos[0].AnchorPointX, PlayBGPos[0].AnchorPointY };

        auto conKeyLight = manager->GetPosition(SkinGroup::Playing, "KeyLighting"); // conf.GetPosition("KeyLighting");
        auto conKeyButton = manager->GetPosition(SkinGroup::Playing, "KeyButton");  // conf.GetPosition("KeyButton");

        if (conKeyLight.size() < 7 || conKeyButton.size() < 7) {
            throw Exceptions::EstException("Playing.ini : Positions : KeyLighting#KeyButton : Not enough positions! (count < 7)");
        }

        auto playfieldPos = manager->GetPosition(SkinGroup::Playing, "Playfield"); // conf.GetPosition("Playfield");
        m_Playfield = std::make_unique<Image>(playingPath / "Playfield.png");
        m_Playfield->Position = UDim2::fromOffset(playfieldPos[0].X, playfieldPos[0].Y);
        m_Playfield->AnchorPoint = { playfieldPos[0].AnchorPointX, playfieldPos[0].AnchorPointY };

        for (int i = 0; i < 7; i++) {
            m_keyLighting[i] = std::move(std::make_unique<Image>(playingPath / ("KeyLighting" + std::to_string(i) + ".png")));
            m_keyButtons[i] = std::move(std::make_unique<Image>(playingPath / ("KeyButton" + std::to_string(i) + ".png")));

            m_keyLighting[i]->Position = UDim2::fromOffset(conKeyLight[i].X, conKeyLight[i].Y);
            m_keyButtons[i]->Position = UDim2::fromOffset(conKeyButton[i].X, conKeyButton[i].Y);
        }

        std::vector<std::filesystem::path> numComboPaths = {};
        for (int i = 0; i < 10; i++) {
            auto filePath = arenaPath / ("ComboNum" + std::to_string(i) + ".png");

            if (!std::filesystem::exists(filePath)) {
                std::cout << "Missing: " << filePath.filename() << std::endl;

                throw Exceptions::EstException("Failed to load Integer Images 0-9, please check your skin folder.");
            }

            numComboPaths.emplace_back(filePath);
        }

        m_comboNum = std::make_unique<NumberSprite>(numComboPaths);
        auto numPos = manager->Arena_GetNumeric("Combo").front(); // arena_conf.GetNumeric("Combo").front();

        m_comboNum->Position = UDim2::fromOffset(numPos.X, numPos.Y);
        m_comboNum->NumberPosition = IntToPos(numPos.Direction);
        m_comboNum->MaxDigits = numPos.MaxDigit;
        m_comboNum->FillWithZeros = numPos.FillWithZero;
        m_comboNum->AlphaBlend = true;

        std::vector<std::filesystem::path> numJamPaths = {};
        for (int i = 0; i < 10; i++) {
            auto filePath = playingPath / ("JamNum" + std::to_string(i) + ".png");

            if (!std::filesystem::exists(filePath)) {
                std::cout << "Missing: " << filePath.filename() << std::endl;

                throw Exceptions::EstException("Failed to load Integer Images 0-9, please check your skin folder.");
            }

            numJamPaths.emplace_back(filePath);
        }

        m_jamNum = std::make_unique<NumberSprite>(numJamPaths);
        numPos = manager->GetNumeric(SkinGroup::Playing, "Jam").front();

        m_jamNum->Position = UDim2::fromOffset(numPos.X, numPos.Y);
        m_jamNum->NumberPosition = IntToPos(numPos.Direction);
        m_jamNum->MaxDigits = numPos.MaxDigit;
        m_jamNum->FillWithZeros = numPos.FillWithZero;

        std::vector<std::filesystem::path> numScorePaths = {};
        for (int i = 0; i < 10; i++) {
            auto filePath = playingPath / ("ScoreNum" + std::to_string(i) + ".png");

            if (!std::filesystem::exists(filePath)) {
                std::cout << "Missing: " << filePath.filename() << std::endl;

                throw Exceptions::EstException("Failed to load Integer Images 0-9, please check your skin folder.");
            }

            numScorePaths.emplace_back(filePath);
        }

        m_scoreNum = std::make_unique<NumberSprite>(numScorePaths);
        numPos = manager->GetNumeric(SkinGroup::Playing, "Score").front(); // conf.GetNumeric("Score").front();

        m_scoreNum->Position = UDim2::fromOffset(numPos.X, numPos.Y);
        m_scoreNum->NumberPosition = IntToPos(numPos.Direction);
        m_scoreNum->MaxDigits = numPos.MaxDigit;
        m_scoreNum->FillWithZeros = numPos.FillWithZero;

        std::vector<std::string> judgeFileName = { "Miss", "Bad", "Good", "Cool" };
        auto                     judgePos = manager->Arena_GetPosition("Judge"); // arena_conf.GetPosition("Judge");
        if (judgePos.size() < 4) {
            throw Exceptions::EstException("Playing.ini : Positions : Judge : Not enough positions! (count < 4)");
        }

        for (int i = 0; i < 4; i++) {
            auto filePath = arenaPath / ("Judge" + judgeFileName[i] + ".png");

            if (!std::filesystem::exists(filePath)) {
                throw Exceptions::EstException("Failed to load Judge image!");
            }

            m_judgement[i] = std::move(std::make_unique<Image>(filePath));
            m_judgement[i]->Position = UDim2::fromOffset(judgePos[i].X, judgePos[i].Y);
            m_judgement[i]->AlphaBlend = true;
        }

        m_jamGauge = std::make_unique<Image>(playingPath / "JamGauge.png");
        auto gaugePos = manager->GetPosition(SkinGroup::Playing, "JamGauge"); // conf.GetPosition("JamGauge");
        if (gaugePos.size() < 1) {
            throw Exceptions::EstException("Playing.ini : Positions : JamGauge : Position Not defined!");
        }

        m_jamGauge->Position = UDim2::fromOffset(gaugePos[0].X, gaugePos[0].Y);
        m_jamGauge->AnchorPoint = { gaugePos[0].AnchorPointX, gaugePos[0].AnchorPointY };

        auto                               jamLogoPos = manager->GetSprite(SkinGroup::Playing, "JamLogo"); // conf.GetSprite("JamLogo");
        std::vector<std::filesystem::path> jamLogoFileName = {};
        for (int i = 0; i < jamLogoPos.numOfFrames; i++) {
            auto filePath = playingPath / ("JamLogo" + std::to_string(i) + ".png");

            if (!std::filesystem::exists(filePath)) {
                std::cout << "Missing: " << filePath.filename() << std::endl;

                throw Exceptions::EstException("Failed to load Jam Logo image!");
            }

            jamLogoFileName.emplace_back(filePath);
        }

        m_jamLogo = std::make_unique<Sprite>(jamLogoFileName, 0.25f);
        m_jamLogo->Position = UDim2::fromOffset(jamLogoPos.X, jamLogoPos.Y);
        m_jamLogo->AnchorPoint = { (double)jamLogoPos.AnchorPointX, (double)jamLogoPos.AnchorPointY };
        m_jamLogo->SetDelay(jamLogoPos.FrameTime);

        auto                               lifeBarPos = manager->GetSprite(SkinGroup::Playing, "LifeBar"); // conf.GetSprite("LifeBar");
        std::vector<std::filesystem::path> lifeBarFileName = {};
        for (int i = 0; i < lifeBarPos.numOfFrames; i++) {
            auto filePath = playingPath / ("LifeBar" + std::to_string(i) + ".png");

            if (!std::filesystem::exists(filePath)) {
                std::cout << "Missing: " << filePath.filename() << std::endl;
                throw Exceptions::EstException("Failed to load Life Bar image!");
            }

            lifeBarFileName.emplace_back(filePath);
        }

        m_lifeBar = std::make_unique<Sprite>(lifeBarFileName, 0.15f);
        m_lifeBar->Position = UDim2::fromOffset(lifeBarPos.X, lifeBarPos.Y);
        m_lifeBar->AnchorPoint = { lifeBarPos.AnchorPointX, lifeBarPos.AnchorPointY };
        m_lifeBar->SetDelay(lifeBarPos.FrameTime);

        std::vector<std::filesystem::path> lnComboFileName = {};
        for (int i = 0; i < 10; i++) {
            auto filePath = playingPath / ("LongNoteNum" + std::to_string(i) + ".png");

            if (!std::filesystem::exists(filePath)) {
                std::cout << "Missing: " << filePath.filename() << std::endl;
                throw Exceptions::EstException("Failed to load Long Note Combo image!");
            }

            lnComboFileName.emplace_back(filePath);
        }

        std::vector<std::filesystem::path> statsNumFileName = {};
        for (int i = 0; i < 10; i++) {
            auto filePath = playingPath / ("StatsNum" + std::to_string(i) + ".png");

            if (!std::filesystem::exists(filePath)) {
                std::cout << "Missing: " << filePath.filename() << std::endl;
                throw Exceptions::EstException("Failed to load Stats Number image!");
            }

            statsNumFileName.emplace_back(filePath);
        }

        m_statsNum = std::make_unique<NumberSprite>(statsNumFileName);
        auto statsNumPos = manager->GetNumeric(SkinGroup::Playing, "Stats"); // conf.GetNumeric("Stats");
        if (statsNumPos.size() < 5) {
            throw Exceptions::EstException("Playing.ini : Numerics : Stats : Not enough positions! (count < 5)");
        }

        m_statsNum->NumberPosition = IntToPos(statsNumPos[0].Direction);
        m_statsNum->MaxDigits = statsNumPos[0].MaxDigit;
        m_statsNum->FillWithZeros = statsNumPos[0].FillWithZero;
        m_statsNum->NumberPosition = IntToPos(statsNumPos[0].Direction);
        m_statsNum->AnchorPoint = { 0, .5 };

        // COOL: 0 -> MAXCOMBO: 4
        for (int i = 0; i < 5; i++) {
            m_statsPos[i] = UDim2::fromOffset(statsNumPos[i].X, statsNumPos[i].Y);
        }

        m_lnComboNum = std::make_unique<NumberSprite>(lnComboFileName);
        auto lnComboPos = manager->GetNumeric(SkinGroup::Playing, "LongNoteCombo"); // conf.GetNumeric("LongNoteCombo");
        if (lnComboPos.size() < 1) {
            throw Exceptions::EstException("Playing.ini : Numerics : LongNoteCombo : Position Not defined!");
        }

        auto btnExitPos = manager->GetPosition(SkinGroup::Playing, "ExitButton"); // conf.GetPosition("ExitButton");
        auto btnExitRect = manager->GetRect(SkinGroup::Playing, "Exit");          // conf.GetRect("Exit");

        if (btnExitPos.size() < 1 || btnExitRect.size() < 1) {
            throw Exceptions::EstException("Playing.ini : Positions|Rect : Exit : Not defined!");
        }

        m_exitBtn = std::make_unique<Image>(playingPath / "Exit.png");
        m_exitBtn->Position = UDim2::fromOffset(btnExitRect[0].X, btnExitRect[0].Y); // Fix Exit not functional with Playing.ini
        m_exitBtn->AnchorPoint = { btnExitPos[0].AnchorPointX, btnExitPos[0].AnchorPointY };

        // auto OnButtonHover = [&](int state) {
        //     m_drawExitButton = state;
        // };

        // auto OnButtonClick = [&]() {
        //     m_doExit = true;
        // };

        // m_exitButtonFunc = std::make_unique<Button>(btnExitRect[0].X, btnExitRect[0].Y, btnExitRect[0].Width, btnExitRect[0].Height);
        // m_exitButtonFunc->OnMouseClick = OnButtonClick;
        // m_exitButtonFunc->OnMouseHover = OnButtonHover;

        m_lnComboNum->Position = UDim2::fromOffset(lnComboPos[0].X, lnComboPos[0].Y);
        m_lnComboNum->NumberPosition = IntToPos(lnComboPos[0].Direction);
        m_lnComboNum->MaxDigits = lnComboPos[0].MaxDigit;
        m_lnComboNum->FillWithZeros = lnComboPos[0].FillWithZero;
        m_lnComboNum->AlphaBlend = true;

        auto                               lnLogoPos = manager->GetSprite(SkinGroup::Playing, "LongNoteLogo"); // conf.GetSprite("LongNoteLogo");
        std::vector<std::filesystem::path> lnLogoFileName = {};
        for (int i = 0; i < lnLogoPos.numOfFrames; i++) {
            auto filePath = playingPath / ("LongNoteLogo" + std::to_string(i) + ".png");

            if (!std::filesystem::exists(filePath)) {
                std::cout << "Missing: " << filePath.filename() << std::endl;
                throw Exceptions::EstException("Failed to load Long Note Logo image!");
            }

            lnLogoFileName.emplace_back(filePath);
        }

        m_lnLogo = std::make_unique<Sprite>(lnLogoFileName, 0.25f);
        m_lnLogo->Position = UDim2::fromOffset(lnLogoPos.X, lnLogoPos.Y);
        m_lnLogo->AnchorPoint = { lnLogoPos.AnchorPointX, lnLogoPos.AnchorPointY };
        m_lnLogo->SetDelay(lnLogoPos.FrameTime);
        m_lnLogo->AlphaBlend = true;

        auto                               comboLogoPos = manager->Arena_GetSprite("ComboLogo"); // arena_conf.GetSprite("ComboLogo");
        std::vector<std::filesystem::path> comboFileName = {};
        for (int i = 0; i < comboLogoPos.numOfFrames; i++) {
            auto file = arenaPath / ("ComboLogo" + std::to_string(i) + ".png");

            if (!std::filesystem::exists(file)) {
                break;
            }

            comboFileName.emplace_back(file);
        }

        m_comboLogo = std::make_unique<Sprite>(comboFileName, 0.25f);
        m_comboLogo->Position = UDim2::fromOffset(comboLogoPos.X, comboLogoPos.Y);
        m_comboLogo->AnchorPoint = { comboLogoPos.AnchorPointX, comboLogoPos.AnchorPointY };
        m_comboLogo->SetDelay(comboLogoPos.FrameTime);
        m_comboLogo->AlphaBlend = true;

        m_waveGage = std::make_unique<Image>(playingPath / "WaveGage.png");
        auto waveGagePos = manager->GetPosition(SkinGroup::Playing, "WaveGage").front(); // conf.GetPosition("WaveGage").front();
        m_waveGage->Position = UDim2::fromOffset(waveGagePos.X, waveGagePos.Y);
        m_waveGage->AnchorPoint = { waveGagePos.AnchorPointX, waveGagePos.AnchorPointY };

        std::vector<std::filesystem::path> numTimerPaths = {};
        for (int i = 0; i < 10; i++) {
            numTimerPaths.emplace_back(playingPath / ("PlayTimeNum" + std::to_string(i) + ".png"));

            if (!std::filesystem::exists(numTimerPaths.back())) {
                throw Exceptions::EstException("Failed to load Timer Images 0-9, please check your skin folder.");
            }
        }

        auto                               targetPos = manager->GetSprite(SkinGroup::Playing, "TargetBar"); // conf.GetSprite("TargetBar");
        std::vector<std::filesystem::path> targetBarPaths = {};
        for (int i = 0; i < targetPos.numOfFrames; i++) {
            auto filePath = playingPath / ("TargetBar" + std::to_string(i) + ".png");

            if (!std::filesystem::exists(filePath)) {
                throw Exceptions::EstException("Failed to load TargetBar Images, please check your skin folder.");
            }

            targetBarPaths.emplace_back(filePath);
        }

        auto playfooterPos = manager->GetPosition(SkinGroup::Playing, "Playfooter").front();
        m_Playfooter = std::make_unique<Image>(playingPath / "PlayfieldFooter.png");
        m_Playfooter->Position = UDim2::fromOffset(playfooterPos.X, playfooterPos.Y);
        m_Playfooter->AnchorPoint = { playfooterPos.AnchorPointX, playfooterPos.AnchorPointY };

        m_targetBar = std::make_unique<Sprite>(targetBarPaths);
        m_targetBar->Position = UDim2::fromOffset(targetPos.X, targetPos.Y);
        m_targetBar->AnchorPoint = { targetPos.AnchorPointX, targetPos.AnchorPointY };
        m_targetBar->SetDelay(targetPos.FrameTime);

        m_minuteNum = std::make_unique<NumberSprite>(numTimerPaths);
        auto minutePos = manager->GetNumeric(SkinGroup::Playing, "Minute"); // conf.GetNumeric("Minute");
        m_minuteNum->NumberPosition = IntToPos(minutePos[0].Direction);
        m_minuteNum->MaxDigits = minutePos[0].MaxDigit;
        m_minuteNum->FillWithZeros = minutePos[0].FillWithZero;
        m_minuteNum->Position = UDim2::fromOffset(minutePos[0].X, minutePos[0].Y);

        m_secondNum = std::make_unique<NumberSprite>(numTimerPaths);
        auto secondPos = manager->GetNumeric(SkinGroup::Playing, "Second"); // conf.GetNumeric("Second");
        m_secondNum->NumberPosition = IntToPos(secondPos[0].Direction);
        m_secondNum->MaxDigits = secondPos[0].MaxDigit;
        m_secondNum->FillWithZeros = secondPos[0].FillWithZero;
        m_secondNum->Position = UDim2::fromOffset(secondPos[0].X, secondPos[0].Y);

        auto pillsPosition = manager->GetPosition(SkinGroup::Playing, "Pill"); // conf.GetPosition("Pill");
        if (pillsPosition.size() < 5) {
            throw Exceptions::EstException("Playing.ini : Positions : Pill : Not enough positions! (count < 5)");
        }

        for (int i = 0; i < 5; i++) {
            auto file = playingPath / ("Pill" + std::to_string(i) + ".png");

            auto pos = pillsPosition[i];
            m_pills[i] = std::move(std::make_unique<Image>(file));
            m_pills[i]->Position = UDim2::fromOffset(pos.X, pos.Y);
            m_pills[i]->AnchorPoint = { pos.AnchorPointX, pos.AnchorPointY };
        }

        auto hitEffectPos = manager->Arena_GetSprite("HitEffect");   // arena_conf.GetSprite("HitEffect");
        auto holdEffectPos = manager->Arena_GetSprite("HoldEffect"); // arena_conf.GetSprite("HoldEffect");

        std::vector<std::filesystem::path> HitEffect = {};
        for (int i = 0; i < hitEffectPos.numOfFrames; i++) {
            auto path = arenaPath / ("HitEffect" + std::to_string(i) + ".png");

            if (!std::filesystem::exists(path)) {
                break;
            }

            HitEffect.emplace_back(path);
        }

        std::vector<std::filesystem::path> holdEffect = {};
        for (int i = 0; i < holdEffectPos.numOfFrames; i++) {
            auto path = arenaPath / ("HoldEffect" + std::to_string(i) + ".png");

            if (!std::filesystem::exists(path)) {
                break;
            }

            holdEffect.emplace_back(path);
        }

        {
            Chart *chart = reinterpret_cast<Chart *>(Env::GetPointer("Chart"));

            if (!chart) {
                throw Exceptions::EstException("Chart is not loaded!");
            }

            m_Engine = std::make_shared<RhythmEngine>();
            Env::SetInt("Autoplay", 1);

            RhythmGameInfo info = {};
            info.Chart = chart;

            m_Engine->SetLaneOffset(LaneOffset);
            m_Engine->SetHitPosition(HitPos);

            int idx = 2;
            try {
                idx = Configuration::GetInt("Game", "GuideLine");
            } catch (const std::invalid_argument &) {
                idx = 2;
            }

            m_Engine->SetGuideLineIndex(idx);

            auto OnTrackEvent = [&](TrackEvent e) {
                if (e.IsKeyEvent) {
                    m_keyState[e.Lane] = e.State;
                } else {
                    if (e.IsHitEvent) {
                        if (e.IsHitLongEvent) {
                            m_holdEffect[e.Lane]->Reset();
                            m_drawHit[e.Lane] = false;
                            m_drawHold[e.Lane] = e.State;

                            if (!e.State) {
                                m_hitEffect[e.Lane]->Reset();
                                m_drawHit[e.Lane] = true;
                            }
                        } else {
                            m_hitEffect[e.Lane]->Reset();
                            m_drawHit[e.Lane] = true;
                            m_drawHold[e.Lane] = false;
                        }
                    }
                }
            };

            m_Engine->ListenKeyEvent(OnTrackEvent);

            m_Engine->Load(info);

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

            Inputs::Keys keys[7] = {};
            std::string  keyName = "Lane";
            if (chart->m_keyCount != 7) {
                keyName = std::to_string(chart->m_keyCount) + "_" + keyName;
            }

            for (int i = 0; i < chart->m_keyCount; i++) {
                auto value = Configuration::Get("KeyMapping", keyName + std::to_string(i + 1));
                auto key = static_cast<Inputs::Keys>(SDL_GetScancodeFromName(value.c_str()));
                if (key == Inputs::Keys::INVALID_KEY) {
                    const char *valueStr = value.c_str();
                    throw Exceptions::EstException("Unknown key: %s, try check your keybind again!", valueStr);
                }

                keys[mappedKeyIndex[chart->m_keyCount][i]] = key;
            }

            m_Engine->SetKeys(keys);

            auto OnHitEvent = [&](NoteHitInfo info) {
                m_scoreTimer = 0;
                m_judgeTimer = 0;
                m_comboTimer = 0;
                m_judgeSize = 0.5;

                m_drawCombo = true;
                m_drawJudge = true;

                m_comboTimer = 0;
                m_comboLogo->Reset();
                m_judgeIndex = (int)info.Result;
            };

            m_Engine->GetScoreManager()->ListenHit(OnHitEvent);

            auto OnJamEvent = [&](int combo) {
                m_drawJam = true;
                m_jamTimer = 0;
                m_jamLogo->Reset();
            };

            m_Engine->GetScoreManager()->ListenJam(OnJamEvent);

            auto OnLongComboEvent = [&] {
                m_lnTimer = 0;
                m_drawLN = true;
            };

            m_Engine->GetScoreManager()->ListenLongNote(OnLongComboEvent);
        }

        auto lanePos = m_Engine->GetLanePos();
        auto laneSize = m_Engine->GetLaneSizes();
        for (int i = 0; i < 7; i++) {
            m_hitEffect[i] = std::move(std::make_unique<Sprite>(HitEffect));
            m_holdEffect[i] = std::move(std::make_unique<Sprite>(holdEffect));

            m_hitEffect[i]->SetDelay(hitEffectPos.FrameTime);
            m_holdEffect[i]->SetDelay(holdEffectPos.FrameTime);
            m_hitEffect[i]->ResetLast();
            m_hitEffect[i]->Repeat = false;
            m_hitEffect[i]->AlphaBlend = true;
            m_holdEffect[i]->AlphaBlend = true;

            float pos = std::ceil(lanePos[i] + (laneSize[i] / 2.0f));
            auto  hitPos = UDim2::fromOffset(pos, HitPos - 15) + UDim2::fromOffset(hitEffectPos.X, hitEffectPos.Y);
            auto  holdPos = UDim2::fromOffset(pos, HitPos - 15) + UDim2::fromOffset(holdEffectPos.X, holdEffectPos.Y);

            m_hitEffect[i]->Position = hitPos;   // -15 or it will too Deep
            m_holdEffect[i]->Position = holdPos; // -15 or it will too Deep
            m_hitEffect[i]->AnchorPoint = { hitEffectPos.AnchorPointX, hitEffectPos.AnchorPointY };
            m_holdEffect[i]->AnchorPoint = { holdEffectPos.AnchorPointX, holdEffectPos.AnchorPointY };
        }

        bool IsHD = Env::GetBool("Hidden");
        bool IsFL = Env::GetBool("Flashlight");
        if (IsHD || IsFL) {
            std::vector<Segment> segments;

            if (IsFL) {
                segments = {
                    { 0.00f, 0.20f, { 0, 0, 0, 255 }, { 0, 0, 0, 255 } },
                    { 0.33f, 0.38f, { 0, 0, 0, 255 }, { 0, 0, 0, 0 } },
                    { 0.38f, 0.61f, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
                    { 0.61f, 0.67f, { 0, 0, 0, 0 }, { 0, 0, 0, 255 } },
                    { 0.67f, 1.00f, { 0, 0, 0, 255 }, { 0, 0, 0, 255 } }
                };
            } else {
                segments = {
                    { 0.00f, 0.00f, { 0, 0, 0, 255 }, { 0, 0, 0, 255 } },
                    { 0.00f, 0.00f, { 0, 0, 0, 255 }, { 0, 0, 0, 0 } },
                    { 0.00f, 0.45f, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
                    { 0.45f, 0.50f, { 0, 0, 0, 0 }, { 0, 0, 0, 255 } },
                    { 0.50f, 1.00f, { 0, 0, 0, 255 }, { 0, 0, 0, 255 } }
                };
            }

            int   imageWidth = static_cast<int>(accumulate(laneSize, laneSize + 7, 0));
            int   imageHeight = HitPos;
            float imagePos = lanePos[0];

            std::vector<uint8_t> imageBuffer = ImageGenerator::GenerateGradientImage(imageWidth, imageHeight, segments);

            m_laneHideImage = std::make_unique<Image>((const char *)imageBuffer.data(), imageBuffer.size());
            m_laneHideImage->Position = UDim2::fromOffset(imagePos, 0);
            m_laneHideImage->Size = UDim2::fromOffset(imageWidth, imageHeight);
        }

    } catch (const Exceptions::EstException &e) {
        MsgBox::Show("Error", e.what(), MsgBox::Type::Ok, MsgBox::Flags::Error);
        m_resourceFucked = true;
    }

    MsgBox::InShow("msg_id", "Hello title", "Hello world");
    return true;
}

bool Gameplay::Detach()
{
    m_Engine.reset();
    return true;
}