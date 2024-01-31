#include "O2NumericTexture.hpp"
#include "Misc/Lodepng.h"
#include "O2Texture.hpp"
#include "Rendering/Renderer.h"

O2NumericTexture::O2NumericTexture(OJS *ojs)
{
    if (ojs->FrameCount < 10) {
        throw std::runtime_error("O2NumericTexture::O2NumericTexture: Frame size must be at least 10");
    }

    Renderer *render = Renderer::GetInstance();

    for (int i = 0; i < 10; i++) {
        auto frame = ojs->Frames[i].get();

        auto tex = new O2Texture(frame);
        m_numericsTexture.emplace_back(tex);
        m_numbericsWidth[i] = tex->GetOriginalRECT();
    }
}