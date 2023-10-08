#include <UI/Circle.h>
#include <glm/glm.hpp>

#include <Rendering/Window.h>
#include <Rendering/Renderer.h>

constexpr float PI = 3.14159265358979323846f;

Circle::Circle() {
    Radius = 1.0f;
    Counts = 360;
}

Circle::~Circle() {

}

void Circle::OnDraw() {
    auto window = GameWindow::GetInstance();
    auto renderer = Renderer::GetInstance();
    
    ImU32 color = IM_COL32(Color3.R * 255, Color3.G * 255, Color3.B * 255, Transparency * 255);
    SDL_Color sdlcolor = { 
        Color3.R * 255, Color3.G * 255, Color3.B * 255, Transparency * 255
    };

    CalculateSize();

    m_vertices.clear();
    m_indices.clear();

    auto position = AbsolutePosition;
    auto size = AbsoluteSize;

    float angle = 360.0f / Counts;
    int triangleCount = Counts - 2;

    for (int i = 0; i < Counts; i++) {
        float currentAngle = angle * i;
        float x = Radius * cos(glm::radians(currentAngle));
        float y = Radius * sin(glm::radians(currentAngle));

        ImDrawVert vertex;
        vertex.pos = ImVec2(x, y);
        vertex.uv = ImVec2(0.0f, 0.0f);
        vertex.col = color;

        m_vertices.push_back(vertex);
    }

    for (int i = 0; i < triangleCount; i++) {
        m_indices.push_back(0);
        m_indices.push_back(i + 1);
        m_indices.push_back(i + 2);
    }

    /*float scaleX = size.X / (2.0f * Radius);
    float scaleY = size.Y / (2.0f * Radius);

    for (int i = 0; i < m_vertices.size(); i++) {
        m_vertices[i].pos.x = position.X + scaleX * (m_vertices[i].pos.x - position.X);
        m_vertices[i].pos.y = position.Y + scaleY * (m_vertices[i].pos.y - position.Y);
    }*/
}