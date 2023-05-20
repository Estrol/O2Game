#include "MyGame.h"
#include "GameScenes.h"
#include <fstream>

#include "../Engine/FontResources.hpp"

#include "./Resources/GameResources.hpp"
#include "./Resources/Configuration.hpp"
#include "./Data/Util/Util.hpp"
#include "./Data/MusicDatabase.h"
#include "EnvironmentSetup.hpp"

/* Scenes */
#include "./Scenes/GameplayScene.h"
#include "./Scenes/LoadingScene.h"
#include "./Scenes/SongSelectScene.h"
#include "./Scenes/IntroScene.hpp"
#include "./Scenes/ResultScene.hpp"


MyGame::~MyGame() {
	GameNoteResource::Dispose();
}

bool MyGame::Init() {
	SetRenderMode(RendererMode::DIRECTX);
	SetBufferSize(800, 600);
	SetWindowSize(1280, 720);

	{
		auto value = Configuration::Load("Game", "Window");
		auto split = splitString(value, 'x');

		if (split.size() == 2) {
			SetWindowSize(std::stoi(split[0]), std::stoi(split[1]));
		}
		else {
			MessageBoxA(NULL, "Invalid Game::Window configuration value!", "EstGame Error", MB_ICONERROR);
			return false;
		}
	}

	{
		auto value = Configuration::Load("Game", "Vulkan");
		if (value == "1") {
			SetRenderMode(RendererMode::VULKAN);
		}
	}

	{
		auto value = Configuration::Load("Game", "Skin");
		if (!Configuration::Skin_Exist(value)) {
			MessageBoxA(NULL, ("Skin: " + value + " is not found!").c_str(), "EstGame Error", MB_ICONERROR);
			return false;
		}

		auto resolution = Configuration::Skin_LoadValue(value, "Window", "NativeSize");
		auto split = splitString(resolution, 'x');

		if (split.size() == 2) {
			SetBufferSize(std::stoi(split[0]), std::stoi(split[1]));
		}
		else {
			MessageBoxA(NULL, "Invalid Skin::Window::NativeSize configuration value!", "EstGame Error", MB_ICONERROR);
			return false;
		}
	}

	bool result = Game::Init();
	if (result) {
		if (!GameNoteResource::Load()) return false;

		SceneManager::AddScene(GameScene::INTRO, new IntroScene());
		SceneManager::AddScene(GameScene::MAINMENU, new SongSelectScene());
		SceneManager::AddScene(GameScene::LOADING, new LoadingScene());
		SceneManager::AddScene(GameScene::RESULT, new ResultScene());
		SceneManager::AddScene(GameScene::GAME, new GameplayScene());

		std::string title = "Unnamed O2 Clone (Beta 3)";
		m_window->SetWindowTitle(title);

		if (EnvironmentSetup::GetPath("FILE").empty()) {
			SceneManager::ChangeScene(GameScene::INTRO);
		}
		else {
			SceneManager::ChangeScene(GameScene::LOADING);
		}
	}

	return result;
}

void MyGame::Run(double frameRate) {
	frameRate = std::clamp(frameRate, 15.0, 1500.0);

	Game::Run(frameRate);
}

void MyGame::SelectSkin(std::string name) {

}

void MyGame::Update(double delta) {
	m_sceneManager->Update(delta);
}

void MyGame::Render(double delta) {
	m_sceneManager->Render(delta);
}

void MyGame::Input(double delta) {
	m_sceneManager->Input(delta);
}
