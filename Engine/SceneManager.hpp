#pragma once
#include <unordered_map>

/* Forward Declaration */
class Game;
class Scene;
struct KeyState;

class SceneManager {
public:
	void Update(double delta);
	void Render(double delta);
	void Input(double delta);

	void OnKeyDown(const KeyState& state);
	void OnKeyUp(const KeyState& state);

	void IAddScene(int idx, Scene* scene);
	void IChangeScene(int idx);

	void SetParent(Game* parent);

	static void AddScene(int idx, Scene* scene);
	static void ChangeScene(int idx);

	static SceneManager* GetInstance();
	static void Release();

private:
	SceneManager();
	~SceneManager();

	static SceneManager* s_instance;
	
	std::unordered_map<int, Scene*> m_scenes;
	Scene* m_nextScene = nullptr;
	Scene* m_currentScene = nullptr;

	Game* m_parent = nullptr;
};