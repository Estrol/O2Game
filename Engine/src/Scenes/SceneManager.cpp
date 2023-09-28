#include "SceneManager.h"
#include "Scene.h"
#include "Game.h"
#include "MsgBox.h"
#include "Overlay.h"
#include "Imgui/ImguiUtil.h"

#include <condition_variable>
#include <string>
#include <mutex>
#include <iostream>
#include <exception>

static std::condition_variable m_cv;
static bool m_ready_change_state = false;

SceneManager::SceneManager() {
	m_scenes = {};
	m_overlays = {};
	m_parent = nullptr;
}

SceneManager::~SceneManager() {
	for (auto& it : m_scenes) {
		it.second->Detach();
	}

	for (auto& it : m_overlays) {
		it.second->Detach();
	}

	m_scenes.clear();
	m_overlays.clear();
}

SceneManager* SceneManager::s_instance = nullptr;

void SceneManager::Update(double delta) {
	if (m_nextScene != nullptr) {
		//std::lock_guard<std::mutex> lock(m_mutex);
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

	if (m_nextOverlay != nullptr) {
		//std::lock_guard<std::mutex> lock(m_mutex);

		if (!m_nextOverlay->Attach()) {
			MsgBox::ShowOut("EstEngine Error", "Failed to init next overlay", MsgBoxType::OK, MsgBoxFlags::BTN_ERROR);
			m_parent->Stop();
			return;
		}

		if (m_currentOverlay) {
			auto curOverlay = m_currentOverlay;
			m_currentOverlay = nullptr;

			if (!curOverlay->Detach()) {
				MsgBox::ShowOut("EstEngine Error", "Failed to detach current overlay", MsgBoxType::OK, MsgBoxFlags::BTN_ERROR);
				m_parent->Stop();
				return;
			}
		}

		m_currentOverlay = m_nextOverlay;
		m_nextOverlay = nullptr;
	} else {
		if (m_currentOverlay) {
			m_currentOverlay->Update(delta);
		}
	}
}

void SceneManager::Render(double delta) {
	m_renderId = std::this_thread::get_id();
	m_ready_change_state = false;

	if (m_currentScene) m_currentScene->Render(delta);
	if (m_currentOverlay && !MsgBox::Any()) {
		ImguiUtil::NewFrame();
		auto& io = ImGui::GetIO();

		ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		ImGui::SetNextWindowSize(m_currentOverlay->GetSize(), ImGuiCond_Always);

		std::string title = m_currentOverlay->GetName() + "###SceneManagerOverlay";
		ImGui::OpenPopup(title.c_str());
		
		if (ImGui::BeginPopupModal(
			title.c_str(), 
			nullptr, 
			ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize)) {

			m_currentOverlay->Render(delta);
			ImGui::EndPopup();
		}

		if (m_currentOverlay->IsClosed()) {
			if (!m_currentOverlay->Detach()) {
				MsgBox::ShowOut("EstEngine Error", "Failed to detach current overlay", MsgBoxType::OK, MsgBoxFlags::BTN_ERROR);
				m_parent->Stop();
				return;
			}

			m_currentOverlay = nullptr;
		}
	}

	if (m_nextScene != nullptr) {
		m_ready_change_state = true;
		m_cv.notify_one();
	}
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
	s_instance->m_scenes[idx] = std::unique_ptr<Scene>(scene);
}

void SceneManager::ChangeScene(int idx) {
	if (s_instance == nullptr) throw std::runtime_error(notInitialized);
	
	s_instance->IChangeScene(idx);
}

void SceneManager::AddOverlay(int Idx, Overlay* overlay) {
	if (s_instance == nullptr) throw std::runtime_error(notInitialized);

	s_instance->m_overlays[Idx] = std::unique_ptr<Overlay>(overlay);
}

void SceneManager::OverlayShow(int idx) {
	auto& map = s_instance->m_overlays;

	if (map.find(idx) == map.end()) {
		std::string msg = "Failed to find OverlayId: " + std::to_string(idx);

		MsgBox::ShowOut("EstEngine Error", msg.c_str(), MsgBoxType::OK, MsgBoxFlags::BTN_ERROR);
		return;
	}

	s_instance->m_nextOverlay = map[idx].get();
}

void SceneManager::OverlayClose() {
	if (s_instance == nullptr) throw std::runtime_error(notInitialized);

	if (s_instance->m_currentOverlay) {
		s_instance->m_currentOverlay->Detach();
		s_instance->m_currentOverlay = nullptr;
	}
}

void SceneManager::IAddScene(int idx, Scene* scene) {
	m_scenes[idx] = std::unique_ptr<Scene>(scene);
}

void SceneManager::IChangeScene(int idx) {
	if (m_scenes.find(idx) == m_scenes.end()) {
		std::string msg = "Failed to find SceneId: " + std::to_string(idx);
		
		MsgBox::ShowOut("EstEngine Error", msg.c_str(), MsgBoxType::OK, MsgBoxFlags::BTN_ERROR);
		return;
	}

	m_lastSceneId = m_currentSceneId;
	m_currentSceneId = idx;

	m_nextScene = m_scenes[idx].get();
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

int SceneManager::GetCurrentSceneIndex() const {
	return m_currentSceneId;
}

int SceneManager::GetLastSceneIndex() const {
	return m_lastSceneId;
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
