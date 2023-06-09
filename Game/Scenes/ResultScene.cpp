#include "ResultScene.hpp"
#include "../../Engine/Imgui/ImGui.h"
#include "../../Engine/Imgui/ImguiUtil.hpp"
#include "../../Engine/AudioManager.hpp"
#include "../Resources/Configuration.hpp"
#include "../../Engine/SceneManager.hpp"
#include "../GameScenes.h"
#include "../Data/Chart.hpp"
#include "../EnvironmentSetup.hpp"
#include "../../Engine/MathUtils.hpp"
#include "../../Engine/Window.hpp"
#include "../../Engine/Imgui/imgui_internal.h"

ResultScene::ResultScene() {
}

void ResultScene::Render(double delta) {
	ImguiUtil::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    auto window = Window::GetInstance();

    auto windowNextSz = ImVec2(window->GetBufferWidth(), window->GetBufferHeight());
    ImGui::SetNextWindowSize(MathUtil::ScaleVec2(windowNextSz));

    if (m_background) {
        m_background->Draw();
    }

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
            if (ImGui::Button("Back", MathUtil::ScaleVec2(ImVec2(50, 0)))) {
                m_backButton = true;
            }

            ImGui::Text("Performance result");
            ImGui::EndMenuBar();
        }

        if (ImGui::BeginChild("#ResultState", MathUtil::ScaleVec2(ImVec2(0, 0)), true)) {

            int score = EnvironmentSetup::GetInt("Score");
            int cool = EnvironmentSetup::GetInt("Cool");
            int good = EnvironmentSetup::GetInt("Good");
            int bad = EnvironmentSetup::GetInt("Bad");
            int miss = EnvironmentSetup::GetInt("Miss");
            int jamCombo = EnvironmentSetup::GetInt("JamCombo");
            int maxJamCombo = EnvironmentSetup::GetInt("MaxJamCombo");
            int combo = EnvironmentSetup::GetInt("Combo");
            int maxCombo = EnvironmentSetup::GetInt("MaxCombo");
            int lnCombo = EnvironmentSetup::GetInt("LNCombo");
            int lnMaxCombo = EnvironmentSetup::GetInt("LNMaxCombo");

			// imgui set cursor pos to mid
            auto size = MathUtil::ScaleVec2(ImVec2(250, 0));
            auto center = ImGui::GetMainViewport()->GetCenter();
            ImGui::SetCursorPosX(center.x - (size.x / 2));

            if (ImGui::BeginChild("#ResultState2", size, true)) {
                ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0, 0));

				ImGui::Text("Score");
				ImGui::Button((std::to_string(score) + "###Score1").c_str(), size);

				ImGui::Text("Cool");
				ImGui::Button((std::to_string(cool) + "###Cool1").c_str(), size);

				ImGui::Text("Good");
				ImGui::Button((std::to_string(good) + "###Good1").c_str(), size);

				ImGui::Text("Bad");
				ImGui::Button((std::to_string(bad) + "###Bad1").c_str(), size);

				ImGui::Text("Miss");
				ImGui::Button((std::to_string(miss) + "###Miss1").c_str(), size);

				ImGui::Text("Jam Combo");
				ImGui::Button((std::to_string(jamCombo) + "###JamCombo1").c_str(), size);

				ImGui::Text("Max Jam Combo");
				ImGui::Button((std::to_string(maxJamCombo) + "###MaxJamCombo1").c_str(), size);

				ImGui::Text("Combo");
				ImGui::Button((std::to_string(combo) + "###Combo1").c_str(), size);

				ImGui::Text("Max Combo");
				ImGui::Button((std::to_string(maxCombo) + "###MaxCombo1").c_str(), size);

				ImGui::Text("LongNote Combo");
				ImGui::Button((std::to_string(lnCombo) + "###LNCombo1").c_str(), size);

				ImGui::Text("Max LongNote Combo");
				ImGui::Button((std::to_string(lnMaxCombo) + "###LNMaxCombo1").c_str(), size);
				
                ImGui::PopItemFlag();
                ImGui::PopStyleVar();

                ImGui::EndChild();
            }

            ImGui::EndChild();
        }

        ImGui::End();
    }

    if (m_backButton) {
        if (EnvironmentSetup::GetPath("FILE").empty()) {
            SceneManager::DisplayFade(100, [] {
                SceneManager::ChangeScene(GameScene::MAINMENU); 
            });
        }
        else {
            SceneManager::GetInstance()->StopGame();
        }
    }
}

bool ResultScene::Attach() {
    SceneManager::DisplayFade(0, [] {});
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

    Chart* chart = (Chart*)EnvironmentSetup::GetObj("SONG");
    EnvironmentSetup::SetObj("SONG", nullptr);

    if (chart->m_backgroundBuffer.size() > 0 && m_background == nullptr) {
        Window* window = Window::GetInstance();

        m_background = std::make_unique<Texture2D>((uint8_t*)chart->m_backgroundBuffer.data(), chart->m_backgroundBuffer.size());
        m_background->Size = UDim2::fromOffset(window->GetBufferWidth(), window->GetBufferHeight());
    }

    delete chart;

	return true;
}

bool ResultScene::Detach() {
    Audio* audio = AudioManager::GetInstance()->Get("FINISH");
    if (audio) {
        audio->Stop();
    }

    m_background.reset();
	return true;
}
