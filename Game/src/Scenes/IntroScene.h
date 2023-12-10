#pragma once
#include "Scene.h"
#include <UI/Circle.h>
#include <UI/Quad.h>
#include <map>
#include <memory>

class IntroScene : public Scene
{
public:
    IntroScene();

    void Render(double delta) override;
    void OnKeyDown(const KeyState &state) override;

    bool Attach() override;
    bool Detach() override;

private:
    std::unique_ptr<Quad>   m_quad;
    std::unique_ptr<Circle> m_circle;
};