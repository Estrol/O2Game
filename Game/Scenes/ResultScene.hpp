#pragma once
#include <memory>
#include "../../Engine/Scene.hpp"
#include "../../Engine/Texture2D.hpp"

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