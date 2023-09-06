#pragma once
#include <map>

#include "Scene.h"
#include "Texture/Text.h"
#include "Texture/Texture2D.h"
#include "../Engine/Button.hpp"

struct KeyState;

class IntroScene : public Scene {
public:
	IntroScene();

	void Render(double delta) override;
	void OnKeyDown(const KeyState& state) override;

	bool Attach() override;
	bool Detach() override;

private:
	void PrepareDB();

	bool IsReady = false;
	bool IsOpenPrompt = false;
	double waitFrame = 0; // this is :evil:

	int currentState = 0;
	std::vector<std::string> SelectedFolders;
	
	std::unique_ptr<Text> m_text;
	int nextIndex = 0;
	std::vector<std::filesystem::path> m_songFiles;
};