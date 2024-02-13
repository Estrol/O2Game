/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#ifndef __SPRITE_H_
#define __SPRITE_H_

#include <vector>

#include "Image.h"
#include <glm/glm.hpp>

namespace UI {
    class Sprite : public Image
    {
    public:
        Sprite();
        Sprite(
            std::filesystem::path               path,
            std::vector<std::vector<glm::vec2>> texCoords,
            double                              frameTime);

        Sprite(
            std::shared_ptr<Graphics::Texture2D> texture,
            std::vector<std::vector<glm::vec2>>  texCoords,
            double                               frameTime);

        Sprite(
            const char                         *buf,
            size_t                              size,
            std::vector<std::vector<glm::vec2>> texCoords,
            double                              frameTime);

        Sprite(
            const char                         *pixbuf,
            uint32_t                            width,
            uint32_t                            height,
            std::vector<std::vector<glm::vec2>> texCoords,
            double                              frameTime);

        void Draw(double delta);

        void SetSpriteIndex(int index);
        int  GetSpriteIndex() const;

        void   SetFrameTime(double time);
        double GetFrameTime() const;

    protected:
        void                                OnDraw() override;
        double                              m_frameTime;
        double                              m_elapsedFrameTime;
        int                                 m_spriteIndex;
        std::vector<std::vector<glm::vec2>> m_texCoords;
    };
} // namespace UI

#endif