/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include <UI/TextBox.h>

using namespace UI;

TextBox::TextBox() : Text::Text()
{
    nextTick = 0.0f;
    hintText = "";
    onFocus = false;
    showTick = false;
}

TextBox::TextBox(std::string fontName, float fontSize) : Text::Text(fontName, fontSize)
{
    nextTick = 0.0f;
    hintText = "";
    onFocus = false;
    showTick = false;
}

TextBox::TextBox(Fonts::FontLoadFileInfo &info) : Text::Text(info)
{
    nextTick = 0.0f;
    hintText = "";
    onFocus = false;
    showTick = false;
}

TextBox::TextBox(Fonts::FontLoadBufferInfo &info) : Text::Text(info)
{
    nextTick = 0.0f;
    hintText = "";
    onFocus = false;
    showTick = false;
}

void TextBox::Draw(double delta)
{
    nextTick += static_cast<float>(delta);
    if (nextTick >= 0.5) {
        showTick = !showTick;
    }

    if (currentText.empty() && !onFocus) {
        DrawString(hintText);
    } else {
        if (currentText.empty()) {
            if (showTick) {
                DrawString("|");
            }
        } else {
            DrawString(currentText);
        }
    }
}

void TextBox::OnInput()
{
}

void TextBox::SetHintText(std::string text)
{
    hintText = text;
}
