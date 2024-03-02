#include "DrawableNote.hpp"
#include "../Resources/GameResources.hpp"
#include "Rendering/Renderer.h"
#include "Texture/Texture2D.h"

DrawableNote::DrawableNote(NoteImage* frame) : FrameTimer::FrameTimer()
{
    m_frames = std::vector<Texture2D*>();

    if (Renderer::GetInstance()->IsVulkan()) {
        for (auto& frame : frame->VulkanTexture) {
            m_frames.push_back(new Texture2D(frame));
        }
    }
    else {
        for (auto& frame : frame->Texture) {
            m_frames.push_back(new Texture2D(frame));
        }
    }

    AnchorPoint = { 0.0, 1.0 };

    for (auto& _frame : m_frames) {
        _frame->SetOriginalRECT(frame->TextureRect);
    }

    SetFPS(0); // FIXME: i had to use this value otherwise it has glitch
}
