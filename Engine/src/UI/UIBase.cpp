#include <UI/UIBase.h>
#include <Rendering/Window.h>
#include <Rendering/Renderer.h>
#include <Rendering/Vulkan/VulkanEngine.h>
#include <Rendering/Vulkan/Texture2DVulkan.h>
#include <algorithm>
#include <iostream>

UIBase::UIBase() {
    Position = UDim2::fromScale(0.0, 0.0);
    Size = UDim2::fromScale(1.0, 1.0);
    AnchorPoint = { 0.0f, 0.0f };
    Color3 = Color3::FromRGB(255, 255, 255);
    Transparency = 1;
    Rotation = 0;
    Parent = nullptr;
    ClampToParent = false;
}

UIBase::~UIBase() {

}

void UIBase::Draw() {
    auto window = GameWindow::GetInstance();
    clipRect = {
        0,
        0,
        window->GetBufferWidth(),
        window->GetBufferHeight()
    };

    OnDraw();

    DrawVertices();
}

void UIBase::Draw(Rect _clipRect) {
    clipRect = _clipRect;

    OnDraw();
    
    DrawVertices();
}

void UIBase::CalculateSize() {
    float ScreenSizeX, ScreenSizeY;
    float PositionX, PositionY;

    if (Parent == nullptr || Parent == this) {
        auto window = GameWindow::GetInstance();

        ScreenSizeX = (float)window->GetBufferWidth();
        ScreenSizeY = (float)window->GetBufferHeight();
        PositionX = 0;
        PositionY = 0;
    } else {
        Parent->CalculateSize();

        ScreenSizeX = (float)Parent->AbsoluteSize.X;
        ScreenSizeY = (float)Parent->AbsoluteSize.Y;
        PositionX = (float)Parent->AbsolutePosition.X;
        PositionY = (float)Parent->AbsolutePosition.Y;
    }

    float x0 = ScreenSizeX * Position.X.Scale + Position.X.Offset;
    float y0 = ScreenSizeY * Position.Y.Scale + Position.Y.Offset;
    
    float x1 = ScreenSizeX * Size.X.Scale + Size.X.Offset;
    float y1 = ScreenSizeY * Size.Y.Scale + Size.Y.Offset;

    float xAnchor = x1 * std::clamp((float)AnchorPoint.X, 0.0f, 1.0f);
    float yAnchor = y1 * std::clamp((float)AnchorPoint.Y, 0.0f, 1.0f);

    x0 -= xAnchor;
    y0 -= yAnchor;

    if (ClampToParent && Parent != nullptr) {
        x0 = std::clamp(x0, PositionX, PositionX + ScreenSizeX - x1); // adjusted so width doesn't exceed the parent boundary
        y0 = std::clamp(y0, PositionY, PositionY + ScreenSizeY - y1); // adjusted so height doesn't exceed the parent boundary

        float tmp = std::clamp(x0 + x1, PositionX, PositionX + ScreenSizeX);
        x1 = tmp - x0;

        tmp = std::clamp(y0 + y1, PositionY, PositionY + ScreenSizeY);
        y1 = tmp - y0;
    }

    AbsolutePosition = { PositionX + x0, PositionY + y0 };
    AbsoluteSize = { x1, y1 };
}

void UIBase::OnDraw() {
    throw std::runtime_error("Not implemented!");
}

void UIBase::RotatePoint(float x, float y, float cx, float cy, float theta, float &out_x, float &out_y) {

}

void UIBase::DrawVertices() {
    auto window = GameWindow::GetInstance();
    auto renderer = Renderer::GetInstance();

    auto finalRect = clipRect;

    {
        finalRect.left *= window->GetWidthScale();
        finalRect.right *= window->GetHeightScale();
        finalRect.top *= window->GetWidthScale();
        finalRect.bottom *= window->GetHeightScale();
    }

    if (renderer->IsVulkan()) {
        VkRect2D scissor = {};
        scissor.offset.x = finalRect.left;
        scissor.offset.y = finalRect.top;
        scissor.extent.width = finalRect.right - finalRect.left;
        scissor.extent.height = finalRect.bottom - finalRect.top;

        auto dummyTex = vkTexture::GetDummyImage();

        SubmitQueueInfo info = {};
        info.descriptor = VK_NULL_HANDLE;
        info.vertices = m_vertices;
        info.scissor = scissor;
        info.indices = m_indices;

        info.descriptor = vkTexture::GetVkDescriptorSet(dummyTex);
        info.AlphaBlend = Transparency < 1.0;

        auto vkEngine = renderer->GetVulkanEngine();
        vkEngine->queue_submit(info);
    } else {
        std::vector<uint32_t> indices = {};
        std::vector<SDL_Vertex> vertices = {};

        for (int i = 0; i < m_vertices.size(); i++) {
            ImDrawVert& vertex = m_vertices[i];

            auto col = ImGui::ColorConvertU32ToFloat4(vertex.col);
            SDL_Color sdlcolor = {
                col.x * 255,
                col.y * 255,
                col.z * 255,
                col.w * 255
            };

            SDL_Vertex sdlvertex = {
                { vertex.pos.x, vertex.pos.y },
                sdlcolor
            };

            vertices.push_back(sdlvertex);
        }

        for (int i = 0; i < m_indices.size(); i++) {
            indices.push_back(m_indices[i]);
        }

        SDL_BlendMode previousBlend;
        SDL_GetRenderDrawBlendMode(renderer->GetSDLRenderer(), &previousBlend);
        SDL_SetRenderDrawBlendMode(renderer->GetSDLRenderer(), renderer->GetSDLBlendMode());

        SDL_Rect rc = {
            finalRect.left,
            finalRect.top,
            finalRect.right,
            finalRect.bottom
        };

        auto result = SDL_RenderSetClipRect(renderer->GetSDLRenderer(), &rc);
        if (result != 0) {
            throw std::runtime_error("Failed to set clip rect");
        }

        result = SDL_RenderGeometry(
            renderer->GetSDLRenderer(),
            nullptr,
            vertices.data(),
            vertices.size(),
            (int*)indices.data(), // bruh casting
            indices.size()
        );

        if (result != 0) {
            const char* err = SDL_GetError();
            throw std::runtime_error("Failed to render geometry");
        }

        SDL_RenderSetClipRect(renderer->GetSDLRenderer(), nullptr);
        SDL_SetRenderDrawBlendMode(renderer->GetSDLRenderer(), previousBlend);
    }   
}