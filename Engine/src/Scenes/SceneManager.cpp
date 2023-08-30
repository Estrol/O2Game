#include "SceneManager.h"
#include "Scene.h"
#include "Game.h"
#include "MsgBox.h"

#include <string>
#include <mutex>
#include <iostream>
#include <exception>

SceneManager::SceneManager() {
	m_scenes = std::unordered_map<int, Scene*>();
	m_parent = nullptr;
}

SceneManager::~SceneManager() {
	{
		std::lock_guard<std::mutex> lock(m_mutex);

		for (auto& it : m_scenes) {
			it.second->Detach();
			delete it.second;
		}
	}
}

SceneManager* SceneManager::s_instance = nullptr;

void SceneManager::Update(double delta) {
	if (m_nextScene != nullptr) {
		std::lock_guard<std::mutex> lock(m_mutex);

		if (!m_nextScene->Attach()) {
			MsgBox::ShowOut("EstEngine Error", "Failed to init next scene", MsgBoxType::OK, MsgBoxFlags::BTN_ERROR);
			m_parent->Stop();
			return;
		}

		if (m_currentScene) {
			auto curScene = m_currentScene;
			m_currentScene = nullptr;

			if (!curScene->Detach()) {
				MsgBox::ShowOut("EstEngine Error", "Failed to detach current screen", MsgBoxType::OK, MsgBoxFlags::BTN_ERROR);
				m_parent->Stop();
				return;
			}
		}

		m_currentScene = m_nextScene;
		m_nextScene = nullptr;
	}
	else {
		m_currentScene->Update(delta);

		if (m_queue_render.size()) {
			for (auto it = m_queue_render.begin(); it != m_queue_render.end();) {
				if (it->time <= std::chrono::system_clock::now()) {
					it->callback();
					it = m_queue_render.erase(it);
				}
				else {
					it++;
				}
			}
		}
	}
}

void SceneManager::Render(double delta) {
	m_renderId = std::this_thread::get_id();

	if (m_currentScene) m_currentScene->Render(delta);
}

void SceneManager::Input(double delta) {
	m_inputId = std::this_thread::get_id();

	if (m_currentScene) m_currentScene->Input(delta);

	if (m_queue_input.size()) {
		for (auto it = m_queue_input.begin(); it != m_queue_input.end();) {
			if (it->time <= std::chrono::system_clock::now()) {
				it->callback();
				it = m_queue_input.erase(it);
			}
			else {
				it++;
			}
		}
	}
}

void SceneManager::OnKeyDown(const KeyState& state) {
	if (m_currentScene) m_currentScene->OnKeyDown(state);
}

void SceneManager::OnKeyUp(const KeyState& state) {
	if (m_currentScene) m_currentScene->OnKeyUp(state);
}

void SceneManager::OnMouseDown(const MouseState& state) {
	if (m_currentScene) m_currentScene->OnMouseDown(state);
}

void SceneManager::OnMouseUp(const MouseState& state) {
	if (m_currentScene) m_currentScene->OnMouseUp(state);
}

static const char* notInitialized = "SceneManager is not initalized";

void SceneManager::AddScene(int idx, Scene* scene) {
	if (s_instance == nullptr) throw std::runtime_error(notInitialized);

	std::cout << "Added scene: " << idx << std::endl;
	s_instance->m_scenes[idx] = scene;
}

void SceneManager::ChangeScene(int idx) {
	if (s_instance == nullptr) throw std::runtime_error(notInitialized);

	std::cout << "Change scene: " << idx << std::endl;
	s_instance->IChangeScene(idx);
}

void SceneManager::IAddScene(int idx, Scene* scene) {
	m_scenes[idx] = scene;
}

void SceneManager::IChangeScene(int idx) {
	if (m_scenes.find(idx) == m_scenes.end()) {
		std::string msg = "Failed to find SceneId: " + std::to_string(idx);
		
		MsgBox::ShowOut("EstEngine Error", msg.c_str(), MsgBoxType::OK, MsgBoxFlags::BTN_ERROR);
		return;
	}

	m_nextScene = m_scenes[idx];
}

void SceneManager::SetParent(Game* parent) {
	m_parent = parent;
}

void SceneManager::SetFrameLimit(double frameLimit) {
	m_parent->SetFramelimit(frameLimit);
}

void SceneManager::SetFrameLimitMode(FrameLimitMode mode) {
	m_parent->SetFrameLimitMode(mode);
}

void SceneManager::DisplayFade(int transparency, std::function<void()> callback) {
	std::thread([&, transparency, callback] {
		std::lock_guard<std::mutex> lock(s_instance->m_mutex);

		s_instance->m_parent->DisplayFade(transparency);
		while (static_cast<int>(s_instance->m_parent->GetDisplayFade()) != transparency) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		callback();
	}).detach();
}

void SceneManager::ExecuteAfter(int ms_time, std::function<void()> callback) {
	QueueInfo info = {};
	info.callback = callback;
	info.time = std::chrono::system_clock::now() + std::chrono::milliseconds(ms_time);

	auto thread_id = std::this_thread::get_id();
	if (thread_id == s_instance->m_renderId) {
		s_instance->m_queue_render.push_back(std::move(info));
	}
	else {
		s_instance->m_queue_input.push_back(std::move(info));
	}
}

void SceneManager::GameExecuteAfter(ExecuteThread thread, int ms_time, std::function<void()> callback) {
	Game* game = s_instance->m_parent;

	if (game->GetThreadMode() == ThreadMode::SINGLE_THREAD) {
		game->GetMainThread()->QueueAction(callback);
	}
	else {
		switch (thread) {
			case ExecuteThread::UPDATE: {
				game->GetRenderThread()->QueueAction(callback);
				break;
			}

			case ExecuteThread::WINDOW: {
				game->GetMainThread()->QueueAction(callback);
				break;
			}
		}
	}
}

void SceneManager::StopGame() {
	m_parent->Stop();
}

SceneManager* SceneManager::GetInstance() {
	if (s_instance == nullptr) {
		s_instance = new SceneManager();
	}

	return s_instance;
}

void SceneManager::Release() {
	if (s_instance != nullptr) {
		delete s_instance;
	}
}
