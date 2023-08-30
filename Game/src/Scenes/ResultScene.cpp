#include "ResultScene.hpp"


#include "SceneManager.h"
#include "Configuration.h"
#include "Rendering/Window.h"
#include "Texture/MathUtils.h"
#include "Audio/AudioManager.h"

#include "Imgui/ImGui.h"
#include "Imgui/ImguiUtil.h"
#include "Imgui/imgui_internal.h"

#include "Fonts/FontResources.h"

#include "../GameScenes.h"
#include "../Data/Chart.hpp"
#include "../EnvironmentSetup.hpp"

ResultScene::ResultScene() {
    
}

void ResultScene::Render(double delta) {
	ImguiUtil::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    auto window = GameWindow::GetInstance();

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

            if (ImGui::BeginChild("#Window1", MathUtil::ScaleVec2(ImVec2(200, 0)), true)) {
                ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0, 0));

                ImGui::Text("Score:");
                ImGui::PushFont(FontResources::GetButtonFont());
                ImGui::Button((std::to_string(score) + "###1").c_str(), MathUtil::ScaleVec2(ImVec2(192, 0)));
                ImGui::PopFont();

                if (ImGui::BeginChild("#Window2", MathUtil::ScaleVec2(ImVec2(95, 0)))) {
                    ImGui::Text("Cool");
                    ImGui::PushFont(FontResources::GetButtonFont());
                    ImGui::Button((std::to_string(cool) + "###2").c_str(), MathUtil::ScaleVec2(ImVec2(95, 0)));
                    ImGui::PopFont();

                    ImGui::Text("Bad");
                    ImGui::PushFont(FontResources::GetButtonFont());
                    ImGui::Button((std::to_string(bad) + "###3").c_str(), MathUtil::ScaleVec2(ImVec2(95, 0)));
                    ImGui::PopFont();

                    ImGui::Text("Max Jam Combo");
                    ImGui::PushFont(FontResources::GetButtonFont());
                    ImGui::Button((std::to_string(maxJamCombo) + "###4").c_str(), MathUtil::ScaleVec2(ImVec2(95, 0)));
                    ImGui::PopFont();

                    ImGui::EndChild();
                }

                ImGui::SameLine();

                if (ImGui::BeginChild("#Window3", MathUtil::ScaleVec2(ImVec2(95, 0)))) {
                    ImGui::Text("Good");
                    ImGui::PushFont(FontResources::GetButtonFont());
                    ImGui::Button((std::to_string(good) + "###5").c_str(), MathUtil::ScaleVec2(ImVec2(95, 0)));
                    ImGui::PopFont();

                    ImGui::Text("Miss");
                    ImGui::PushFont(FontResources::GetButtonFont());
                    ImGui::Button((std::to_string(miss) + "###6").c_str(), MathUtil::ScaleVec2(ImVec2(95, 0)));
                    ImGui::PopFont();

                    ImGui::Text("Max LN Combo");
                    ImGui::PushFont(FontResources::GetButtonFont());
                    ImGui::Button((std::to_string(lnMaxCombo) + "###7").c_str(), MathUtil::ScaleVec2(ImVec2(95, 0)));
                    ImGui::PopFont();

                    ImGui::EndChild();
                }

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
        GameWindow* window = GameWindow::GetInstance();

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
