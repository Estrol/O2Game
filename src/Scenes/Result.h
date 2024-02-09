/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#pragma once

#include <Math/Tween.h>
#include <Screens/Base.h>
#include <UI/Image.h>
#include <UI/Rectangle.h>
#include <UI/Text.h>

#include "../Game/Core/Drawable/ButtonImage.h"
#include "../Game/Core/Drawable/Image.h"
#include "../Game/Core/Drawable/NumberSprite.h"

struct ScoreInfo
{
    int  score;
    int  cool;
    int  good;
    int  bad;
    int  miss;
    int  maxJam;
    int  maxCombo;
    bool isClear;
};

class Result : public Screens::Base
{
public:
    Result();
    ~Result();

    void Update(double delta) override;
    void Draw(double delta) override;
    void Input(double delta) override;

    bool Attach() override;
    bool Detach() override;

private:
    bool      m_IsAttached;
    ScoreInfo m_ScoreInfo;

    std::shared_ptr<ButtonImage> m_BackButton;

    std::shared_ptr<Image> m_Result;
    std::shared_ptr<Image> m_Lose;
    std::shared_ptr<Image> m_Win;

    std::shared_ptr<NumberSprite> m_Score;
    std::shared_ptr<NumberSprite> m_Stats;

    std::map<int, UDim2>   m_StatsPosition;
    std::map<int, Vector2> m_StatsAnchor;

    std::shared_ptr<UI::Text> m_Text;
};