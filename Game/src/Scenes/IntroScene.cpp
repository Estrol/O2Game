// TODO: add welcome message and stuff

#include "IntroScene.h"
#include <fstream>
#include <thread>
#include <MsgBox.h>
#include <SDL2/SDL.h>
#include "../Resources/SkinConfig.hpp"

#include <Rendering/Vulkan/volk/volk.h>
#include <SceneManager.h>
#include "../GameScenes.h"
#include <Configuration.h>
#include <Inputs/Keys.h>
#include <Rendering/Window.h>

IntroScene::IntroScene() {
	
}

void IntroScene::Render(double delta) {
	SceneManager::ChangeScene(GameScene::MAINMENU);
}

void IntroScene::OnKeyDown(const KeyState& state) {

}

bool IntroScene::Attach() {

	return true;
}

bool IntroScene::Detach() {
	return true;
}