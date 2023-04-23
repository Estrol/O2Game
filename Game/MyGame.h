#pragma once
#include "../Engine/EstEngine.hpp"

class MyGame : public Game {
public:
	~MyGame();

	bool Init() override;
	void Run(double frameRate) override;
	void SelectSkin(std::string name);

protected:
	void Update(double deltaTime) override;
	void Render(double deltaTime) override;
	void Input(double deltaTime) override;
};