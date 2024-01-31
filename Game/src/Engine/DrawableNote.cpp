#include "DrawableNote.hpp"
#include "../Resources/GameResources.hpp"
#include "Rendering/Renderer.h"
#include "Texture/Texture2D.h"

DrawableNote::DrawableNote(NoteImage *frame) : FrameTimer::FrameTimer()
{
    m_frames = std::vector<Texture2D *>();

    if (Renderer::GetInstance()->IsVulkan()) {
        for (auto &frame : frame->VulkanTexture) {
            m_frames.emplace_back(new Texture2D(frame));
        }
    } else {
        for (auto &frame : frame->Texture) {
            m_frames.emplace_back(new Texture2D(frame));
        }
    }

    AnchorPoint = { 0.0, 1.0 };

    for (auto &_frame : m_frames) {
        _frame->SetOriginalRECT(frame->TextureRect);
    }

    SetFPS(30); //TODO: Fix animation render glitch, i can't figure it out
}
