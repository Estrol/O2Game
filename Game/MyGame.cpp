#include "MyGame.h"
#include "GameScenes.h"

/* Scenes */
#include "GameplayScene.h"
#include "LoadingScene.h"

#include "Resources/GameResources.hpp"

MyGame::~MyGame() {
	GamePlayingResource::Dispose();
	GameNoteResource::Dispose();
}

bool MyGame::Init() {
	bool result = Game::Init();
	if (result) {
		if (!GamePlayingResource::Load()) return false;
		if (!GameNoteResource::Load()) return false;

		SceneManager::AddScene(GameScene::INTRO, new LoadingScene());
		SceneManager::AddScene(GameScene::GAME, new GameplayScene());
		SceneManager::ChangeScene(GameScene::INTRO);
	}

	return result;
}

void MyGame::Run(double frameRate) {
	Game::Run(frameRate);
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
