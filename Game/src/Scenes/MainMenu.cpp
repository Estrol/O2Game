#include "MainMenu.h"
#include "Imgui/imgui.h"
#include "Imgui/ImguiUtil.h"
#include "Rendering/Window.h"
#include "Texture/MathUtils.h"
#include "Configuration.h"
#include <filesystem>
#include <MsgBox.h>
#include "Fonts/FontResources.h"
#include <SceneManager.h>
#include "../GameScenes.h"
#include <Audio/AudioManager.h>
#include "../Version.h"
#include "../Engine/SkinManager.hpp"

MainMenu::MainMenu() {

}

void MainMenu::Update(double delta) {

}

void MainMenu::Render(double delta) {
    ImguiUtil::NewFrame();

    auto window = GameWindow::GetInstance();
    ImGui::SetNextWindowPos(ImVec2(0, 0));

    auto windowNextSz = ImVec2((float)window->GetBufferWidth(), (float)window->GetBufferHeight());
    ImGui::SetNextWindowSize(MathUtil::ScaleVec2(windowNextSz));
    ImGui::SetWindowPos(ImVec2(0, 0));

    if (m_background) {
        m_background->Draw();
    }

    auto flags = ImGuiWindowFlags_NoTitleBar
        | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoScrollbar
        | ImGuiWindowFlags_NoScrollWithMouse
        | ImGuiWindowFlags_MenuBar
        | ImGuiWindowFlags_NoBringToFrontOnFocus;

    int nextScene = -1;
    if (ImGui::Begin("###BEGIN", nullptr, flags)) {
        if (ImGui::BeginMenuBar()) {
            std::string title = std::string(O2GAME_TITLE) + " " + std::string(O2GAME_VERSION);
            ImGui::Text("%s", title.c_str());

            std::string text = "No Account!";
			auto textWidth = ImGui::CalcTextSize(text.c_str()).x;

			ImGui::SameLine(MathUtil::ScaleVec2(ImVec2(windowNextSz.x, 0)).x - textWidth - 15);
			ImGui::Text(text.c_str());

            ImGui::EndMenuBar();
        }

        {
			ImFont* font = FontResources::GetButtonFont();
            ImGui::PushFont(font);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
            auto ButtonColor = ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);
            ButtonColor.w = 0.25f;
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ButtonColor);
            ButtonColor.w = 0.15f;
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ButtonColor);
            ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.0f, 0.5f));

            auto ButtonSize = MathUtil::ScaleVec2(window->GetBufferWidth(), 40);

            for (int i = 0; i < 5; i++) {
                ImGui::NewLine();
            }

            ImGui::Spacing();
            if (ImGui::Button("Single player", ButtonSize)) {
                nextScene = 0;
            }

            ImGui::Spacing();
            if (ImGui::Button("Multi player", ButtonSize)) {
                MsgBox::Show("Multiplayer", "Error", "Multiplayer is not implemented yet!");
            }

            ImGui::NewLine();
            ImGui::Spacing();
            if (ImGui::Button("Editor", ButtonSize)) {
                nextScene = 2;
            }

			ImGui::NewLine();
            ImGui::Spacing();
			if (ImGui::Button("Options", ButtonSize)) {
                nextScene = 3;
            }

            ImGui::Spacing();
            if (ImGui::Button("Quit", ButtonSize)) {
                nextScene = 4;
            }
			
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(3);
			ImGui::PopFont();
        }

        ImGui::End();
    }

    if (nextScene != -1) {
        switch (nextScene) {
            case 0: {
                SceneManager::DisplayFade(100, [this]() {
                    SceneManager::ChangeScene(GameScene::SONGSELECT);
                });
                break;
            }

            case 1: {
                break;
            }

            case 2: {
                SceneManager::DisplayFade(100, [this]() {
                    SceneManager::ChangeScene(GameScene::EDITOR);
                });
                break;
            }

            case 3: {
                SceneManager::OverlayShow(GameOverlay::SETTINGS);
                break;
            }

            case 4: {
                SDL_Event e = {};
				e.type = SDL_QUIT;

				SDL_PushEvent(&e);
                break;
            }
        }
    }
}

bool MainMenu::Attach() {
    SkinManager::GetInstance()->ReloadSkin();

    auto path = SkinManager::GetInstance()->GetPath();
    auto window = GameWindow::GetInstance();

    auto background_path = path / "Menu" / "MenuBackground.png";
    auto bgm_path = path / "Audio" / "BGM.ogg";

    if (std::filesystem::exists(background_path)) {
        m_background = std::make_unique<Texture2D>(background_path);
        m_background->Size = UDim2::fromOffset(window->GetBufferWidth(), window->GetBufferHeight());
    }

    Audio* bgm = AudioManager::GetInstance()->Get("BGM");
    if (bgm == nullptr || !bgm->IsPlaying()) {
        if (!bgm) {
            if (std::filesystem::exists(bgm_path)) {
                AudioManager::GetInstance()->Create("BGM", bgm_path, &bgm);
            }
        }

        if (bgm && !bgm->IsPlaying()) {
            bgm->SetVolume(50);
            bgm->Play(0, true);
        }
    }

    SceneManager::DisplayFade(0, [] {});
    return true;
}

bool MainMenu::Detach() {
    if (m_background) {
        m_background.reset();
    }

    if (SceneManager::GetInstance()->GetCurrentSceneIndex() != GameScene::SONGSELECT) {
        Audio* bgm = AudioManager::GetInstance()->Get("BGM");

        if (bgm) {
            bgm->Stop();
        }
    }

    return true;
}