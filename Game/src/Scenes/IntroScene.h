#pragma once
#include "Scene.h"
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
};