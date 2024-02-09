#include "Result.h"

#include "../Game/Core/Skinning/LuaSkin.h"
#include "../Game/Env.h"

#include <MsgBox.h>

Result::Result()
{
    m_IsAttached = false;
}

Result::~Result()
{
}

void Result::Update(double delta)
{
    (void)delta;
}

void Result::Draw(double delta)
{
    if (!m_IsAttached) {
        return;
    }

    m_Result->Draw();

    m_Score->Draw(m_ScoreInfo.score);

    if (m_ScoreInfo.isClear) {
        m_Win->Draw();
    } else {
        m_Lose->Draw();
    }

    m_BackButton->Draw();

    m_Stats->Position = m_StatsPosition[0];
    m_Stats->AnchorPoint = m_StatsAnchor[0];
    m_Stats->Draw(m_ScoreInfo.cool);

    m_Stats->Position = m_StatsPosition[1];
    m_Stats->AnchorPoint = m_StatsAnchor[1];
    m_Stats->Draw(m_ScoreInfo.good);

    m_Stats->Position = m_StatsPosition[2];
    m_Stats->AnchorPoint = m_StatsAnchor[2];
    m_Stats->Draw(m_ScoreInfo.bad);

    m_Stats->Position = m_StatsPosition[3];
    m_Stats->AnchorPoint = m_StatsAnchor[3];
    m_Stats->Draw(m_ScoreInfo.miss);

    m_Stats->Position = m_StatsPosition[4];
    m_Stats->AnchorPoint = m_StatsAnchor[4];
    m_Stats->Draw(m_ScoreInfo.maxCombo);

    m_Stats->Position = m_StatsPosition[5];
    m_Stats->AnchorPoint = m_StatsAnchor[5];
    m_Stats->Draw(m_ScoreInfo.maxJam);

    (void)delta;
}

void Result::Input(double delta)
{
    if (!m_IsAttached) {
        return;
    }

    if (m_BackButton->UpdateInput()) {
        return;
    }

    (void)delta;
}

bool Result::Attach()
{
    auto manager = LuaSkin::Get();
    manager->LoadScript(SkinGroup::Result);

    auto resultPos = manager->GetPosition("Result").front();
    m_Result = std::make_shared<Image>(resultPos.Path);
    m_Result->Position = resultPos.Position;
    m_Result->Size = resultPos.Size;
    m_Result->AnchorPoint = resultPos.AnchorPoint;
    m_Result->Color3 = resultPos.Color;

    auto scorePos = manager->GetPosition("Score").front();
    auto scoreImgs = manager->GetNumeric("Score").front();
    m_Score = std::make_shared<NumberSprite>(scoreImgs.Files);
    m_Score->Position = scorePos.Position;
    m_Score->Size = scorePos.Size;
    m_Score->AnchorPoint = scorePos.AnchorPoint;
    m_Score->NumberPosition = IntToPos(scoreImgs.Direction);
    m_Score->FillWithZeros = scoreImgs.FillWithZero;

    auto winPos = manager->GetPosition("Win").front();
    m_Win = std::make_shared<Image>(winPos.Path);
    m_Win->Position = winPos.Position;
    m_Win->Size = winPos.Size;
    m_Win->AnchorPoint = winPos.AnchorPoint;
    m_Win->Color3 = winPos.Color;

    auto losePos = manager->GetPosition("Lose").front();
    m_Lose = std::make_shared<Image>(losePos.Path);
    m_Lose->Position = losePos.Position;
    m_Lose->Size = losePos.Size;
    m_Lose->AnchorPoint = losePos.AnchorPoint;
    m_Lose->Color3 = losePos.Color;

    auto backRect = manager->GetRect("Back").front();
    auto backHoverOn = manager->GetPosition("BackHoverOn").front();
    auto backHoverOff = manager->GetPosition("BackHoverOff").front();

    auto backHoverOnImg = std::make_shared<Image>(backHoverOn.Path);
    backHoverOnImg->Position = backHoverOn.Position;
    backHoverOnImg->Size = backHoverOn.Size;
    backHoverOnImg->AnchorPoint = backHoverOn.AnchorPoint;
    backHoverOnImg->Color3 = backHoverOn.Color;

    auto backHoverOffImg = std::make_shared<Image>(backHoverOff.Path);
    backHoverOffImg->Position = backHoverOff.Position;
    backHoverOffImg->Size = backHoverOff.Size;
    backHoverOffImg->AnchorPoint = backHoverOff.AnchorPoint;
    backHoverOffImg->Color3 = backHoverOff.Color;

    m_BackButton = std::make_shared<ButtonImage>(
        Rect{
            (int)backRect.Position.X.Offset,
            (int)backRect.Position.Y.Offset,
            (int)backRect.Size.X.Offset,
            (int)backRect.Size.Y.Offset },
        std::make_pair(backHoverOffImg, backHoverOnImg));

    m_BackButton->OnClick([]() { MsgBox::Show("Hello", "Hello"); });

    auto coolPos = manager->GetPosition("StatsCool").front();
    auto goodPos = manager->GetPosition("StatsGood").front();
    auto badPos = manager->GetPosition("StatsBad").front();
    auto missPos = manager->GetPosition("StatsMiss").front();
    auto maxcomboPos = manager->GetPosition("StatsMaxCombo").front();
    auto maxjamPos = manager->GetPosition("StatsMaxJam").front();

    m_StatsPosition = {
        { 0, coolPos.Position },
        { 1, goodPos.Position },
        { 2, badPos.Position },
        { 3, missPos.Position },
        { 4, maxcomboPos.Position },
        { 5, maxjamPos.Position }
    };

    m_StatsAnchor = {
        { 0, coolPos.AnchorPoint },
        { 1, goodPos.AnchorPoint },
        { 2, badPos.AnchorPoint },
        { 3, missPos.AnchorPoint },
        { 4, maxcomboPos.AnchorPoint },
        { 5, maxjamPos.AnchorPoint }
    };

    auto statsImgs = manager->GetNumeric("Stats").front();
    m_Stats = std::make_shared<NumberSprite>(statsImgs.Files);
    m_Stats->NumberPosition = IntToPos(statsImgs.Direction);
    m_Stats->FillWithZeros = statsImgs.FillWithZero;
    m_Stats->Color = statsImgs.Color;
    m_Stats->Size = statsImgs.Size;

    m_ScoreInfo = ScoreInfo{
        .score = Env::GetInt("Score"),
        .cool = Env::GetInt("Cool"),
        .good = Env::GetInt("Good"),
        .bad = Env::GetInt("Bad"),
        .miss = Env::GetInt("Miss"),
        .maxJam = Env::GetInt("MaxJam"),
        .maxCombo = Env::GetInt("MaxCombo"),
        .isClear = Env::GetBool("IsClear")
    };

    m_IsAttached = true;
    return true;
}

bool Result::Detach()
{
    m_Result.reset();
    m_Text.reset();

    return true;
}