#ifndef D3D11_TEXTURE_2D_H
#define D3D11_TEXTURE_2D_H

#if _WIN32 && defined(__ENABLE_D3D11__)
#ifndef _MSC_VER
#error "This complication only support MSVC compiler"
#endif

#include <Graphics/GraphicsTexture2D.h>
#include <d3d11.h>

namespace Graphics {
    struct D3D11Texture2D : public Texture2D
    {
    public:
        D3D11Texture2D(TextureSamplerInfo samplerInfo);
        ~D3D11Texture2D() override;

        void Load(std::filesystem::path path) override;
        void Load(const unsigned char *buf, size_t size) override;
        void Load(const unsigned char *pixbuf, uint32_t width, uint32_t height) override;

        const void *GetId() override;
        const Rect  GetSize() override;

    private:
        ID3D11Texture2D          *Texture;
        ID3D11ShaderResourceView *TextureView;
        ID3D11SamplerState       *SamplerState;

        Rect Size;
        int  Channels;
    };
} // namespace Graphics

#endif // _WIN32 && defined(__ENABLE_D3D11__)
#endif