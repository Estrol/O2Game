#pragma once

#include "../../Engine/Texture2D.hpp"
#include "../../Engine/Scene.hpp"
#include "../../Engine/Text.hpp"
#include "../Engine/Button.hpp"
#include <map>

struct KeyState;

class IntroScene : public Scene {
public:
	IntroScene();

	void Render(double delta) override;
	void OnKeyDown(const KeyState& state) override;

	bool Attach() override;
	bool Detach() override;

private:
	bool IsReady = false;
	double waitFrame = 0; // this is :evil:

	Text* m_text;
	int nextIndex = 0;
	std::vector<std::filesystem::path> m_songFiles;
};