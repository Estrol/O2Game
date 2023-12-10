#include "MyGame.h"
#include "GameScenes.h"
#include "MsgBox.h"
#include "Version.h"
#include <array>
#include <fstream>

#include "Configuration.h"
#include "Fonts/FontResources.h"

#include "./Data/Util/Util.hpp"
#include "./Engine/SkinManager.hpp"
#include "./Resources/GameDatabase.h"
#include "./Resources/GameResources.hpp"
#include "./Resources/MusicListMaker.h"
#include "EnvironmentSetup.hpp"

/* Scenes */
#include "./Scenes/EditorScene.h"
#include "./Scenes/GameplayScene.h"
#include "./Scenes/IntroScene.h"
#include "./Scenes/LoadingScene.h"
#include "./Scenes/MainMenu.h"
#include "./Scenes/ReloadScene.h"
#include "./Scenes/ResultScene.h"
#include "./Scenes/SongSelectScene.h"

/* Overlays */
#include "./Scenes/Overlays/Settings.h"

MyGame::~MyGame()
{
    GameDatabase::Release();
    EnvironmentSetup::OnExitCheck();
}

bool MyGame::Init()
{
    SetRenderMode(RendererMode::DIRECTX);
    SetBufferSize(800, 600);
    SetWindowSize(1280, 720);
    SetFullscreen(false);

    bool result = LoadConfiguration();
    if (!result) {
        return result;
    }

    result = Game::Init();
    if (result) {
        m_window->SetScaleOutput(true);

        EnvironmentSetup::SetInt("Key", -1);

        /* Screen */
        SceneManager::AddScene(GameScene::INTRO, new IntroScene());
        SceneManager::AddScene(GameScene::MAINMENU, new MainMenu());
        SceneManager::AddScene(GameScene::SONGSELECT, new SongSelectScene());
        SceneManager::AddScene(GameScene::LOADING, new LoadingScene());
        SceneManager::AddScene(GameScene::GAMEPLAY, new GameplayScene());
        SceneManager::AddScene(GameScene::RESULT, new ResultScene());

        SceneManager::AddScene(GameScene::EDITOR, new EditorScene());
        // SceneManager::AddScene(GameScene::MULTIPLAYER, new MultiplayerScene());
        // SceneManager::AddScene(GameScene::MULTIROOM, new MultiroomScene());
        SceneManager::AddScene(GameScene::RELOAD, new ReloadScene());

        /* Overlays */
        SceneManager::AddOverlay(GameOverlay::SETTINGS, new SettingsOverlay());

        std::string title = std::string(O2GAME_TITLE) + " " + std::string(O2GAME_VERSION);
        m_window->SetWindowTitle(title);

        if (EnvironmentSetup::GetPath("FILE").empty()) {
            std::filesystem::path path = Configuration::Load("Music", "Folder");
            if (!path.empty() && std::filesystem::exists(path)) {
                Configuration::Set("Music", "Folder", path.string());
            }

            SceneManager::ChangeScene(GameScene::INTRO);
        } else {
            SceneManager::ChangeScene(GameScene::LOADING);
        }
    }

    return result;
}

void MyGame::Run()
{
    Game::Run();
}

void MyGame::SelectSkin(std::string name)
{
}

bool MyGame::LoadConfiguration()
{
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

                    default:
                    {
                        std::cout << "Invalid renderer mode: " << index << std::endl;
                        SetRenderMode(RendererMode::OPENGL);
                        break;
                    }
                }
            } catch (const std::invalid_argument &) {
                std::cout << "Failed to parse Game.ini::Game::Renderer" << std::endl;
                SetRenderMode(RendererMode::OPENGL);
            }
        }
    }

    {
        auto SkinName = Configuration::Load("Game", "Skin");
        auto path = std::filesystem::current_path() / "Skins" / SkinName;
        if (!std::filesystem::exists(path)) {
            SkinName = "default";
        }

        Configuration::Font_SetPath(path);

        auto manager = SkinManager::GetInstance();
        manager->LoadSkin(SkinName);

        std::array<std::string, 3> screens = {
            "NativeSize"
        };

        for (int i = 0; i < 1; i++) {
            auto value = manager->GetSkinProp("Window", screens[i], "800x600");
            auto split = splitString(value, 'x');

            if (split.size() == 2) {
                FontResources::RegisterFontIndex(i, std::stoi(split[0]), std::stoi(split[1]));
            } else {
                std::string errmsg = "Invalid Skin::Window::" + screens[i] + " configuration value!";

                MsgBox::ShowOut("EstGame Error", errmsg, MsgBoxType::OK, MsgBoxFlags::BTN_ERROR);
                return false;
            }
        }
    }

    {
        auto value = Configuration::Load("Game", "Resolution");
        auto split = splitString(value, 'x');

        if (split.size() == 2) {
            SetWindowSize(std::stoi(split[0]), std::stoi(split[1]));
        } else {
            MsgBox::ShowOut("EstGame Error", "Invalid Game::Resolution configuration value!", MsgBoxType::OK, MsgBoxFlags::BTN_ERROR);
            return false;
        }
    }

    {
        FontResources::LoadFontRegion(TextRegion::Japanese);
        FontResources::LoadFontRegion(TextRegion::Chinese);
        FontResources::LoadFontRegion(TextRegion::Korean);
    }

    return true;
}

void MyGame::Update(double delta)
{
    if (ImGui::IsKeyPressed(ImGuiKey_F10, false)) {
        SceneManager::OverlayShow(GameOverlay::SETTINGS);
    }

    m_sceneManager->Update(delta);
}

void MyGame::Render(double delta)
{
    FontResources::SetFontIndex(0);
    m_sceneManager->Render(delta);
}

void MyGame::Input(double delta)
{
    m_sceneManager->Input(delta);
}
