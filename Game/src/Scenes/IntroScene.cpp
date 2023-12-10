// TODO: add welcome message and stuff

#include "IntroScene.h"
#include "../Engine/SkinConfig.hpp"
#include <MsgBox.h>
#include <SDL2/SDL.h>
#include <fstream>
#include <thread>

#include "../GameScenes.h"
#include <Configuration.h>
#include <Inputs/Keys.h>
#include <Rendering/Vulkan/volk/volk.h>
#include <Rendering/Window.h>
#include <SceneManager.h>

#include <Imgui/ImguiUtil.h>
#include <Texture/MathUtils.h>

IntroScene::IntroScene()
{
}

void IntroScene::Render(double delta)
{
    SceneManager::ChangeScene(GameScene::MAINMENU);
}

void IntroScene::OnKeyDown(const KeyState &state)
{
}

bool IntroScene::Attach()
{
    return true;
}

bool IntroScene::Detach()
{
    return true;
}