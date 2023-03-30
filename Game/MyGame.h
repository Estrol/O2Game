#pragma once
#include "../Engine/EstEngine.hpp"

class MyGame : public Game {
public:
	~MyGame();

	bool Init() override;
	void Run(double frameRate) override;

protected:
	void Update(double deltaTime) override;
	void Render(double deltaTime) override;
	void Input(double deltaTime) override;
};