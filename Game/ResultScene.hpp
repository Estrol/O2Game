#pragma once
#include "../Engine/Scene.hpp"

class ResultScene : public Scene {
public:
	ResultScene();

	bool Attach() override;
	bool Detach() override;
};