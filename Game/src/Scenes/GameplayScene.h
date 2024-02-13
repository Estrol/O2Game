#pragma once
#include "Scene.h"
#include "Texture/NumericTexture.h"
#include "Texture/Sprite2D.h"
#include "Texture/Text.h"

#include "../Engine/DrawableNote.hpp"
#include "../Engine/FrameTimer.hpp"
#include "../Engine/RhythmEngine.hpp"

#include "../Engine/Button.hpp"
#include "../ManiaKeys.h"

struct ManiaKeyState
{
    Keys key;
    bool isPressed;
};

class GameplayScene : public Scene
{
public:
    GameplayScene();

    void Update(double delta) override;
    void Render(double delta) override;
    void Input(double delta) override;

    void OnKeyDown(const KeyState &state) override;
    void OnKeyUp(const KeyState &state) override;
    void OnMouseDown(const MouseState &state) override;

    bool Attach() override;
    bool Detach() override;

private:
    void *CreateScreenshotWin32();

    std::unordered_map<int, std::shared_ptr<Texture2D>>  m_keyLighting;
    std::unordered_map<int, std::shared_ptr<Texture2D>>  m_keyButtons;
    std::unordered_map<int, std::shared_ptr<Texture2D>>  m_judgement;
    std::unordered_map<int, std::shared_ptr<Texture2D>>  m_pills;
    std::unordered_map<int, std::shared_ptr<FrameTimer>> m_hitEffect;
    std::unordered_map<int, std::shared_ptr<FrameTimer>> m_holdEffect;
    std::unordered_map<int, bool>                        m_keyState;
    std::unordered_map<int, UDim2>                       m_statsPos;

    std::unique_ptr<Button>    m_exitButtonFunc;
    std::unique_ptr<Texture2D> m_exitBtn;
    std::unique_ptr<Texture2D> m_Playfield;
    std::unique_ptr<Texture2D> m_PlayBG;
    std::unique_ptr<Texture2D> m_Playfooter;
    std::unique_ptr<Texture2D> m_noteMod;
    std::unique_ptr<Texture2D> m_visualMod;

    std::unique_ptr<Texture2D> m_jamGauge;
    std::unique_ptr<Texture2D> m_waveGage;

    std::unique_ptr<Sprite2D> m_jamLogo;
    std::unique_ptr<Sprite2D> m_lifeBar;
    std::unique_ptr<Sprite2D> m_lnLogo;
    std::unique_ptr<Sprite2D> m_comboLogo;
    std::unique_ptr<Sprite2D> m_targetBar;

    std::unique_ptr<NumericTexture> m_statsNum;
    std::unique_ptr<NumericTexture> m_lnComboNum;
    std::unique_ptr<NumericTexture> m_jamNum;
    std::unique_ptr<NumericTexture> m_scoreNum;
    std::unique_ptr<NumericTexture> m_comboNum;
    std::unique_ptr<NumericTexture> m_minuteNum;
    std::unique_ptr<NumericTexture> m_secondNum;

    std::unique_ptr<Text> m_title;
    std::unique_ptr<Text> m_autoText;

    std::unique_ptr<Texture2D> m_laneHideImage;

    std::unique_ptr<RhythmEngine> m_game;

    bool m_resourceFucked;
    bool m_starting;
    bool m_ended;
    int  m_judgeIndex;

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
    double m_position;

    /* button */
    bool m_drawExitButton;
    bool m_doExit;

    /* auto text size */
    bool  m_autoPlay;
    int   m_autoTextSize;
    UDim2 m_autoTextPos;
};