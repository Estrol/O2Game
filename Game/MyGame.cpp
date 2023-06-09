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
	//SetFullscreen(true);
	
	{
		auto value = Configuration::Load("Game", "Renderer");
		if (value.size()) {
			try {
				int index = std::stoi(value.c_str());

				switch (index) {
					case 0:
						SetRenderMode(RendererMode::OPENGL);
						break;

					case 1:
						SetRenderMode(RendererMode::VULKAN);
						break;

					case 2:
						SetRenderMode(RendererMode::DIRECTX);
						break;

					case 3:
						SetRenderMode(RendererMode::DIRECTX11);
						break;

					case 4:
						SetRenderMode(RendererMode::DIRECTX12);
						break;

					default: {
						std::cout << "Invalid renderer mode: " << index << std::endl;
						SetRenderMode(RendererMode::OPENGL);
						break;
					}
				}
			}
			catch (std::invalid_argument& e) {
				std::cout << "Failed to parse Game.ini::Game::Renderer" << std::endl;
				SetRenderMode(RendererMode::OPENGL);
			}
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

	{
		auto value = Configuration::Load("Game", "Resolution");
		auto split = splitString(value, 'x');

		if (split.size() == 2) {
			SetWindowSize(std::stoi(split[0]), std::stoi(split[1]));
		}
		else {
			MessageBoxA(NULL, "Invalid Game::Resolution configuration value!", "EstGame Error", MB_ICONERROR);
			return false;
		}
	}

	bool result = Game::Init();
	if (result) {
		m_window->SetScaleOutput(true);

		SceneManager::AddScene(GameScene::INTRO, new IntroScene());
		SceneManager::AddScene(GameScene::MAINMENU, new SongSelectScene());
		SceneManager::AddScene(GameScene::LOADING, new LoadingScene());
		SceneManager::AddScene(GameScene::RESULT, new ResultScene());
		SceneManager::AddScene(GameScene::GAME, new GameplayScene());

		std::string title = "Unnamed O2 Clone (Beta 5)";
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
