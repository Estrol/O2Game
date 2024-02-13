/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#pragma once
#include "../Game/Core/Drawable/Image.h"
#include "../Game/Core/Drawable/NumberSprite.h"
#include "../Game/Core/Drawable/Sprite.h"
#include "../Game/Core/RhythmEngine.h"
#include <Graphics/SpriteBatch.h>
#include <Math/Tween.h>
#include <Screens/Base.h>
#include <UI/Image.h>
#include <UI/Text.h>
#include <memory>

class Gameplay : public Screens::Base
{
public:
    void Update(double delta) override;
    void Draw(double delta) override;
    void Input(double delta) override;
    void FixedUpdate(double fixedDelta) override;

    void OnKeyDown(const Inputs::State &state) override;
    void OnKeyUp(const Inputs::State &state) override;

    bool Attach() override;
    bool Detach() override;

private:
    std::shared_ptr<RhythmEngine> m_Engine;

    std::unique_ptr<UI::Text> m_title, m_autoText;

    std::unique_ptr<NumberSprite> m_statsNum;
    std::unique_ptr<NumberSprite> m_lnComboNum;
    std::unique_ptr<NumberSprite> m_jamNum;
    std::unique_ptr<NumberSprite> m_scoreNum;
    std::unique_ptr<NumberSprite> m_comboNum;
    std::unique_ptr<NumberSprite> m_minuteNum;
    std::unique_ptr<NumberSprite> m_secondNum;

    std::shared_ptr<Graphics::SpriteBatch> m_noteSpriteBatch;
    std::shared_ptr<Graphics::SpriteBatch> m_holdSpriteBatch;
    std::shared_ptr<Graphics::SpriteBatch> m_measureSpriteBatch;

    std::unordered_map<int, std::shared_ptr<NumberSprite>> m_statsNums;
    std::unordered_map<int, std::shared_ptr<Image>>        m_keyLighting;
    std::unordered_map<int, std::shared_ptr<Image>>        m_keyButtons;
    std::unordered_map<int, std::shared_ptr<Image>>        m_judgement;
    std::unordered_map<int, std::shared_ptr<Image>>        m_pills;
    std::unordered_map<int, std::shared_ptr<Sprite>>       m_hitEffect;
    std::unordered_map<int, std::shared_ptr<Sprite>>       m_holdEffect;
    std::unordered_map<int, bool>                          m_keyState;
    std::unordered_map<int, UDim2>                         m_statsPos;

    std::unique_ptr<Image> m_exitBtn;
    std::unique_ptr<Image> m_Playfield;
    std::unique_ptr<Image> m_PlayBG;
    std::unique_ptr<Image> m_Playfooter;
    std::unique_ptr<Image> m_laneHideImage;

    std::unique_ptr<Image> m_jamGauge;
    std::unique_ptr<Image> m_waveGage;

    std::unique_ptr<Sprite> m_jamLogo;
    std::unique_ptr<Sprite> m_lifeBar;
    std::unique_ptr<Sprite> m_lnLogo;
    std::unique_ptr<Sprite> m_comboLogo;
    std::unique_ptr<Sprite> m_targetBar;

    std::shared_ptr<Image> m_ResultTopRowImage;
    std::shared_ptr<Image> m_ResultBottomRowImage;

    std::shared_ptr<Tween> m_TweenTop;
    std::shared_ptr<Tween> m_TweenBottom;

    bool m_resourceFucked;
    bool m_starting;
    bool m_frameCaptured;
    bool m_ended;
    bool m_usingFLHD;
    int  m_judgeIndex;

    double m_startTime;

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
    double m_amplitude;

    /* button */
    bool m_drawExitButton;
    bool m_doExit;

    /* auto text size */
    bool  m_autoPlay;
    int   m_autoTextSize;
    UDim2 m_autoTextPos;

    /* RhythmEngine callback */
    void OnTrackEvent(TrackEvent e);
};
