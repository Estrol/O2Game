#pragma once
#include <memory>
#include "Scene.h"
#include "Texture/Texture2D.h"

class ResultScene : public Scene {
public:
	ResultScene();

	void Render(double delta) override;

	bool Attach() override;
	bool Detach() override;

private:
	bool m_backButton;

	std::unique_ptr<Texture2D> m_background;
};