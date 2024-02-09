/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include "../../Env.h"
#include "NumberSprite.h"

#include <Exceptions/EstException.h>
#include <Graphics/NativeWindow.h>

NumberSprite::NumberSprite(std::vector<std::filesystem::path> numericsFiles)
{
    if (numericsFiles.size() != 10) {
        throw Exceptions::EstException("NumberSprite::NumberSprite: numericsFiles.size() != 10");
    }

    Position2 = UDim2::fromOffset(0, 0);
    AnchorPoint = { 0, 0 };
    Color = Color3::fromRGB(255, 255, 255);
    Size = UDim2::fromScale(1, 1);

    m_numericsTexture.resize(10);
    for (int i = 0; i < 10; i++) {
        auto path = numericsFiles[i];
        m_numericsTexture[i] = std::make_shared<Image>(path);
        m_numbericsWidth[i] = m_numericsTexture[i]->GetImageSize();
    }
}

NumberSprite::~NumberSprite()
{
    m_numericsTexture.clear();
}

void NumberSprite::Draw(int number)
{
    std::string numberString = std::to_string(number);
    if (MaxDigits != 0 && numberString.size() > MaxDigits) {
        numberString = numberString.substr(numberString.size() - MaxDigits, MaxDigits);
    } else {
        while (numberString.size() < MaxDigits && FillWithZeros) {
            numberString = "0" + numberString;
        }
    }

    auto rect = Graphics::NativeWindow::Get()->GetBufferSize();

    double xPos = (rect.Width * Position.X.Scale) + Position.X.Offset;
    double yPos = (rect.Height * Position.Y.Scale) + Position.Y.Offset;

    double xMPos = (rect.Width * Position2.X.Scale) + Position2.X.Offset;
    double yMPos = (rect.Height * Position2.Y.Scale) + Position2.Y.Offset;

    xPos += xMPos;
    yPos += yMPos;

    float offsetScl = static_cast<float>(Offset) / 100.0f;

    switch (NumberPosition) {
        case NumericPosition::LEFT:
        {
            float tx = static_cast<float>(xPos);
            for (int i = (int)numberString.length() - 1; i >= 0; i--) {
                int digit = numberString[i] - '0';

                tx -= (float)m_numbericsWidth[digit].Width + (m_numbericsWidth[digit].Width * offsetScl);
                auto &tex = m_numericsTexture[digit];
                tex->Position = UDim2({ 0, tx }, { 0, (float)yPos });
                tex->AlphaBlend = AlphaBlend;
                tex->AnchorPoint = AnchorPoint;
                tex->Size = Size;
                tex->Color3 = Color;
                tex->Draw();
            }
            break;
        }

        case NumericPosition::MID:
        {
            float totalWidth = 0;
            for (int i = 0; i < numberString.length(); i++) {
                int digit = numberString[i] - '0';
                totalWidth += (float)m_numbericsWidth[digit].Width + (m_numbericsWidth[digit].Width * offsetScl);
            }

            float tx = static_cast<float>(xPos) - totalWidth / 2 + (Offset * totalWidth) / 200;
            for (int i = 0; i < numberString.length(); i++) {
                int   digit = numberString[i] - '0';
                auto &tex = m_numericsTexture[digit];
                tex->Position = UDim2({ 0, (float)tx }, { 0, (float)yPos });
                tex->AlphaBlend = AlphaBlend;
                tex->AnchorPoint = AnchorPoint;
                tex->Size = Size;
                tex->Color3 = Color;
                tex->Draw();
                tx += (float)m_numbericsWidth[digit].Width + (m_numbericsWidth[digit].Width * offsetScl);
            }
            break;
        }

        case NumericPosition::RIGHT:
        {
            float tx = static_cast<float>(xPos);
            for (int i = 0; i < numberString.length(); i++) {
                int   digit = numberString[i] - '0';
                auto &tex = m_numericsTexture[digit];
                tex->Position = UDim2({ 0, (float)tx }, { 0, (float)yPos });
                tex->AnchorPoint = AnchorPoint;
                tex->AlphaBlend = AlphaBlend;
                tex->Size = Size;
                tex->Color3 = Color;
                tex->Draw();
                tx += (float)m_numbericsWidth[digit].Width + (m_numbericsWidth[digit].Width * offsetScl);
            }
            break;
        }

        default:
        {
            throw Exceptions::EstException("Invalid NumericPosition");
        }
    }
}

NumericPosition IntToPos(int i)
{
    switch (i) {
        case -1:
            return NumericPosition::LEFT;
        case 0:
            return NumericPosition::MID;
        case 1:
            return NumericPosition::RIGHT;

        default:
            throw Exceptions::EstException("IntToPos: i is not a valid NumericPosition");
    }
}