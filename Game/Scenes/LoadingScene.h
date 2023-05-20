#pragma once
#include "../../Engine/EstEngine.hpp"

class LoadingScene : public Scene {
public:
	LoadingScene();
	~LoadingScene();

	void Update(double delta) override;
	void Render(double delta) override;

	bool Attach() override;
	bool Detach() override;

private:
	// can't load chart lmao
	bool fucked = false;
	bool is_shown = false;

	double m_counter;
	Texture2D* m_background;
};