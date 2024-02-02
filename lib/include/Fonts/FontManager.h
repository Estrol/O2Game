/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#ifndef __FONTMANAGER_H_
#define __FONTMANAGER_H_

#include <Graphics/GraphicsTexture2D.h>
#include <filesystem>
#include <glm/glm.hpp>
#include <map>
#include <memory>
#include <vector>

namespace Fonts {
    struct Glyph
    {
        uint32_t  Character;
        glm::vec2 Size;
        glm::vec2 Bearing;
        glm::vec2 Top;
        glm::vec2 Bottom;
        glm::vec4 Rect;
        glm::vec2 UV[4];
        float     FaceHeight;
        float     Ascender;
        float     Descender;
        uint32_t  Advance;
    };

    struct FontAtlas
    {
        std::unique_ptr<Graphics::Texture2D> Texture;
        std::vector<Glyph>                   Glyphs;
        glm::vec2                            TexSize;

        Glyph Invalid;
        float NewlineHeight;
        float FontSize;
    };

    struct FontLoadBufferInfo
    {
        std::vector<uint8_t>                       Buffer;
        float                                      FontSize;
        std::vector<std::pair<uint32_t, uint32_t>> Ranges;
    };

    struct FontLoadFileInfo
    {
        std::filesystem::path                      Path;
        float                                      FontSize;
        std::vector<std::pair<uint32_t, uint32_t>> Ranges;
    };

    class FontManager
    {
    public:
        FontAtlas *LoadFirst();
        FontAtlas *LoadFont(FontLoadBufferInfo &info);
        FontAtlas *LoadFont(FontLoadFileInfo &info);

        static FontManager *Get();
        static void         Destroy();

    private:
        FontManager();
        ~FontManager();

        static FontManager *m_instance;

        std::map<std::string, std::unique_ptr<FontAtlas>> m_fonts;
    };
} // namespace Fonts

#endif