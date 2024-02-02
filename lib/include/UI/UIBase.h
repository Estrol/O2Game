/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#ifndef __Base_H_
#define __Base_H_

#include <Graphics/GraphicsBackendBase.h>
#include <Graphics/GraphicsTexture2D.h>
#include <Graphics/Utils/Rect.h>
#include <Math/Color3.h>
#include <Math/UDim2.h>
#include <Math/Vector2.h>
#include <Math/Vector4.h>

namespace UI {
    class Base;

    struct UIBaseVariableCache
    {
        Vector2 AbsolutePosition;
        Vector2 AbsoluteSize;
        UDim2   Position;
        UDim2   Size;
        Base   *Parent;
        bool    ClampToParent;
        Vector2 AnchorPoint;
        Vector4 CornerRadius;
        Color3  Color3;
        float   Transparency;
        float   Rotation;
    };

    enum class RenderMode {
        Normal,
        Batches,
    };

    class Base
    {
    public:
        Base();
        virtual ~Base();

        Vector2 AbsolutePosition;
        Vector2 AbsoluteSize;

        UDim2 Position;
        UDim2 Size;

        Base *Parent;
        bool  ClampToParent;
        bool  Static;

        Color3  Color3;
        Vector2 AnchorPoint;
        Vector4 CornerRadius;

        Graphics::Backends::BlendHandle BlendState;

        float Transparency;
        float Rotation;

        virtual void Draw();
        virtual void Draw(Rect clipRect);

        virtual void CalculateSize();

    protected:
        virtual bool NeedsUpdate();
        virtual void UpdateCache();

        virtual void OnDraw();
        void         InsertToBatch();
        void         RotateVertex();

        bool m_ScaleSize = false;

        glm::vec4 roundedCornerPixels;

        Rect                                   clipRect = {};
        Graphics::Backends::ShaderFragmentType shaderFragmentType;

        std::vector<Graphics::Backends::Vertex> m_vertices;
        std::vector<uint16_t>                   m_indices;

        std::unique_ptr<Graphics::Texture2D> m_texture;
        Graphics::Texture2D                 *m_texturePtr = nullptr;
        RenderMode                           m_renderMode = RenderMode::Normal;

        Graphics::Backends::SubmitInfo              m_SubmitInfo;
        std::vector<Graphics::Backends::SubmitInfo> m_batches;

    private:
        UIBaseVariableCache m_cache;

        void DrawVertices();
    };
} // namespace UI

#endif