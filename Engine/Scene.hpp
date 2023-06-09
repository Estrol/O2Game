#pragma once
struct KeyState;
struct MouseState;

class Scene {
public:
	Scene();
	virtual ~Scene() = default;

	virtual bool Attach();
	virtual bool Detach();
	
	virtual void Update(double delta);
	virtual void Render(double delta);
	virtual void Input(double delta);

	virtual void OnKeyDown(const KeyState& state);
	virtual void OnKeyUp(const KeyState& state);
	virtual void OnMouseDown(const MouseState& state);
	virtual void OnMouseUp(const MouseState& state);
};