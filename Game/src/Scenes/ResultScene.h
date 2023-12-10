#pragma once
#include "Scene.h"
#include "Texture/Texture2D.h"
#include <memory>

class ResultScene : public Scene
{
public:
    ResultScene();

    void Render(double delta) override;

    bool Attach() override;
    bool Detach() override;

private:
    bool m_backButton;
    bool m_retryButton;

    std::unique_ptr<Texture2D> m_background;
};