#include "ResultScene.hpp"
#include "../../Engine/Imgui/ImGui.h"
#include "../../Engine/Imgui/ImguiUtil.hpp"
#include "../../Engine/AudioManager.hpp"
#include "../Resources/Configuration.hpp"
#include "../../Engine/SceneManager.hpp"
#include "../GameScenes.h"
#include "../Data/Chart.hpp"
#include "../EnvironmentSetup.hpp"

ResultScene::ResultScene() {
}

void ResultScene::Render(double delta) {
	ImguiUtil::NewFrame();

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
            if (ImGui::Button("Back")) {
                m_backButton = true;
            }

            ImGui::Text("Performance result");
            ImGui::EndMenuBar();
        }

        if (ImGui::BeginChild("#ResultState", ImVec2(0, 0), true)) {
            ImGui::Text("Your gameplay result!");

            std::string score = EnvironmentSetup::Get("Score");
            std::string cool = EnvironmentSetup::Get("Cool");
            std::string good = EnvironmentSetup::Get("Good");
            std::string bad = EnvironmentSetup::Get("Bad");
            std::string miss = EnvironmentSetup::Get("Miss");
            std::string jamCombo = EnvironmentSetup::Get("JamCombo");
            std::string maxJamCombo = EnvironmentSetup::Get("MaxJamCombo");
            std::string combo = EnvironmentSetup::Get("Combo");
            std::string maxCombo = EnvironmentSetup::Get("MaxCombo");
            std::string lnCombo = EnvironmentSetup::Get("LNCombo");
            std::string lnMaxCombo = EnvironmentSetup::Get("LNMaxCombo");

            ImGui::Text("Cool: %s", cool.c_str());
            ImGui::Text("Good: %s", good.c_str());
            ImGui::Text("Bad: %s", bad.c_str());
            ImGui::Text("Miss: %s", miss.c_str());

            ImGui::NewLine();
            ImGui::Text("Max LN Combo: %s", lnMaxCombo.c_str());
            ImGui::Text("Max Combo: %s", maxCombo.c_str());
            ImGui::Text("Max Jam: %s", maxJamCombo.c_str());

            ImGui::EndChild();
        }

        ImGui::End();
    }

    if (m_backButton) {
        if (EnvironmentSetup::GetPath("FILE").empty()) {
            SceneManager::ChangeScene(GameScene::MAINMENU);
        }
        else {
            SceneManager::GetInstance()->StopGame();
        }
    }
}

bool ResultScene::Attach() {
    m_backButton = false;

	Audio* audio = AudioManager::GetInstance()->Get("FINISH");
    if (!audio) {
        auto SkinName = Configuration::Load("Game", "Skin");
        auto BGMPath = Configuration::Skin_GetPath(SkinName) / "Audio";
        BGMPath /= "FINISH.ogg";

        if (std::filesystem::exists(BGMPath)) {
            AudioManager::GetInstance()->Create("FINISH", BGMPath, &audio);
        }
    }

    if (audio) {
        audio->SetVolume(50);
        audio->Play();
    }

	return true;
}

bool ResultScene::Detach() {
    Audio* audio = AudioManager::GetInstance()->Get("FINISH");
    if (audio) {
        audio->Stop();
    }

	return true;
}
