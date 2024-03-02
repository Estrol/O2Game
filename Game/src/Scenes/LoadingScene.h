#pragma once
#include "Scene.h"
#include "Texture/Texture2D.h"
#include <iostream>

class LoadingScene : public Scene
{
public:
    LoadingScene();
    ~LoadingScene();

    void Update(double delta) override;
    void Render(double delta) override;

    bool Attach() override;
    bool Detach() override;

private:
    // can't load chart lmao
    bool IsFile = false; // fix for most problem
    bool fucked = false;
    bool is_shown = false;
    bool is_ready = false;
    bool dont_dispose = false;

    std::u8string m_title;

    double     m_counter;
    Texture2D* m_background;
};