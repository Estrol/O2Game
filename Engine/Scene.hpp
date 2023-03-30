#pragma once
struct KeyState;

class Scene {
public:
	Scene();
	~Scene();

	virtual bool Attach();
	virtual bool Detach();
	
	virtual void Update(double delta);
	virtual void Render(double delta);
	virtual void Input(double delta);

	virtual void OnKeyDown(const KeyState& state);
	virtual void OnKeyUp(const KeyState& state);
};