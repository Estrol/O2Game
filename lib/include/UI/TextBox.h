/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#ifndef __TEXT_BOX_H_
#define __TEXT_BOX_H_

#include "Text.h"
#include "UIBase.h"
#include <Fonts/FontManager.h>

// Workaround because there a variable called Color3 in the UI namespace
typedef Color3 Color3Ref;

namespace UI {
    class TextBox : public Text
    {
    public:
        TextBox();
        TextBox(std::string fontName, float fontSize = 16.0f);
        TextBox(Fonts::FontLoadFileInfo &info);
        TextBox(Fonts::FontLoadBufferInfo &info);

        void Draw(double delta);
        void OnInput();
        void SetHintText(std::string text);

        Color3Ref HintTextColor3;

    private:
        std::string currentText;
        std::string hintText;
        bool        onFocus;
        bool        showTick;
    };
} // namespace UI

#endif