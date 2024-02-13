/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#ifndef __SPRITEBATCH_H__
#define __SPRITEBATCH_H__

#include "GraphicsBackendBase.h"
#include "GraphicsTexture2D.h"

#include <map>
#include <vector>

namespace Graphics {
    enum class SpriteBatchMode {
        Deferred,
        Immediate
    };

    enum class SpriteBatchSortMode {
        BackToFront,
        FrontToBack,
    };

    class SpriteBatch
    {
    public:
        SpriteBatch();
        ~SpriteBatch();

        void Begin();
        void Draw(Backends::SubmitInfo info);
        void Draw(std::vector<Backends::SubmitInfo> infos);
        void End();

        SpriteBatchMode     RenderMode;
        SpriteBatchSortMode SortMode;

    private:
        int m_CurrentQueue;

        bool                                             m_DrawingMode;
        std::map<int, std::vector<Backends::SubmitInfo>> m_Queue;
        const void                                      *m_Image;
    };
} // namespace Graphics

#endif