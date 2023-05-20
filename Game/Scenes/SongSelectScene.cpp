#include "SongSelectScene.h"
#include <iostream>

#include "../../Engine/SceneManager.hpp"
#include "../../Engine/Keys.h"

#include "../Resources/Configuration.hpp"
#include "../Resources/SkinConfig.hpp"
#include "../Data/MusicDatabase.h"

#include "../EnvironmentSetup.hpp"
#include "../GameScenes.h"
#include "../../Engine/Window.hpp"
#include "../../Engine/Imgui/ImguiUtil.hpp"
#include "../../Engine/AudioManager.hpp"
#include "../Data/Chart.hpp"
#include "../../Engine/MsgBox.hpp"

#define SAFE_DELETE(x) if (x) { delete x; x = nullptr; }

SongSelectScene::SongSelectScene() {
    m_text = nullptr;
    m_songSelect = nullptr;
}

void SongSelectScene::Render(double delta) {
    ImguiUtil::NewFrame();

    auto music = MusicDatabase::GetInstance();
	
    bool bPlay = false;
    bool bExitPopup = false;
    bool bOptionPopup = false;

    ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(800, 600));

    if (ImGui::Begin("#SongSelectMenuBar",
        nullptr,
        ImGuiWindowFlags_NoTitleBar
        | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoScrollbar
        | ImGuiWindowFlags_NoScrollWithMouse
        | ImGuiWindowFlags_MenuBar
    )) {
        if (ImGui::BeginMenuBar()) {
            if (ImGui::Button("Exit")) {
                bExitPopup = true;
            }

            if (ImGui::Button("Options")) {
                bOptionPopup = true;
            }

            ImGui::Text("Song Select");
            ImGui::EndMenuBar();
        }
		
        const int MAX_ROWS = 19;
	
        // create child window
        if (ImGui::BeginChild("#SongSelectChild", ImVec2(400, 495), true)) {
            if (ImGui::BeginChild("#SongSelectChild2", ImVec2(400, 450))) {
                ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0, 0));
                for (int i = 0; i < MAX_ROWS; i++) {
                    std::string Id = "Button#" + std::to_string(i);

                    if (i + page < music->GetMusicCount()) {
                        DB_MusicItem& item = music->GetMusicItem(i + page);
                        ImGui::PushID(Id.c_str());
                        if (ImGui::Button((const char*)item.Title, ImVec2(500, 20))) {
                            index = i + page;
                        }
                        ImGui::PopID();
                    }
                }
                ImGui::PopStyleVar();
                ImGui::EndChild();
            }

            if (ImGui::Button("Previous Page", ImVec2(100, 0))) {
                page = std::clamp(page - MAX_ROWS, 0, 999999);
            }

            ImGui::SameLine();

            if (ImGui::Button("Next Page", ImVec2(100, 0))) {
                if (page + MAX_ROWS < music->GetMusicCount()) {
                    page = std::clamp(page + MAX_ROWS, 0, 999999);
                }
            }

            ImGui::EndChild();
        }

        ImGui::SameLine();
        if (ImGui::BeginChild("#Container1", ImVec2(350, 500))) {
            if (ImGui::BeginChild("#SongSelectChild2", ImVec2(350, 200), true)) {
                if (index != -1) {
                    DB_MusicItem& item = music->GetMusicItem(index);

                    ImGui::Text("Title\r");
                    ImGui::SameLine();
                    ImGui::Text((const char*)item.Title);

                    ImGui::Text("Artist\r");
                    ImGui::SameLine();
                    ImGui::Text((const char*)item.Artist);

                    ImGui::Text("Notecharter\r");
                    ImGui::SameLine();
                    ImGui::Text((const char*)item.Noter);

                    ImGui::Text("Note count\r");
                    ImGui::SameLine();

                    std::string count = std::to_string(item.MaxNotes[2]);
                    ImGui::Text(count.c_str());

                    std::string currentDifficulty = EnvironmentSetup::Get("Difficulty");

                    {
                        if (currentDifficulty == "0") {
                            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
                        }

                        if (ImGui::Button("EX", ImVec2(50, 0))) {
                            EnvironmentSetup::Set("Difficulty", "0");
                        }

                        if (currentDifficulty == "0") {
                            ImGui::PopItemFlag();
                            ImGui::PopStyleVar();
                        }
                    }

                    ImGui::SameLine();

                    {
                        if (currentDifficulty == "1") {
                            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
                        }

                        if (ImGui::Button("NX", ImVec2(50, 0))) {
                            EnvironmentSetup::Set("Difficulty", "1");
                        }

                        if (currentDifficulty == "1") {
                            ImGui::PopItemFlag();
                            ImGui::PopStyleVar();
                        }
                    }

                    ImGui::SameLine();

                    {
                        if (currentDifficulty == "2") {
                            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
                        }

                        if (ImGui::Button("HX", ImVec2(50, 0))) {
                            EnvironmentSetup::Set("Difficulty", "2");
                        }

                        if (currentDifficulty == "2") {
                            ImGui::PopItemFlag();
                            ImGui::PopStyleVar();
                        }
                    }

                    ImGui::Spacing();

                    ImGui::Text("Notespeed: ");
                    ImGui::SameLine();
                    ImGui::SliderFloat("##Notespeed", &currentSpeed, 0.1f, 4.0f, "%.2f");

                    ImGui::Text("Rate: ");
                    ImGui::SameLine();
                    ImGui::SliderFloat("##Rate", &currentRate, 0.5f, 2.0f, "%.2f");

                    {
                        std::string value = EnvironmentSetup::Get("Mirror");
                        if (value == "1") {
                            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.9f);
                        }

                        if (ImGui::Button("Mirror", ImVec2(80, 0))) {

                        }

                        if (value == "1") {
                            ImGui::PopStyleVar();
                        }
                    }

                    ImGui::SameLine();

                    {
                        std::string value = EnvironmentSetup::Get("Random");
                        if (value == "1") {
                            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.9f);
                        }

                        if (ImGui::Button("Random", ImVec2(80, 0))) {

                        }

                        if (value == "1") {
                            ImGui::PopStyleVar();
                        }
                    }
                }

                ImGui::EndChild();
            }

			if (ImGui::BeginChild("#test2", ImVec2(350, 290), true)) {
				ImGui::Text("test2");
				ImGui::EndChild();
			}

            ImGui::EndChild();
        }

        if (ImGui::Button("Play", ImVec2(100, 0))) {
            bPlay = true;
        }

        ImGui::End();
    }

    if (bExitPopup) {
        SendMessageA(Window::GetInstance()->GetHandle(), WM_CLOSE, 0, 0);
    }

    if (bOptionPopup) {
        ImGui::OpenPopup("Options###OptionsId");
    }

    if (bPlay && index == -1) {
        MsgBox::Show("MustSelectSong", "Error", "You must select a song to play!", MsgBoxType::OK);
    }

    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("Options###OptionsId", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize)) {
        auto bWaiting = EnvironmentSetup::Get("bWaiting");
        if (bWaiting == "1") {
            auto key = EnvironmentSetup::Get("iKey");
			ImGui::Text("Waiting for keybind input...");
            ImGui::Text(("Press any key to set the Key: " + key).c_str());

            std::map<ImGuiKey, bool> blacklistedKey = {
                // Space, Enter
				{ ImGuiKey_Space, true },
				{ ImGuiKey_Enter, true },
				{ ImGuiKey_KeyPadEnter, true },
            };

			for (int i = 0; i < IM_ARRAYSIZE(io.KeysDown); i++) {
				if (io.KeysDown[i] && !blacklistedKey[(ImGuiKey)i]) {
                    EnvironmentSetup::Set("bWaiting", "0");

                    auto ikey = SDL_GetKeyFromScancode((SDL_Scancode)i);
                    std::string name = SDL_GetKeyName(ikey);
                    
                    Configuration::Set("KeyMapping", "Lane" + key, name);
                    break;
				}
			}
        }
        else {
            ImGui::Text("Keyboard configuration");
            for (int i = 0; i < 7; i++) {
                if (i != 0) {
                    ImGui::SameLine();
                }

                std::string currentKey = Configuration::Load("KeyMapping", "Lane" + std::to_string(i + 1));
                if (ImGui::Button((currentKey + "###KEY" + std::to_string(i)).c_str(), ImVec2(60, 0))) {
                    EnvironmentSetup::Set("bWaiting", "1");
                    EnvironmentSetup::Set("iKey", std::to_string(i + 1));
                }
            }

            int GraphicsIndex = 0;
            int nextGraphicsIndex = -1;
            try {
                GraphicsIndex = std::atoi(Configuration::Load("Game", "Vulkan").c_str());
            }
            catch (std::invalid_argument) {

            }

            ImGui::NewLine();

            std::vector<std::string> list = { "DirectX", "Vulkan (DXVK)" };
            ImGui::Text("Graphics");
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "You need restart game after setting this!");
            if (ImGui::BeginCombo("###ComboBox1", list[GraphicsIndex].c_str())) {
                for (int i = 0; i < list.size(); i++) {
                    bool isSelected = (GraphicsIndex == i);

                    if (ImGui::Selectable(list[i].c_str(), &isSelected)) {
                        nextGraphicsIndex = i;
                    }

                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }

                ImGui::EndCombo();
            }

            if (nextGraphicsIndex != -1 && nextGraphicsIndex != GraphicsIndex) {
                Configuration::Set("Game", "Vulkan", std::to_string(nextGraphicsIndex));
            }

            std::vector<std::string> fps = { "30", "60", "75", "120", "144", "360", "Unlimited" };
            int fpsIndex = 0;
            int nextFpsIndex = -1;

            try {
                fpsIndex = std::atoi(Configuration::Load("Game", "FPS").c_str());
            }
            catch (std::invalid_argument) {

            }

            ImGui::NewLine();

            ImGui::Text("FPS");
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Warning: setting unlimited FPS can cause PC to unstable!");
            if (ImGui::BeginCombo("###ComboBox2", fps[fpsIndex].c_str())) {
                for (int i = 0; i < fps.size(); i++) {
                    bool isSelected = (fpsIndex == i);

                    if (ImGui::Selectable(fps[i].c_str(), &isSelected)) {
                        nextFpsIndex = i;
                    }

                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }

                ImGui::EndCombo();
            }

            ImGui::NewLine();

            static int audioOffset = 0;

            try {
                audioOffset = std::atoi(Configuration::Load("Game", "AudioOffset").c_str());
            }
            catch (std::invalid_argument) {

            }

            ImGui::Text("Audio offset");
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Warning: this will make keysounded sample as auto sample!");
			ImGui::SliderInt("###Slider1", &audioOffset, -500, 500);

            ImGui::NewLine();

            ImGui::Button("Reset###Setting1", ImVec2(50, 0));

            ImGui::SameLine();

            if (ImGui::Button("Close###Setting2", ImVec2(50, 0))) {
                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::EndPopup();
    }

    if (bPlay && index != -1 && !is_departing) {
        is_departing = true;
        SaveConfiguration();

        DB_MusicItem& item = music->GetMusicItem(index);

        EnvironmentSetup::Set("Key", std::to_string(item.Id));
        SceneManager::ChangeScene(GameScene::LOADING);
    }

    if (is_quit) {
        SaveConfiguration();
        SceneManager::GetInstance()->StopGame();
    }
}

void SongSelectScene::Input(double delta) {
	
}

void SongSelectScene::OnMouseDown(const MouseState& state) {

}

bool SongSelectScene::Attach() {
    if (EnvironmentSetup::Get("Difficulty").size() == 0) {
        EnvironmentSetup::Set("Difficulty", "0");
    }

    Audio* bgm = AudioManager::GetInstance()->Get("BGM");
    if (!bgm) {
        auto SkinName = Configuration::Load("Game", "Skin");
        auto BGMPath = Configuration::Skin_GetPath(SkinName) / "Audio";
		BGMPath /= "BGM.ogg";

        if (std::filesystem::exists(BGMPath)) {
            AudioManager::GetInstance()->Create("BGM", BGMPath, &bgm);
        }
    }

    if (bgm) {
        bgm->SetVolume(50);
        bgm->Play(0, true);
    }

    auto rateValue = EnvironmentSetup::Get("SongRate");
    auto noteValue = Configuration::Load("Gameplay", "Notespeed");

    try {
        currentRate = std::stof(rateValue.c_str());
    }
    catch (std::invalid_argument) {
        currentRate = 1.0f;
    }

    try {
        int value = std::atoi(noteValue.c_str());

		noteValue = std::clamp(static_cast<float>(value) / 100.0f, 0.1f, 4.0f);
	} 
    catch (std::invalid_argument) {
        noteValue = 2.2f;
    }

    index = -1;
    is_departing = false;
    return true;
}

bool SongSelectScene::Detach() {
    Audio* bgm = AudioManager::GetInstance()->Get("BGM");
    if (bgm) {
        bgm->Stop();
    }

    return true;
}

void SongSelectScene::SaveConfiguration() {
    EnvironmentSetup::Set("SongRate", std::to_string(currentRate));
    Configuration::Set("Gameplay", "Notespeed", std::to_string(static_cast<int>(currentSpeed * 100.0)));
}
