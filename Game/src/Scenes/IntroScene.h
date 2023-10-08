#pragma once
#include <map>
#include "Scene.h"
#include <memory>
#include <UI/Quad.h>
#include <UI/Circle.h>

class IntroScene : public Scene {
public:
	IntroScene();

	void Render(double delta) override;
	void OnKeyDown(const KeyState& state) override;

	bool Attach() override;
	bool Detach() override;

private:
	std::unique_ptr<Quad> m_quad;
	std::unique_ptr<Circle> m_circle;
};