#pragma once
#include "Game.h"

class MyGame : public Game {
public:
	~MyGame();

	bool Init() override;
	void Run() override;
	void SelectSkin(std::string name);

protected:
	void Update(double deltaTime) override;
	void Render(double deltaTime) override;
	void Input(double deltaTime) override;
};