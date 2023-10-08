#include <UI/Quad.h>

#include <algorithm>
#include <iostream>
#include <SDL2/SDL.h>
#include <Rendering/Renderer.h>
#include <Rendering/Window.h>

const float PI = 3.14159265358979323846f;

Quad::Quad() {

}

Quad::~Quad() {

}

void Quad::OnDraw() {
    auto window = GameWindow::GetInstance();
    auto renderer = Renderer::GetInstance();
    
    ImU32 color = IM_COL32(Color3.R * 255, Color3.G * 255, Color3.B * 255, Transparency * 255);
    SDL_Color sdlcolor = { 
        Color3.R * 255, Color3.G * 255, Color3.B * 255, Transparency * 255
    };

    m_vertices.clear();
	m_indices.clear();
    CalculateSize();

	float x1 = AbsolutePosition.X;
	float y1 = AbsolutePosition.Y;
	float x2 = AbsolutePosition.X + AbsoluteSize.X;
	float y2 = AbsolutePosition.Y + AbsoluteSize.Y;

	ImVec2 uv1(0.0f, 0.0f);  // Top-left UV coordinate
	ImVec2 uv2(1.0f, 0.0f);  // Top-right UV coordinate
	ImVec2 uv3(1.0f, 1.0f);  // Bottom-right UV coordinate
	ImVec2 uv4(0.0f, 1.0f);  // Bottom-left UV coordinate

	m_vertices.resize(6);
	for (int i = 0; i < 6; i++) {
		ImDrawVert& vertex = m_vertices[i];
		switch (i) {
			case 0:
				vertex.pos = ImVec2(x1, y1);
				vertex.uv = uv1;
				break;
			case 1:
				vertex.pos = ImVec2(x2, y1);
				vertex.uv = uv2;
				break;
			case 2:
				vertex.pos = ImVec2(x2, y2);
				vertex.uv = uv3;
				break;
			case 3:
				vertex.pos = ImVec2(x1, y1);
				vertex.uv = uv1;
				break;
			case 4:
				vertex.pos = ImVec2(x2, y2);
				vertex.uv = uv3;
				break;
			case 5:
				vertex.pos = ImVec2(x1, y2);
				vertex.uv = uv4;
				break;
		}
		vertex.col = color;
	}

	m_indices = {
		0, 1, 2, 3, 4, 5
	};
}