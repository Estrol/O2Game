#pragma once
#include <unordered_map>
#include <mutex>
#include <functional>
#include <chrono>

/* Forward Declaration */
class Game;
class Scene;
struct KeyState;
struct MouseState;
enum class FrameLimitMode;

enum class ExecuteThread {
	WINDOW,
	UPDATE
};

struct QueueInfo {
	std::function<void()> callback;
	std::chrono::system_clock::time_point time;
};

class SceneManager {
public:
	void Update(double delta);
	void Render(double delta);
	void Input(double delta);

	void OnKeyDown(const KeyState& state);
	void OnKeyUp(const KeyState& state);
	void OnMouseDown(const MouseState& state);
	void OnMouseUp(const MouseState& state);

	void IAddScene(int idx, Scene* scene);
	void IChangeScene(int idx);

	void SetParent(Game* parent);
	void SetFrameLimit(double frameLimit);
	void SetFrameLimitMode(FrameLimitMode mode);
	void StopGame();

	static void DisplayFade(int transparency, std::function<void()> callback);
	static void ExecuteAfter(int ms_time, std::function<void()> callback);
	static void GameExecuteAfter(ExecuteThread thread, int ms_time, std::function<void()> callback);

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

	std::mutex m_mutex;

	std::thread::id m_renderId;
	std::thread::id m_inputId;

	std::vector<QueueInfo> m_queue_render;
	std::vector<QueueInfo> m_queue_input;

	Game* m_parent = nullptr;
};