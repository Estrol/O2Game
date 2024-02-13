#include <Graphics/GraphicsTexture2D.h>
#include <UI/Sprite.h>

using namespace UI;

Sprite::Sprite() : Image()
{
    m_spriteIndex = 0;

    m_frameTime = 1.0;
    m_texCoords = {
        { glm::vec2(0.0f, 0.0f),
          glm::vec2(1.0f, 0.0f),
          glm::vec2(1.0f, 1.0f),
          glm::vec2(0.0f, 1.0f) }
    };
}

Sprite::Sprite(
    std::filesystem::path               path,
    std::vector<std::vector<glm::vec2>> texCoords,
    double                              frameTime)
    : Image(path)
{
    if (!texCoords.size()) {
        throw std::runtime_error("Invalid texture coordinates");
    }

    auto frameSize = m_texture->GetSize();

    m_spriteIndex = 0;
    m_texCoords = texCoords;
    for (auto &coord : m_texCoords) {
        for (auto &uv : coord) {
            uv.x /= (float)frameSize.Width;
            uv.y /= (float)frameSize.Height;
        }
    }

    m_frameTime = 1.0 / frameTime;
}

Sprite::Sprite(
    std::shared_ptr<Graphics::Texture2D> texture,
    std::vector<std::vector<glm::vec2>>  texCoords,
    double                               frameTime)
    : Image(texture)
{
    if (!texCoords.size()) {
        throw std::runtime_error("Invalid texture coordinates");
    }

    auto frameSize = m_texture->GetSize();

    m_spriteIndex = 0;
    m_texCoords = texCoords;
    for (auto &coord : m_texCoords) {
        for (auto &uv : coord) {
            uv.x /= (float)frameSize.Width;
            uv.y /= (float)frameSize.Height;
        }
    }

    m_frameTime = 1.0 / frameTime;
}

Sprite::Sprite(
    const char                         *buf,
    size_t                              size,
    std::vector<std::vector<glm::vec2>> texCoords,
    double                              frameTime)
    : Image(buf, size)
{
    if (!texCoords.size()) {
        throw std::runtime_error("Invalid texture coordinates");
    }

    auto frameSize = m_texture->GetSize();

    m_spriteIndex = 0;
    m_texCoords = texCoords;
    for (auto &coord : m_texCoords) {
        for (auto &uv : coord) {
            uv.x /= (float)frameSize.Width;
            uv.y /= (float)frameSize.Height;
        }
    }

    m_frameTime = 1.0 / frameTime;
}

Sprite::Sprite(
    const char                         *pixbuf,
    uint32_t                            width,
    uint32_t                            height,
    std::vector<std::vector<glm::vec2>> texCoords,
    double                              frameTime)
    : Image(pixbuf, width, height)
{
    if (!texCoords.size()) {
        throw std::runtime_error("Invalid texture coordinates");
    }

    auto frameSize = m_texture->GetSize();

    m_spriteIndex = 0;
    m_texCoords = texCoords;
    for (auto &coord : m_texCoords) {
        for (auto &uv : coord) {
            uv.x /= (float)frameSize.Width;
            uv.y /= (float)frameSize.Height;
        }
    }

    m_frameTime = 1.0 / frameTime;
}

void Sprite::Draw(double delta)
{
    m_elapsedFrameTime += delta;

    if (m_elapsedFrameTime >= m_frameTime) {
        m_elapsedFrameTime = 0;
        m_spriteIndex = (m_spriteIndex + 1) % m_texCoords.size();
    }

    Image::Draw();
}

void Sprite::SetSpriteIndex(int index)
{
    m_spriteIndex = index;
}

int Sprite::GetSpriteIndex() const
{
    return m_spriteIndex;
}

void Sprite::SetFrameTime(double time)
{
    m_frameTime = time;
}

double Sprite::GetFrameTime() const
{
    return m_frameTime;
}

void Sprite::OnDraw()
{
    CalculateSize();

    double x1 = AbsolutePosition.X;
    double y1 = AbsolutePosition.Y;
    double x2 = x1 + AbsoluteSize.X;
    double y2 = y1 + AbsoluteSize.Y;

    auto uv1 = m_texCoords[m_spriteIndex][0]; // Top-left
    auto uv2 = m_texCoords[m_spriteIndex][1]; // Top-right
    auto uv3 = m_texCoords[m_spriteIndex][3]; // Bottom-Right
    auto uv4 = m_texCoords[m_spriteIndex][2]; // Bottom-left

    glm::vec4 color = {
        Color3.R * 255,
        Color3.G * 255,
        Color3.B * 255,
        Transparency * 255
    };

    // clang-format off
    uint32_t col = ((uint32_t)(color.a) << 24) 
        | ((uint32_t)(color.b) << 16) 
        | ((uint32_t)(color.g) << 8) 
        | ((uint32_t)(color.r) << 0);
    // clang-format on

    shaderFragmentType = Graphics::Backends::ShaderFragmentType::Image;

    m_SubmitInfo.indices = { 0, 1, 2, 3, 4, 5 };
    m_SubmitInfo.vertices = {
        { { x1, y1 }, uv1, col },
        { { x1, y2 }, uv4, col },
        { { x2, y2 }, uv3, col },

        { { x1, y1 }, uv1, col },
        { { x2, y2 }, uv3, col },
        { { x2, y1 }, uv2, col },
    };
}