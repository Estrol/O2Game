#include "DrawableNote.hpp"
#include "../Resources/GameResources.hpp"

DrawableNote::DrawableNote(NoteImage* frame) : Texture2D::Texture2D() {
	m_pTexture = frame->Texture;
	m_bDisposeTexture = false;

	m_actualSize = frame->TextureRect;

	// clean up this later
	Size.X.Scale = 1.0f;
	Size.X.Offset = 0;
	Size.Y.Scale = 1.0f;
	Size.Y.Offset = 0;

	AnchorPoint = { 0, 1 };

	m_pSpriteBatch = Renderer::GetInstance()->GetSpriteBatch(1);
}
