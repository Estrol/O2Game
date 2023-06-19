#include "DrawableNote.hpp"
#include "../Resources/GameResources.hpp"

DrawableNote::DrawableNote(NoteImage* frame) : FrameTimer::FrameTimer(frame->Texture) {
	AnchorPoint = { 0.0, 1.0 };

	for (auto& _frame : m_frames) {
		_frame->SetOriginalRECT(frame->TextureRect);
	}

	SetFPS(30);
}
