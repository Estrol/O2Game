#include "DrawableTile.hpp"
#include "../Resources/GameResources.hpp"

DrawableTile::DrawableTile(NoteImage* frame) : Texture2D::Texture2D(frame->Texture) {
	m_bDisposeTexture = false;

	m_actualSize = frame->TextureRect;

	// clean up this later
	Size.X.Scale = 1.0f;
	Size.X.Offset = 0;
	Size.Y.Scale = 1.0f;
	Size.Y.Offset = 0;

	AnchorPoint = { 0, 0.5 };
}
