#pragma once
#include <map>
#include "Scene.h"
#include <memory>
#include <Texture/Texture2D.h>

class IntroScene : public Scene {
public:
	IntroScene();

	void Render(double delta) override;
	void OnKeyDown(const KeyState& state) override;

	bool Attach() override;
	bool Detach() override;

private:
	std::unique_ptr<Texture2D> m_background;
};