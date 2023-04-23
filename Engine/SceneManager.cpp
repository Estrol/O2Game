#include "SceneManager.hpp"
#include "Scene.hpp"
#include "Game.hpp"

#include "framework.h"
#include <string>
#include <mutex>
#include <iostream>

SceneManager::SceneManager() {
	m_scenes = std::unordered_map<int, Scene*>();
	m_parent = nullptr;
}

SceneManager::~SceneManager() {
	for (auto& it : m_scenes) {
		it.second->Detach();
		delete it.second;
	}
}

SceneManager* SceneManager::s_instance = nullptr;

void SceneManager::Update(double delta) {
	if (m_nextScene != nullptr) {
		static std::mutex mutex;

		std::lock_guard<std::mutex> lock(mutex);

		if (!m_nextScene->Attach()) {
			MessageBoxA(NULL, "Failed to init next scene", "EstEngine Error", MB_ICONERROR);
			m_parent->Stop();
			return;
		}

		if (m_currentScene) {
			auto curScene = m_currentScene;
			m_currentScene = nullptr;

			if (!curScene->Detach()) {
				MessageBoxA(NULL, "Failed to detact current scene", "EstEngine Error", MB_ICONERROR);
				m_parent->Stop();
				return;
			}
		}

		m_currentScene = m_nextScene;
		m_nextScene = nullptr;
	}
	else {
		m_currentScene->Update(delta);
	}
}

void SceneManager::Render(double delta) {
	if (m_currentScene) m_currentScene->Render(delta);
}

void SceneManager::Input(double delta) {
	if (m_currentScene) m_currentScene->Input(delta);
}

void SceneManager::OnKeyDown(const KeyState& state) {
	if (m_currentScene) m_currentScene->OnKeyDown(state);
}

void SceneManager::OnKeyUp(const KeyState& state) {
	if (m_currentScene) m_currentScene->OnKeyUp(state);
}

void SceneManager::AddScene(int idx, Scene* scene) {
	if (s_instance == nullptr) throw std::exception("SceneManager is not initialized");

	std::cout << "Added scene: " << idx << std::endl;
	s_instance->m_scenes[idx] = scene;
}

void SceneManager::ChangeScene(int idx) {
	if (s_instance == nullptr) throw std::exception("SceneManager is not initialized");

	std::cout << "Change scene: " << idx << std::endl;
	s_instance->IChangeScene(idx);
}

void SceneManager::IAddScene(int idx, Scene* scene) {
	m_scenes[idx] = scene;
}

void SceneManager::IChangeScene(int idx) {
	if (m_scenes.find(idx) == m_scenes.end()) {
		std::string msg = "Failed to find SceneId: " + std::to_string(idx);
		
		MessageBoxA(NULL, msg.c_str(), "EstEngine Error", MB_ICONERROR);
		return;
	}

	m_nextScene = m_scenes[idx];
}

void SceneManager::SetParent(Game* parent) {
	m_parent = parent;
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
