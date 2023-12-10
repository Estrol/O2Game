#pragma once
#include <Scene.h>

class ReloadScene : public Scene
{
public:
    ReloadScene();

    void Update(double delta) override;

    bool Attach() override;
    bool Detach() override;

private:
    double m_time = 0;
};