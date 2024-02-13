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

NumberSprite::NumberSprite()
{
    m_renderMode = UI::RenderMode::Batches;
}

NumberSprite::NumberSprite(
    std::filesystem::path               path,
    std::vector<std::vector<glm::vec2>> texCoords)
    : Sprite(path, texCoords, 0)
{
    m_renderMode = UI::RenderMode::Batches;
}

NumberSprite::NumberSprite(
    std::shared_ptr<Graphics::Texture2D> texture,
    std::vector<std::vector<glm::vec2>>  texCoords)
    : Sprite(texture, texCoords, 0)
{
    m_renderMode = UI::RenderMode::Batches;
}

void NumberSprite::Draw(int number)
{
    NumberToDraw = number;

    if (AlphaBlend) {
        BlendState = Env::GetInt("BlendAlpha");
    } else {
        BlendState = Env::GetInt("BlendNonAlpha");
    }

    UI::Base::Draw();
}

void NumberSprite::CalculateSize()
{
    UDim2 PositionToCalculate = Position - Position2;

    Rect   imageRect = GetImageSize();
    Rect   bufferRect = Graphics::NativeWindow::Get()->GetBufferSize();
    double ImageWidth = imageRect.Width;
    double ImageHeight = imageRect.Height;
    double Width = bufferRect.Width;
    double Height = bufferRect.Height;

    double X = 0;
    double Y = 0;

    if (Parent != nullptr && Parent != this) {
        Parent->CalculateSize();

        Width = Parent->AbsoluteSize.X;
        Height = Parent->AbsoluteSize.Y;
        X = Parent->AbsolutePosition.X;
        Y = Parent->AbsolutePosition.Y;
    }

    double x0 = (Width * PositionToCalculate.X.Scale) + PositionToCalculate.X.Offset;
    double y0 = (Height * PositionToCalculate.Y.Scale) + PositionToCalculate.Y.Offset;

    double x1 = (ImageWidth * Size.X.Scale) + Size.X.Offset;
    double y1 = (ImageHeight * Size.Y.Scale) + Size.Y.Offset;

    double xAnchor = x1 * std::clamp(AnchorPoint.X, 0.0, 1.0);
    double yAnchor = y1 * std::clamp(AnchorPoint.Y, 0.0, 1.0);

    x0 -= xAnchor;
    y0 -= yAnchor;

    AbsolutePosition = { (X + x0), (Y + y0) };
    AbsoluteSize = { x1, y1 };

    roundedCornerPixels = glm::vec4();
}

void NumberSprite::OnDraw()
{
    m_batches.clear();

    {
        std::string numberString = std::to_string(NumberToDraw);
        if (MaxDigits != 0 && numberString.size() > MaxDigits) {
            numberString = numberString.substr(numberString.size() - MaxDigits, MaxDigits);
        } else {
            while (numberString.size() < MaxDigits && FillWithZeros) {
                numberString = "0" + numberString;
            }
        }

        CalculateSize();
        float offsetScl = static_cast<float>(Offset) / 100.0f;

        switch (NumberPosition) {
            case NumericPosition::LEFT:
            {
                double tx = AbsolutePosition.X;
                for (int i = (int)numberString.length() - 1; i >= 0; i--) {
                    int digit = numberString[i] - '0';
                    m_spriteIndex = digit;

                    tx -= AbsoluteSize.X + (AbsoluteSize.X * offsetScl);
                    AbsolutePosition.X = tx;
                    AddToQueue();
                }
                break;
            }

            case NumericPosition::MID:
            {
                double totalWidth = 0;
                for (int i = 0; i < numberString.length(); i++) {
                    int digit = numberString[i] - '0';
                    totalWidth += AbsoluteSize.X + (AbsoluteSize.X * offsetScl);
                }

                double tx = AbsolutePosition.X - totalWidth / 2 + (Offset * totalWidth) / 200;
                for (int i = 0; i < numberString.length(); i++) {
                    int digit = numberString[i] - '0';
                    m_spriteIndex = digit;

                    AbsolutePosition.X = tx;
                    tx += AbsoluteSize.X + (AbsoluteSize.X * offsetScl);
                    AddToQueue();
                }
                break;
            }

            case NumericPosition::RIGHT:
            {
                double tx = AbsolutePosition.X;
                for (int i = 0; i < numberString.length(); i++) {
                    int digit = numberString[i] - '0';
                    m_spriteIndex = digit;

                    AbsolutePosition.X = tx;
                    tx += AbsoluteSize.X + (AbsoluteSize.X * offsetScl);
                    AddToQueue();
                }
                break;
            }

            default:
            {
                throw Exceptions::EstException("Invalid NumericPosition");
            }
        }
    }
}

void NumberSprite::AddToQueue()
{
    double x1 = AbsolutePosition.X;
    double y1 = AbsolutePosition.Y;
    double x2 = x1 + AbsoluteSize.X;
    double y2 = y1 + AbsoluteSize.Y;

    auto uv1 = m_texCoords[m_spriteIndex][0]; // Top-left
    auto uv2 = m_texCoords[m_spriteIndex][1]; // Top-right
    auto uv3 = m_texCoords[m_spriteIndex][3]; // Bottom-Right
    auto uv4 = m_texCoords[m_spriteIndex][2]; // Bottom-left

    glm::vec4 color = {
        Color3.R * 255,
        Color3.G * 255,
        Color3.B * 255,
        Transparency * 255
    };

    // clang-format off
    uint32_t col = ((uint32_t)(color.a) << 24) 
        | ((uint32_t)(color.b) << 16) 
        | ((uint32_t)(color.g) << 8) 
        | ((uint32_t)(color.r) << 0);
    // clang-format on

    shaderFragmentType = Graphics::Backends::ShaderFragmentType::Image;

    m_SubmitInfo.indices = { 0, 1, 2, 3, 4, 5 };
    m_SubmitInfo.vertices = {
        { { x1, y1 }, uv1, col },
        { { x1, y2 }, uv4, col },
        { { x2, y2 }, uv3, col },

        { { x1, y1 }, uv1, col },
        { { x2, y2 }, uv3, col },
        { { x2, y1 }, uv2, col },
    };

    InsertToBatch();
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