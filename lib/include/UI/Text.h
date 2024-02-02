/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#ifndef __TEXT_H_
#define __TEXT_H_

#include "UIBase.h"
#include <Fonts/FontManager.h>
#include <string>

namespace UI {
    enum class Alignment {
        Left,
        Center,
        Right
    };

    class Text : public Base
    {
    public:
        Text();
        Text(std::string fontName, float fontSize = 16.0f);
        Text(Fonts::FontLoadFileInfo &info);
        Text(Fonts::FontLoadBufferInfo &info);

        void      DrawString(std::string text);
        void      DrawStringFormatted(std::string text, ...);
        glm::vec2 MeasureString(std::string text);

        float     Scale;
        Alignment Alignment;
        Rect      TextClipping;

    protected:
        void          OnDraw() override;
        Fonts::Glyph *FindGlyph(uint32_t c);

        float             nextTick;
        std::string       m_TextToDraw;
        Fonts::FontAtlas *m_FontAtlas;
    };
} // namespace UI

#endif