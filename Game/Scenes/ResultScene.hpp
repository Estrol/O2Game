#pragma once
#include "../../Engine/Scene.hpp"

class ResultScene : public Scene {
public:
	ResultScene();

	void Render(double delta) override;

	bool Attach() override;
	bool Detach() override;

private:
	bool m_backButton;
};