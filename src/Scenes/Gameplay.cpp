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
#include <Misc/Lodepng.h>

#include "SceneList.h"
#include <random>

#include "../Game/Data/chart.hpp"
#include "../Game/Data/ojn.h"

#include "../Game/Core/Skinning/LuaSkin.h"
#include "../Game/Env.h"

#include "../Game/Core/Drawable/ImageGenerators.h"
#include "../Game/Data/Util/Util.hpp"

#include "../Game/MsgBoxEx.h"
#include <Configuration.h>
#include <Logs.h>
#include <MsgBox.h>

#include <Audio/AudioEngine.h>
#include <Audio/AudioSample.h>

#include <Math/Tween.h>

#include <Exceptions/EstException.h>

#include "../Game/Core/Audio/SampleManager.h"
#include <Screens/ScreenManager.h>

#define AUTOPLAY_TEXT (const char *)u8"Game currently on autoplay!"

void Gameplay::Update(double delta)
{
    if (m_resourceFucked) {
        return;
    }

    if (m_ended) {
        if (!m_frameCaptured) {
            m_frameCaptured = true;
            SampleManager::StopAll();

            auto scoreManager = m_Engine->GetScoreManager();
            auto scoreInfo = scoreManager->GetScore();

            Env::SetInt("Score", scoreInfo.Score);
            Env::SetInt("Cool", scoreInfo.Cool);
            Env::SetInt("Good", scoreInfo.Good);
            Env::SetInt("Bad", scoreInfo.Bad);
            Env::SetInt("Miss", scoreInfo.Miss);
            Env::SetInt("MaxJam", scoreInfo.MaxJamCombo);
            Env::SetInt("MaxCombo", scoreInfo.MaxJamCombo);
            Env::SetBool("IsClear", true);
        } else {
            m_ResultTopRowImage->Position = m_TweenTop->Update((float)delta);
            m_ResultBottomRowImage->Position = m_TweenBottom->Update((float)delta);

            if (m_TweenBottom->IsFinished() && m_TweenTop->IsFinished()) {
                Screens::Manager::Get()->SetScreen(SceneList::RESULT);
            }
        }
    } else {
        m_Engine->Update(delta);

        m_startTime += delta;
        if (m_startTime >= 1.5 && !m_starting) {
            m_starting = true;
            m_Engine->Start();
        }
    }
}

void Gameplay::Draw(double delta)
{
    if (m_resourceFucked) {
        return;
    }

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

    if (m_ended) {
        m_ResultTopRowImage->Draw();
        m_ResultBottomRowImage->Draw();
    }

    Graphics::Renderer::Get()->ImGui_NewFrame();

    // ImGui::ShowDemoWindow();

    Graphics::Renderer::Get()->ImGui_EndFrame();
}

void Gameplay::FixedUpdate(double fixedDelta)
{
    auto &scores = m_Engine->GetScoreManager()->GetScore();

    if (m_drawJudge) {
        m_judgeSize = std::clamp(m_judgeSize + (fixedDelta * 6), 0.5, 1.0); // Nice
        if ((m_judgeTimer += fixedDelta) > 0.60) {
            m_drawJudge = false;
        }
    }

    if (m_drawCombo && scores.Combo > 0) {
        m_comboTimer += fixedDelta;
    }
}

void Gameplay::Input(double delta)
{
}

void Gameplay::OnKeyDown(const Inputs::State &state)
{
    if (state.Keyboard.Key == Inputs::Keys::F9 || state.Keyboard.Key == Inputs::Keys::PrintScreen) {
        auto manager = Screens::Manager::Get();

        Graphics::Renderer::Get()->CaptureFrame([=](std::vector<unsigned char> image_data) {
            auto rect = Graphics::NativeWindow::Get()->GetWindowSize();

            std::string           filename = "screenshot_" + std::to_string(std::time(nullptr)) + ".png";
            std::filesystem::path ssdir = std::filesystem::current_path() / "Screenshots";

            if (!std::filesystem::exists(ssdir)) {
                std::filesystem::create_directory(ssdir);
            }

            std::filesystem::path ssfile = ssdir / filename;
            lodepng::encode(
                ssfile.string(),
                image_data,
                rect.Width,
                rect.Height);
        });

        return;
    }

    if (state.Keyboard.Key == Inputs::Keys::F10) {
        m_ended = true;

        return;
    }

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
    m_frameCaptured = false;
    m_doExit = false;
    m_drawExitButton = false;
    m_resourceFucked = false;
    m_drawJudge = false;

    m_startTime = 0.0f;

    try {
        auto   bufferRect = Graphics::NativeWindow::Get()->GetBufferSize();
        Chart *chart = reinterpret_cast<Chart *>(Env::GetPointer("Chart"));

        if (!chart) {
            throw Exceptions::EstException("Chart is not loaded!");
        }

        auto manager = LuaSkin::Get();

        int LaneOffset = 5;
        int HitPos = 480;

        try {
            LaneOffset = std::stoi(manager->GetSkinProp("Game", "LaneOffset", "5"));
            HitPos = std::stoi(manager->GetSkinProp("Game", "HitPos", "480"));
        } catch (const std::invalid_argument &) {
            throw Exceptions::EstException("Invalid parameter on Skin::Game::LaneOffset or Skin::Game::HitPos");
        }

        int arena = Env::GetInt("Arena");
        if (arena == 0) {
            std::random_device dev;
            std::mt19937       rng(dev());

            std::uniform_int_distribution<> dist(1, 12);

            arena = dist(rng);
        }

        Env::SetInt("KeyCount", chart->m_keyCount);
        Env::SetInt("CurrentArena", arena);

        {
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

            m_Engine->ListenKeyEvent([=](TrackEvent e) { OnTrackEvent(e); });

            m_Engine->Load(info);
        }

        /**
         * Begin playing scene group
         */

        manager->LoadScript(SkinGroup::Playing);

        for (int i = 0; i < 7; i++) {
            m_keyState[i] = false;
            m_drawHit[i] = false;
            m_drawHold[i] = false;
        }

        m_title = std::make_unique<UI::Text>("arial.ttf", 13.0f);
        auto TitlePos = manager->GetPosition("Title").front();
        auto RectPos = manager->GetRect("Title");
        m_title->Position = TitlePos.Position;
        m_title->AnchorPoint = TitlePos.AnchorPoint;
        m_title->TextClipping = {
            (int)RectPos[0].Position.X.Offset,
            (int)RectPos[0].Position.Y.Offset,
            (int)RectPos[0].Size.X.Offset,
            (int)RectPos[0].Size.Y.Offset
        };

        m_autoText = std::make_unique<UI::Text>("arial.ttf", 13.0f);
        m_autoTextSize = static_cast<int>(m_autoText->MeasureString(AUTOPLAY_TEXT).y);

        m_autoTextPos = UDim2::fromOffset(bufferRect.Width, 50);

        auto playfieldPos = manager->GetPosition("Playfield").front();
        m_Playfield = std::make_unique<Image>(playfieldPos.Path);
        m_Playfield->Position = playfieldPos.Position;
        m_Playfield->Size = playfieldPos.Size;
        m_Playfield->AnchorPoint = playfieldPos.AnchorPoint;

        auto conKeyLight = manager->GetPosition("KeyLighting");
        auto conKeyButton = manager->GetPosition("KeyButton");

        if (conKeyLight.size() < 7 || conKeyButton.size() < 7) {
            throw Exceptions::EstException("Playing.ini : Positions : KeyLighting#KeyButton : Not enough positions! (count < 7)");
        }

        for (int i = 0; i < 7; i++) {
            m_keyLighting[i] = std::move(std::make_unique<Image>(conKeyLight[i].Path));
            m_keyButtons[i] = std::move(std::make_unique<Image>(conKeyButton[i].Path));

            m_keyLighting[i]->Position = conKeyLight[i].Position;
            m_keyButtons[i]->Position = conKeyButton[i].Position;

            m_keyLighting[i]->Size = conKeyLight[i].Size;
            m_keyButtons[i]->Size = conKeyButton[i].Size;
        }

        auto jamNumPos = manager->GetNumeric("Jam").front();
        m_jamNum = std::make_unique<NumberSprite>(jamNumPos.Files);

        m_jamNum->Position = jamNumPos.Position;
        m_jamNum->Size = jamNumPos.Size;
        m_jamNum->NumberPosition = IntToPos(jamNumPos.Direction);
        m_jamNum->MaxDigits = jamNumPos.MaxDigit;
        m_jamNum->Color = jamNumPos.Color;
        m_jamNum->FillWithZeros = jamNumPos.FillWithZero;

        auto scoreNumPos = manager->GetNumeric("Score").front(); // conf.GetNumeric("Score").front();
        m_scoreNum = std::make_unique<NumberSprite>(scoreNumPos.Files);

        m_scoreNum->Position = scoreNumPos.Position;
        m_scoreNum->Size = scoreNumPos.Size;
        m_scoreNum->NumberPosition = IntToPos(scoreNumPos.Direction);
        m_scoreNum->MaxDigits = scoreNumPos.MaxDigit;
        m_scoreNum->Color = scoreNumPos.Color;
        m_scoreNum->FillWithZeros = scoreNumPos.FillWithZero;

        auto gaugePos = manager->GetPosition("JamGauge").front();
        m_jamGauge = std::make_unique<Image>(gaugePos.Path);

        m_jamGauge->Position = gaugePos.Position;
        m_jamGauge->Size = gaugePos.Size;
        m_jamGauge->Color3 = gaugePos.Color;
        m_jamGauge->AnchorPoint = gaugePos.AnchorPoint;

        auto jamLogoPos = manager->GetSprite("JamLogo");

        m_jamLogo = std::make_unique<Sprite>(jamLogoPos.Files, jamLogoPos.FrameTime);
        m_jamLogo->Position = jamLogoPos.Position;
        m_jamLogo->Size = jamLogoPos.Size;
        m_jamLogo->AnchorPoint = jamLogoPos.AnchorPoint;

        auto lifeBarPos = manager->GetSprite("LifeBar");

        m_lifeBar = std::make_unique<Sprite>(lifeBarPos.Files, lifeBarPos.FrameTime);
        m_lifeBar->Position = lifeBarPos.Position;
        m_lifeBar->Size = lifeBarPos.Size;
        m_lifeBar->AnchorPoint = lifeBarPos.AnchorPoint;

        auto statsNumPos = manager->GetNumeric("Stats"); // conf.GetNumeric("Stats");
        if (statsNumPos.size() < 5) {
            throw Exceptions::EstException("Playing.ini : Numerics : Stats : Not enough positions! (count < 5)");
        }

        m_statsNum = std::make_unique<NumberSprite>(statsNumPos[0].Files);

        m_statsNum->Size = statsNumPos[0].Size;
        m_statsNum->NumberPosition = IntToPos(statsNumPos[0].Direction);
        m_statsNum->MaxDigits = statsNumPos[0].MaxDigit;
        m_statsNum->FillWithZeros = statsNumPos[0].FillWithZero;
        m_statsNum->NumberPosition = IntToPos(statsNumPos[0].Direction);
        m_statsNum->AnchorPoint = { 0, .5 };

        // COOL: 0 -> MAXCOMBO: 4
        for (int i = 0; i < 5; i++) {
            m_statsPos[i] = statsNumPos[i].Position;
        }

        auto lnComboPos = manager->GetNumeric("LongNoteCombo").front();
        m_lnComboNum = std::make_unique<NumberSprite>(lnComboPos.Files);

        m_lnComboNum->Position = lnComboPos.Position;
        m_lnComboNum->NumberPosition = IntToPos(lnComboPos.Direction);
        m_lnComboNum->MaxDigits = lnComboPos.MaxDigit;
        m_lnComboNum->FillWithZeros = lnComboPos.FillWithZero;
        m_lnComboNum->AlphaBlend = true;
        m_lnComboNum->Size = lnComboPos.Size;

        auto btnExitPos = manager->GetPosition("ExitButton").front();
        auto btnExitRect = manager->GetRect("Exit");

        m_exitBtn = std::make_unique<Image>(btnExitPos.Path);
        m_exitBtn->Position = btnExitPos.Position; // Fix Exit not functional with Playing.ini
        m_exitBtn->Size = btnExitPos.Size;
        m_exitBtn->AnchorPoint = btnExitPos.AnchorPoint;

        auto lnLogoPos = manager->GetSprite("LongNoteLogo"); // conf.GetSprite("LongNoteLogo");

        m_lnLogo = std::make_unique<Sprite>(lnLogoPos.Files, lnLogoPos.FrameTime);
        m_lnLogo->Position = lnLogoPos.Position;
        m_lnLogo->Size = lnLogoPos.Size;
        m_lnLogo->AnchorPoint = lnLogoPos.AnchorPoint;
        m_lnLogo->TintColor = lnLogoPos.Color;
        m_lnLogo->AlphaBlend = true;

        auto waveGagePos = manager->GetPosition("WaveGage").front();
        m_waveGage = std::make_unique<Image>(waveGagePos.Path);
        m_waveGage->Position = waveGagePos.Position;
        m_waveGage->Size = waveGagePos.Size;
        m_waveGage->AnchorPoint = waveGagePos.AnchorPoint;
        m_waveGage->Color3 = waveGagePos.Color;

        auto playfooterPos = manager->GetPosition("Playfooter").front();
        m_Playfooter = std::make_unique<Image>(playfooterPos.Path);
        m_Playfooter->Position = playfooterPos.Position;
        m_Playfooter->Size = playfooterPos.Size;
        m_Playfooter->AnchorPoint = playfooterPos.AnchorPoint;
        m_Playfooter->Color3 = playfooterPos.Color;

        auto targetPos = manager->GetSprite("TargetBar");
        m_targetBar = std::make_unique<Sprite>(targetPos.Files, targetPos.FrameTime);
        m_targetBar->Position = targetPos.Position;
        m_targetBar->Size = targetPos.Size;
        m_targetBar->TintColor = targetPos.Color;
        m_targetBar->AnchorPoint = targetPos.AnchorPoint;

        auto minutePos = manager->GetNumeric("Minute").front();
        m_minuteNum = std::make_unique<NumberSprite>(minutePos.Files);
        m_minuteNum->NumberPosition = IntToPos(minutePos.Direction);
        m_minuteNum->MaxDigits = minutePos.MaxDigit;
        m_minuteNum->FillWithZeros = minutePos.FillWithZero;
        m_minuteNum->Position = minutePos.Position;
        m_minuteNum->Size = minutePos.Size;
        m_minuteNum->Color = minutePos.Color;

        auto secondPos = manager->GetNumeric("Second").front();
        m_secondNum = std::make_unique<NumberSprite>(secondPos.Files);
        m_secondNum->NumberPosition = IntToPos(secondPos.Direction);
        m_secondNum->MaxDigits = secondPos.MaxDigit;
        m_secondNum->FillWithZeros = secondPos.FillWithZero;
        m_secondNum->Position = secondPos.Position;
        m_secondNum->Size = secondPos.Size;
        m_secondNum->Color = secondPos.Color;

        auto pillsPosition = manager->GetPosition("Pill");
        if (pillsPosition.size() < 5) {
            throw Exceptions::EstException("Playing.ini : Positions : Pill : Not enough positions! (count < 5)");
        }

        for (int i = 0; i < 5; i++) {
            auto pos = pillsPosition[i];
            m_pills[i] = std::move(std::make_unique<Image>(pos.Path));
            m_pills[i]->Position = pos.Position;
            m_pills[i]->Size = pos.Size;
            m_pills[i]->AnchorPoint = pos.AnchorPoint;
            m_pills[i]->Color3 = pos.Color;
        }

        auto topRowImagePos = manager->GetPosition("TopRow").front();
        m_ResultTopRowImage = std::make_unique<Image>(topRowImagePos.Path);
        m_ResultTopRowImage->Position = topRowImagePos.Position;
        m_ResultTopRowImage->Size = topRowImagePos.Size;
        m_ResultTopRowImage->AnchorPoint = topRowImagePos.AnchorPoint;
        m_ResultTopRowImage->Color3 = topRowImagePos.Color;

        auto bottomRowImagePos = manager->GetPosition("BottomRow").front();
        m_ResultBottomRowImage = std::make_unique<Image>(bottomRowImagePos.Path);
        m_ResultBottomRowImage->Position = bottomRowImagePos.Position;
        m_ResultBottomRowImage->Size = bottomRowImagePos.Size;
        m_ResultBottomRowImage->AnchorPoint = bottomRowImagePos.AnchorPoint;
        m_ResultBottomRowImage->Color3 = bottomRowImagePos.Color;

        auto topRowTween = manager->GetTween("TopRow");
        m_TweenTop = std::make_shared<Tween>(
            m_ResultTopRowImage->Position,
            topRowTween.Destination,
            topRowTween.Duration,
            topRowTween.Type);

        auto bottomRowTween = manager->GetTween("BottomRow");
        m_TweenBottom = std::make_shared<Tween>(
            m_ResultBottomRowImage->Position,
            bottomRowTween.Destination,
            bottomRowTween.Duration,
            bottomRowTween.Type);

        /**
         * End playing scene group
         * Begin arena scene group
         */

        manager->LoadScript(SkinGroup::Arena);

        // TODO: Arena
        std::vector<std::string> judgeFileName = { "Miss", "Bad", "Good", "Cool" };
        auto                     judgePos = manager->GetPosition("Judge");
        if (judgePos.size() < 4) {
            throw Exceptions::EstException("Playing.ini : Positions : Judge : Not enough positions! (count < 4)");
        }

        for (int i = 0; i < 4; i++) {
            auto file = judgePos[i];

            if (!std::filesystem::exists(file.Path)) {
                throw Exceptions::EstException("Failed to load Judge image!");
            }

            m_judgement[i] = std::move(std::make_unique<Image>(file.Path));
            m_judgement[i]->Position = judgePos[i].Position;
            m_judgement[i]->Size = judgePos[i].Size;
            m_judgement[i]->AnchorPoint = judgePos[i].AnchorPoint;
            m_judgement[i]->Color3 = judgePos[i].Color;
            m_judgement[i]->AlphaBlend = true;
        }

        // TODO: Arena
        auto comboLogoPos = manager->GetSprite("ComboLogo");

        m_comboLogo = std::make_unique<Sprite>(comboLogoPos.Files, comboLogoPos.FrameTime);
        m_comboLogo->Position = comboLogoPos.Position;
        m_comboLogo->Size = comboLogoPos.Size;
        m_comboLogo->TintColor = comboLogoPos.Color;
        m_comboLogo->AnchorPoint = comboLogoPos.AnchorPoint;
        m_comboLogo->AlphaBlend = true;

        // TODO: Arena
        auto playingBgPos = manager->GetPosition("PlayingBG").front();

        m_PlayBG = std::make_unique<Image>(playingBgPos.Path);
        m_PlayBG->Position = playingBgPos.Position;
        m_PlayBG->Size = playingBgPos.Size;
        m_PlayBG->Color3 = playingBgPos.Color;
        m_PlayBG->AnchorPoint = playingBgPos.AnchorPoint;

        // TODO: Arena
        auto numPos = manager->GetNumeric("Combo").front();
        m_comboNum = std::make_unique<NumberSprite>(numPos.Files);

        m_comboNum->Position = numPos.Position;
        m_comboNum->Size = numPos.Size;
        m_comboNum->Color = numPos.Color;
        m_comboNum->NumberPosition = IntToPos(numPos.Direction);
        m_comboNum->MaxDigits = numPos.MaxDigit;
        m_comboNum->FillWithZeros = numPos.FillWithZero;
        m_comboNum->AlphaBlend = true;

        {
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

        auto hitEffectPos = manager->GetSprite("HitEffect");
        auto holdEffectPos = manager->GetSprite("HoldEffect");

        auto lanePos = m_Engine->GetLanePos();
        auto laneSize = m_Engine->GetLaneSizes();
        for (int i = 0; i < 7; i++) {
            m_hitEffect[i] = std::move(std::make_unique<Sprite>(hitEffectPos.Files, hitEffectPos.FrameTime));
            m_holdEffect[i] = std::move(std::make_unique<Sprite>(holdEffectPos.Files, holdEffectPos.FrameTime));

            m_hitEffect[i]->SetDelay(hitEffectPos.FrameTime);
            m_holdEffect[i]->SetDelay(holdEffectPos.FrameTime);
            m_hitEffect[i]->ResetLast();
            m_hitEffect[i]->Repeat = false;
            m_hitEffect[i]->AlphaBlend = true;
            m_holdEffect[i]->AlphaBlend = true;

            float pos = std::ceil(lanePos[i] + (laneSize[i] / 2.0f));
            auto  hitPos = UDim2::fromOffset(pos, HitPos - 15) + hitEffectPos.Position;
            auto  holdPos = UDim2::fromOffset(pos, HitPos - 15) + holdEffectPos.Position;

            m_hitEffect[i]->Position = hitPos;   // -15 or it will too Deep
            m_holdEffect[i]->Position = holdPos; // -15 or it will too Deep
            m_hitEffect[i]->AnchorPoint = hitEffectPos.AnchorPoint;
            m_holdEffect[i]->AnchorPoint = holdEffectPos.AnchorPoint;
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

    return true;
}

bool Gameplay::Detach()
{
    m_Engine.reset();
    return true;
}

void Gameplay::OnTrackEvent(TrackEvent e)
{
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
}