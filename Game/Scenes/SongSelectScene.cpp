#include "SongSelectScene.h"
#include <iostream>

#include "../../Engine/SceneManager.hpp"
#include "../../Engine/Keys.h"

#include "../../Engine/Configuration.hpp"
#include "../Resources/SkinConfig.hpp"
#include "../Data/MusicDatabase.h"

#include "../EnvironmentSetup.hpp"
#include "../GameScenes.h"
#include "../../Engine/Window.hpp"
#include "../../Engine/Imgui/ImguiUtil.hpp"
#include "../../Engine/Imgui/imgui.h"
#include "../../Engine/AudioManager.hpp"
#include "../Data/Chart.hpp"
#include "../../Engine/MsgBox.hpp"
#include "../../Engine/Imgui/imgui_internal.h"
#include "../../Engine/MathUtils.hpp"
#include "../Data/Util/Util.hpp"
#include "../../Engine/FontResources.hpp"
#include "../../Engine/SDLException.hpp"

#define SAFE_DELETE(x) if (x) { delete x; x = nullptr; }

SongSelectScene::SongSelectScene() {
    index = -1;
}

SongSelectScene::~SongSelectScene() {
    if (m_bgm) {
        m_bgm.reset();
    }

    m_songBackground.reset();
    m_background.reset();
}

void SongSelectScene::Render(double delta) {
    ImguiUtil::NewFrame();

    auto music = MusicDatabase::GetInstance();
    auto window = Window::GetInstance();
	
    bool bPlay = false;
    bool bExitPopup = false;
    bool bOptionPopup = false;
    bool bSelectNewSong = false;
    bool bOpenSongContext = false;
    bool bOpenEditor = false;
    bool bOpenRearrange = false;
    bool bScaleOutput = window->IsScaleOutput();

    ImGui::SetNextWindowPos(ImVec2(0, 0));

    auto windowNextSz = ImVec2(window->GetBufferWidth(), window->GetBufferHeight());

    if (m_songBackground) {
        m_songBackground->Draw();
    }
    else {
        if (m_background) {
            m_background->Draw();
        }
    }

    /*image->Position = UDim2::fromOffset(0, 0);
    image->Size = UDim2::fromOffset(22, 4);
    image->Draw();
    image->Position = UDim2::fromOffset(0, 10);
    image->Draw();
    image->Position = UDim2::fromOffset(60, 40);
    image->Draw();
    image->Position = UDim2::fromOffset(40, 30);
    image->Draw();
    image->Position = UDim2::fromOffset(60, 40);
    image->Draw();*/

	//ImGui::GetForegroundDrawList()->AddRectFilled(ImVec2(0, 0), ImVec2(window->GetWidth(), window->GetHeight()), ImColor(0, 0, 0, 200));
	ImGui::SetNextWindowSize(MathUtil::ScaleVec2(windowNextSz));

    ImGui::GetStyle().DisabledAlpha = std::clamp(currentAlpha / 100.0, 0.00000001, 1.0);
    auto flags = ImGuiWindowFlags_NoTitleBar
        | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoScrollbar
        | ImGuiWindowFlags_NoScrollWithMouse
        | ImGuiWindowFlags_MenuBar;

    ImGui::BeginDisabled(is_departing);

    if (ImGui::Begin("#SongSelectMenuBar",
        nullptr,
        flags
    )) {
        if (ImGui::BeginMenuBar()) {
            if (ImGui::Button("Exit", MathUtil::ScaleVec2(ImVec2(50, 0)))) {
                bExitPopup = true;
            }

            if (ImGui::Button("Options", MathUtil::ScaleVec2(ImVec2(50, 0)))) {
                bOptionPopup = true;
            }

            ImGuiIO& io = ImGui::GetIO();

            ImGui::Text("Song Select");

            std::string text = "No Account!";
			auto textWidth = ImGui::CalcTextSize(text.c_str()).x;

			ImGui::SameLine(MathUtil::ScaleVec2(ImVec2(windowNextSz.x, 0)).x - textWidth - 15);
			ImGui::Text(text.c_str());

            ImGui::EndMenuBar();
        }
	
        // create child window
        if (ImGui::BeginChild("#Container1", MathUtil::ScaleVec2(ImVec2(200, 500)))) {
            if (ImGui::BeginChild("#SongSelectChild2", MathUtil::ScaleVec2(ImVec2(200, 200)), true)) {
                ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0, 0));

                DB_MusicItem* item = music->Find(index);
                ImGui::Text("Title\r");
                ImGui::Button((const char*)(index != -1 ? item->Title : u8"###EMPTY1"), MathUtil::ScaleVec2(ImVec2(340, 0)));

                ImGui::Text("Artist\r");
                ImGui::Button((const char*)(index != -1 ? item->Artist : u8"###EMPTY1"), MathUtil::ScaleVec2(ImVec2(340, 0)));

                ImGui::Text("Notecharter\r");
                ImGui::Button((const char*)(index != -1 ? item->Noter : u8"###EMPTY1"), MathUtil::ScaleVec2(ImVec2(340, 0)));

                ImGui::Text("Note count\r");

                std::string count = std::to_string(index != -1 ? item->MaxNotes[2] : 0);
                ImGui::Button(count.c_str(), MathUtil::ScaleVec2(ImVec2(340, 0)));

                ImGui::PopItemFlag();
                ImGui::PopStyleVar();

                ImGui::EndChild();
            }

            if (ImGui::BeginChild("#test2", MathUtil::ScaleVec2(ImVec2(200, 290)), true)) {
                std::string currentDifficulty = EnvironmentSetup::Get("Difficulty");
                std::vector<std::string> difficulty = { "EX", "NX", "HX" };

                ImGui::Text("Note difficulty");
                for (int i = 0; i < difficulty.size(); i++) {
                    int index = std::atoi(currentDifficulty.c_str());

                    if (index == i) {
                        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
                    }

                    if (ImGui::Button(difficulty[i].c_str(), MathUtil::ScaleVec2(ImVec2(30, 30)))) {
                        EnvironmentSetup::Set("Difficulty", std::to_string(i));
                    }

                    if (index == i) {
                        ImGui::PopItemFlag();
                        ImGui::PopStyleVar();
                    }

                    if (i != difficulty.size() - 1) {
                        ImGui::SameLine();
                    }
                }

                ImGui::Spacing();

                ImGui::Text("Notespeed");
                // set slider width
                ImGui::PushItemWidth(ImGui::GetCurrentWindow()->Size.x - 15);
                ImGui::SliderFloat("##Notespeed", &currentSpeed, 0.1f, 4.0f, "%.2f");

                ImGui::Text("Rate");
                ImGui::SliderFloat("##Rate", &currentRate, 0.5f, 2.0f, "%.2f");
                ImGui::PopItemWidth();

                ImGui::Text("Mods");
                std::vector<std::string> Mods = { "Mirror", "Random", "Rearrange", "Autoplay" };

                for (int i = 0; i < Mods.size(); i++) {
                    auto mod = Mods[i];
					
                    int value = EnvironmentSetup::GetInt(mod);
                    if (value == 1) {
                        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.9f);
                        ImVec4 color = ImGui::GetStyleColorVec4(ImGuiCol_Button);

                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(color.x * 1.2, color.y * 1.2, color.z * 1.2, 1));
                    }

                    if (ImGui::Button(mod.c_str(), MathUtil::ScaleVec2(ImVec2(80, 0)))) {
                        EnvironmentSetup::SetInt(mod, value == 1 ? 0 : 1);

                        switch (i) {
                            case 0: {
                                EnvironmentSetup::SetInt(Mods[1], 0);
                                EnvironmentSetup::SetInt(Mods[2], 0);
                                break;
                            }

                            case 1: {
                                EnvironmentSetup::SetInt(Mods[0], 0);
                                EnvironmentSetup::SetInt(Mods[2], 0);
                                break;
                            }

                            case 2: {
                                bOpenRearrange = EnvironmentSetup::GetInt(Mods[2]) == 1;

                                EnvironmentSetup::SetInt(Mods[0], 0);
                                EnvironmentSetup::SetInt(Mods[1], 0);
                                break;
                            }
                        }
                    }

                    if (value == 1) {
                        ImGui::PopStyleVar();
                        ImGui::PopStyleColor();
                    }

                    // NewLine after 2 button on SameLine
					if ((i + 1) % 2 == 1) {
						ImGui::SameLine();
					}
                }
				

                ImGui::NewLine();

                ImGui::PushItemWidth(ImGui::GetCurrentWindow()->Size.x - 15);
                ImGui::Text("Arena");
                std::vector<std::string> Arena = { "Random", "Arena 1", "Arena 2", "Arena 3", "Arena 4", "Arena 5", "Arena 6", "Arena 7", "Arena 8", "Arena 9", "Arena 10", "Arena 11", "Arena 12" };

                // select
                int value = EnvironmentSetup::GetInt("Arena");
                if (ImGui::BeginCombo("###ComboBox1Arena", Arena[value].c_str(), 0)) {
                    for (int i = 0; i < Arena.size(); i++) {
                        bool is_selected = i == value;
                        if (ImGui::Selectable(Arena[i].c_str(), is_selected)) {
                            EnvironmentSetup::SetInt("Arena", i);
                        }
                    }

                    ImGui::EndCombo();
                }
                ImGui::PopItemWidth();

                ImGui::EndChild();
            }

            ImGui::EndChild();
        }

		// get current cursor pos
		ImVec2 cursorPos = ImGui::GetCursorPos();

        ImGui::SameLine();
        auto size = ImVec2(250, 575);
        auto xPos = MathUtil::ScaleVec2(ImVec2(windowNextSz.x - size.x - 5, 0));

        ImGui::SetCursorPosX(xPos.x);

        const int MAX_ROWS = 18;

        if (ImGui::BeginChild("#SongSelectChild", MathUtil::ScaleVec2(size), true)) {
            if (ImGui::BeginChild("#SongSelectChild2", MathUtil::ScaleVec2(ImVec2(400, 500)))) {
                ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0, 0));
                for (int i = 0; i < MAX_ROWS; i++) {
                    std::string Id = "###Button" + std::to_string(i);

                    if (i + page < music->GetMusicCount()) {
                        DB_MusicItem& item = music->GetMusicItem(i + page);
                        bool isSelected = item.Id == index;

                        if (isSelected) {
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
                        }

                        std::u8string title = std::u8string(item.Title) + std::u8string(Id.begin(), Id.end());
                        auto cursorPos = ImGui::GetCursorPos();
						
                        if (ImGui::ButtonEx(
                            (const char*)title.c_str(), 
                            MathUtil::ScaleVec2(500, 25), ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight)) {
							
                            if (item.Id != index) {
                                index = item.Id;

                                bSelectNewSong = true;
                            }
                        }

                        /*
                            FIXME: Goddamit!, this is hack because for some reason
							ImGui::IsMouseClicked won't work if ImGui::ButtonEx handled it.
                        */
                        title += u8"_Context";
                        if (ImGui::BeginPopupContextWindow((const char*)title.c_str())) {
                            ImGui::CloseCurrentPopup();
                            ImGui::EndPopup();

                            bOpenSongContext = true;
                        }

                        if (isSelected) {
                            ImGui::PopStyleColor();
                        }
                    }
                }
                ImGui::PopStyleVar();
                ImGui::EndChild();
            }

			// set cursor pos Y at bottom of window
			ImGui::SetCursorPosY(cursorPos.y - 20);

            if (ImGui::Button("Previous Page", MathUtil::ScaleVec2(ImVec2(100, 50)))) {
                page = std::clamp(page - MAX_ROWS, 0, 999999);
            }

            ImGui::SameLine();

            if (ImGui::Button("Next Page", MathUtil::ScaleVec2(ImVec2(100, 50)))) {
                if (page + MAX_ROWS < music->GetMusicCount()) {
                    page = std::clamp(page + MAX_ROWS, 0, 999999);
                }
            }

            ImGui::EndChild();
        }

        ImGui::SetCursorPos(cursorPos);
        //float oldScale = FontResources::GetButtonFont()->Scale;

        //FontResources::GetButtonFont()->Scale = 1.5f;
        ImGui::PushFont(FontResources::GetButtonFont());

        if (ImGui::Button("Play", MathUtil::ScaleVec2(ImVec2(200, 60)))) {
            bPlay = true;
        }
		
       // FontResources::GetButtonFont()->Scale = oldScale;
        ImGui::PopFont();

        // fuck it, whatever rebuild the font, this thing is required!
        if (ImGui::GetCurrentWindow()->Flags & ImGuiWindowFlags_ChildWindow) {
            ImGui::EndChild();
        }

        ImGui::End();
    }

    if (bExitPopup) {
        SDL_Event ev = {};
        ev.type = SDL_QUIT;

        SDL_PushEvent(&ev);
    }

    if (bOptionPopup) {
        ImGui::OpenPopup("Options###OptionsId");
    }

    if (bPlay && index == -1) {
        MsgBox::Show("MustSelectSong", "Error", "You must select a song to play!", MsgBoxType::OK);
    }

    if (bOpenSongContext) {
        ImGui::OpenPopup("###pop_up_right_click_song_context");
    }

    if (bOpenRearrange) {
        memset(lanePos, NULL, sizeof(lanePos));
        ImGui::OpenPopup("###open_rearrange");
    }

    ImGuiIO& io = ImGui::GetIO();

    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("Set lane position###open_rearrange", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize)) {
        int state = EnvironmentSetup::GetInt("rearrange_dialog_state");
        switch (state) {
            case 1: {
				ImGui::Text("%s", EnvironmentSetup::Get("rearrange_dialog_msg").c_str());
                
                if (ImGui::Button("OK", MathUtil::ScaleVec2(50, 0))) {
					EnvironmentSetup::SetInt("rearrange_dialog_state", 0);
                }
                break;
            }

            default: {
                int* data = reinterpret_cast<int*>(EnvironmentSetup::GetObj("LaneData"));
                if (data == NULL) {
                    data = new int[7];

                    EnvironmentSetup::SetObj("LaneData", data);
                }

                ImGui::InputText("###LanePos", lanePos, sizeof(lanePos), ImGuiInputTextFlags_CharsDecimal);
                
                ImGui::NewLine();
                if (ImGui::Button("OK", MathUtil::ScaleVec2(50, 0))) {
                    bool error = false;

                    int laneIndex[7] = {};
                    memset(laneIndex, -1, sizeof(laneIndex));
                    
                    for (int i = 0; i < 7; i++) {
                        char c = lanePos[i];

                        try {
                            // check if c is a number
                            if (!std::isdigit(c)) {
                                throw std::invalid_argument("Invalid input found!, Please input correct number between 0 to 6\n\nExample: 0123456 or 6543210 or 6540123");
                            }

                            // parse char to number
                            int num = std::stoi(std::string(1, c));

                            // check range between 0 to 6
                            if (num < 0 || num > 6) {
                                throw std::invalid_argument("Invalid input found!, Please input correct number between 0 to 6\n\nExample: 0123456 or 6543210 or 6540123");
                            }

							// find duplicates
                            for (int j = 0; j < 7; j++) {
                                if (j == i) {
                                    continue;
                                }

								if (num == laneIndex[j]) {
                                    throw std::invalid_argument("Duplicated lane position found!, Please input different lane from 0 to 6\n\nExample: 0123456 or 6543210 or 6540123");
								}
                            }

                            laneIndex[i] = num;
                        }
                        catch (std::invalid_argument e) {
                            error = true;

                            EnvironmentSetup::Set("rearrange_dialog_msg", e.what());
                            EnvironmentSetup::SetInt("rearrange_dialog_state", 1);
                        }
                    }

                    if (!error) {
                        memcpy(data, laneIndex, sizeof(laneIndex));

                        ImGui::CloseCurrentPopup();
                    }
                }

                ImGui::SameLine();

                if (ImGui::Button("Cancel", MathUtil::ScaleVec2(50, 0))) {
                    EnvironmentSetup::SetInt("Rearrange", 0);
                    ImGui::CloseCurrentPopup();
                }

                break;
            }
        }

        ImGui::EndPopup();
    }

    bool changeResolution = false;
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    
    if (ImGui::BeginPopupModal("###pop_up_right_click_song_context", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize)) {
        std::string text = "What do you want to do?";
        ImVec2 size = ImGui::CalcTextSize(text.c_str());

        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - size.x) * 0.5f);
        ImGui::Text(text.c_str());

        ImGui::NewLine();

        if (ImGui::Button("Play", MathUtil::ScaleVec2(295, 25))) {
            bPlay = true;
            ImGui::CloseCurrentPopup();
        }

        ImGui::NewLine();
        if (ImGui::Button("Open in Editor", MathUtil::ScaleVec2(295, 25))) {
            bOpenEditor = true;
            ImGui::CloseCurrentPopup();
        }

        ImGui::NewLine();
        if (ImGui::Button("Exit this menu", MathUtil::ScaleVec2(295, 25)) || ImGui::IsKeyDown(ImGuiKey_Escape)) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
    
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("Options###OptionsId", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize)) {
        auto bWaiting = EnvironmentSetup::Get("bWaiting");
        int iWaiting = bWaiting.size() ? std::atoi(bWaiting.c_str()) : -1;

        switch (iWaiting) {
            case 1: {
                auto key = EnvironmentSetup::Get("iKey");
                ImGui::Text("Waiting for keybind input...");
                ImGui::Text(("Press any key to set the Key: " + key).c_str());

                std::map<ImGuiKey, bool> blacklistedKey = {
                    // Space, Enter
                    { ImGuiKey_Space, true },
                    { ImGuiKey_Enter, true },
                    { ImGuiKey_KeyPadEnter, true },
                };

                std::string keyCount = EnvironmentSetup::Get("ikeyCount");
                for (int i = 0; i < IM_ARRAYSIZE(io.KeysDown); i++) {
                    if (io.KeysDown[i] && !blacklistedKey[(ImGuiKey)i]) {
                        EnvironmentSetup::Set("bWaiting", "0");

                        auto ikey = SDL_GetKeyFromScancode((SDL_Scancode)i);
                        std::string name = SDL_GetKeyName(ikey);

                        Configuration::Set("KeyMapping", keyCount + "Lane" + key, name);
                        break;
                    }
                }
                break;
            }

            case 2: {
                ImGui::Text("You sure you want reset the settings?");
                ImGui::Text("This will reset all settings to default.");

                bool done = false;

                if (ImGui::Button("Yes###ButtonPrompt1", MathUtil::ScaleVec2(ImVec2(40, 0)))) {
                    done = true;
                }

                ImGui::SameLine();

                if (ImGui::Button("No###ButtonPrompt2", MathUtil::ScaleVec2(ImVec2(40, 0)))) {
                    done = true;
                }

                if (done) {
                    EnvironmentSetup::Set("bWaiting", "0");
                }
                break;
            }

            default: {
                auto childSettingVec2 = MathUtil::ScaleVec2(ImVec2(400, 250));
	
                if (ImGui::BeginChild("###ChildSettingWnd", MathUtil::ScaleVec2(ImVec2(400, 250)))) {
                    if (ImGui::BeginTabBar("OptionTabBar")) {
                        if (ImGui::BeginTabItem("Inputs")) {
                            ImGui::Text("7 Keys Configuration");
                            for (int i = 0; i < 7; i++) {
                                if (i != 0) {
                                    ImGui::SameLine();
                                }

                                std::string currentKey = Configuration::Load("KeyMapping", "Lane" + std::to_string(i + 1));
                                if (ImGui::Button((currentKey + "###7KEY" + std::to_string(i)).c_str(), MathUtil::ScaleVec2(ImVec2(50, 0)))) {
                                    EnvironmentSetup::Set("bWaiting", "1");
                                    EnvironmentSetup::Set("ikeyCount", "");
                                    EnvironmentSetup::Set("iKey", std::to_string(i + 1));
                                }
                            }

                            ImGui::NewLine();

                            ImGui::Text("6 Keys Configuration");
                            for (int i = 0; i < 6; i++) {
                                if (i != 0) {
                                    ImGui::SameLine();
                                }

                                std::string currentKey = Configuration::Load("KeyMapping", "6_Lane" + std::to_string(i + 1));
                                if (ImGui::Button((currentKey + "###6KEY" + std::to_string(i)).c_str(), MathUtil::ScaleVec2(ImVec2(50, 0)))) {
                                    EnvironmentSetup::Set("bWaiting", "1");
                                    EnvironmentSetup::Set("ikeyCount", "6_");
                                    EnvironmentSetup::Set("iKey", std::to_string(i + 1));
                                }
                            }

                            ImGui::NewLine();

                            ImGui::Text("5 Keys Configuration");
                            for (int i = 0; i < 5; i++) {
                                if (i != 0) {
                                    ImGui::SameLine();
                                }

                                std::string currentKey = Configuration::Load("KeyMapping", "5_Lane" + std::to_string(i + 1));
                                if (ImGui::Button((currentKey + "###5KEY" + std::to_string(i)).c_str(), MathUtil::ScaleVec2(ImVec2(50, 0)))) {
                                    EnvironmentSetup::Set("bWaiting", "1");
                                    EnvironmentSetup::Set("ikeyCount", "5_");
                                    EnvironmentSetup::Set("iKey", std::to_string(i + 1));
                                }
                            }

                            ImGui::NewLine();

                            ImGui::Text("4 Keys Configuration");
                            for (int i = 0; i < 4; i++) {
                                if (i != 0) {
                                    ImGui::SameLine();
                                }

                                std::string currentKey = Configuration::Load("KeyMapping", "4_Lane" + std::to_string(i + 1));
                                if (ImGui::Button((currentKey + "###4KEY" + std::to_string(i)).c_str(), MathUtil::ScaleVec2(ImVec2(50, 0)))) {
                                    EnvironmentSetup::Set("bWaiting", "1");
                                    EnvironmentSetup::Set("ikeyCount", "4_");
                                    EnvironmentSetup::Set("iKey", std::to_string(i + 1));
                                }
                            }

                            ImGui::EndTabItem();
                        }

                        if (ImGui::BeginTabItem("Graphics")) {
                            int GraphicsIndex = 0;
                            int nextGraphicsIndex = -1;
                            try {
                                GraphicsIndex = std::atoi(Configuration::Load("Game", "Renderer").c_str());
                            }
                            catch (std::invalid_argument) {

                            }

                            std::vector<std::string> list = { "OpenGL", "Vulkan (DXVK)", "Direct3D-9", "Direct3D-11", "Direct3D-12" };
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
                                Configuration::Set("Game", "Renderer", std::to_string(nextGraphicsIndex));
                            }

                            int currentResolutionIndexRn = currentResolutionIndex;

                            ImGui::Text("Resolution");
							if (ImGui::BeginCombo("###ComboBox3", m_resolutions[currentResolutionIndex].c_str())) {
								for (int i = 0; i < m_resolutions.size(); i++) {
									bool isSelected = (currentResolutionIndex == i);

									if (ImGui::Selectable(m_resolutions[i].c_str(), &isSelected)) {
                                        currentResolutionIndex = i;
									}

									if (isSelected) {
										ImGui::SetItemDefaultFocus();
									}
								}

								ImGui::EndCombo();
							}

                            if (currentResolutionIndex != currentResolutionIndexRn) {
                                changeResolution = true;
                                Configuration::Set("Game", "Resolution", m_resolutions[currentResolutionIndex]);
                            }
						
                            ImGui::NewLine();

                            ImGui::Text("FPS");
                            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Warning: setting unlimited FPS can cause PC to unstable!");
                            if (ImGui::BeginCombo("###ComboBox2", m_fps[currentFPSIndex].c_str())) {
                                for (int i = 0; i < m_fps.size(); i++) {
                                    bool isSelected = (currentFPSIndex == i);

                                    if (ImGui::Selectable(m_fps[i].c_str(), &isSelected)) {
                                        currentFPSIndex = i;
                                    }

                                    if (isSelected) {
                                        ImGui::SetItemDefaultFocus();
                                    }
                                }

                                ImGui::EndCombo();
                            }

                            ImGui::EndTabItem();
                        }

                        if (ImGui::BeginTabItem("Audio")) {
                            ImGui::Text("Audio Volume");
                            ImGui::SliderInt("###Slider2", &currentVolume, 0, 100);

                            ImGui::NewLine();

                            ImGui::Text("Audio Offset");
                            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Warning: this will make keysounded sample as auto sample!");
                            ImGui::SliderInt("###Slider1", &currentOffset, -500, 500);

                            ImGui::NewLine();
                            ImGui::Checkbox("Convert Sample to Auto Sample###Checkbox1", &convertAutoSound);
                            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Convert all keysounded sample to auto sample");

                            ImGui::EndTabItem();
                        }

                        if (ImGui::BeginTabItem("Game")) {
                            ImGui::Text("Guide Line Length");
							
							std::vector<std::string> list = { "None", "Short", "Normal", "Long" };

                            for (int i = list.size() - 1; i >= 0; i--) {
                                bool is_combo_selected = currentGuideLineIndex == i;
								
                                ImGui::PushItemWidth(50);
                                if (ImGui::Checkbox(("###ComboCheck" + std::to_string(i)).c_str(), &is_combo_selected)) {
                                    currentGuideLineIndex = i;
                                }

                                ImGui::SameLine();
                                ImGui::Text(list[i].c_str()); ImGui::SameLine();
                                ImGui::Dummy(MathUtil::ScaleVec2(ImVec2(25, 0))); ImGui::SameLine();
                                
								if (i < list.size()) {
                                    ImGui::SameLine();
								}
                            }

                            ImGui::NewLine();
                            ImGui::NewLine();

                            ImGui::Text("Gameplay-Related Configuration");
							ImGui::Checkbox("Long Note Lighting###SetCheckbox1", &LongNoteLighting);
                            if (ImGui::IsItemHovered()) {
                                ImGui::SetTooltip("When Long note on hold, change the lighting brightness to 100%% else 90%% brightness");
                            }

                            ImGui::Checkbox("Long Note Head Position at HitPos###SetCheckbox2", &LongNoteOnHitPos);
                            if (ImGui::IsItemHovered()) {
								ImGui::SetTooltip("When Long note on hold, make the head position at hit position else keep going to bottom");
                            }

                            ImGui::EndTabItem();
                        }

                        ImGui::EndTabBar();
                    }

                    ImGui::EndChild();
                }

                ImGui::NewLine();

                if (ImGui::Button("Reset###Setting1", MathUtil::ScaleVec2(50, 0))) {
                    EnvironmentSetup::Set("bWaiting", "2");
                }

                ImGui::SameLine();

                if (ImGui::Button("Close###Setting2", MathUtil::ScaleVec2(50, 0))) {
                    ImGui::CloseCurrentPopup();
                }
                break;
            }
        }

        ImGui::EndPopup();
    }

    ImGui::EndDisabled();

    if (!is_update_bgm && index != -1 && isWait) {
        waitTime += delta;

        if (waitTime > 0.25) {
            isWait = false;
            bSelectNewSong = true;
        }
    }

    if (index != -1 && bSelectNewSong) {
        SaveConfiguration();
        LoadChartImage();

        if (m_bgm->IsPlaying()) {
            m_bgm->Stop();
        }

        m_bgm->Load(index);
    }

    if (bPlay && index != -1 && !is_departing) {
        is_departing = true;
        SaveConfiguration();

        EnvironmentSetup::Set("Key", std::to_string(index));
        if (m_songBackground) {
            EnvironmentSetup::SetObj("SongBackground", m_songBackground.get());
        }

        nextAlpha = 0;
        SceneManager::ExecuteAfter(600, [this]() {
            SceneManager::ChangeScene(GameScene::LOADING);
        });
    }

    if (changeResolution) {
        std::vector<std::string> resolution = splitString(m_resolutions[currentResolutionIndex], 'x');
        Window::GetInstance()->ResizeWindow(std::atoi(resolution[0].c_str()), std::atoi(resolution[1].c_str()));
    }

    if (bOpenEditor) {
        is_departing = true;
        SaveConfiguration();
		
        EnvironmentSetup::Set("Key", std::to_string(index));
        if (m_songBackground) {
            EnvironmentSetup::SetObj("SongBackground", m_songBackground.get());
        }

        SceneManager::DisplayFade(100, [this]() {
            SceneManager::ChangeScene(GameScene::EDITOR);
        });
    }

    if (is_quit) {
        SaveConfiguration();
        SceneManager::GetInstance()->StopGame();
    }
}

void SongSelectScene::Update(double delta) {
    if (static_cast<int>(currentAlpha) != static_cast<int>(nextAlpha)) {
        double increment = (delta * 2) * 100;

        // compare it using epsilon
        if (std::abs(currentAlpha - nextAlpha) < FLT_EPSILON) {
            currentAlpha = nextAlpha;
        }
        else {
            if (currentAlpha < nextAlpha) {
                currentAlpha = std::clamp(currentAlpha + increment, 0.0, 100.0);
            }
            else {
                currentAlpha = std::clamp(currentAlpha - increment, 0.0, 100.0);
            }
        }
    }

    if (is_update_bgm) {
        m_bgm->Update(delta);
    }
}

void SongSelectScene::Input(double delta) {
	
}

void SongSelectScene::OnMouseDown(const MouseState& state) {

}

bool SongSelectScene::Attach() {
    SceneManager::DisplayFade(0, [] {});

    currentAlpha = 100;
    nextAlpha = 100;

    if (EnvironmentSetup::Get("Difficulty").size() == 0) {
        EnvironmentSetup::Set("Difficulty", "0");
    }

    auto SkinName = Configuration::Load("Game", "Skin");
    auto path = Configuration::Skin_GetPath(SkinName);

    Audio* bgm = AudioManager::GetInstance()->Get("BGM");
    if (!bgm) {
        auto BGMPath = path / "Audio";
		BGMPath /= "BGM.ogg";

        if (std::filesystem::exists(BGMPath)) {
            AudioManager::GetInstance()->Create("BGM", BGMPath, &bgm);
        }
    }

    auto bgPath = path / "Menu" / "MenuBackground.png";
    if (std::filesystem::exists(bgPath) && !m_background) {
        Window* wnd = Window::GetInstance();

        m_background = std::make_unique<Texture2D>(bgPath);
        m_background->Size = UDim2::fromOffset(wnd->GetBufferWidth(), wnd->GetBufferHeight());
    }

    if (bgm) {
        bgm->SetVolume(50);
        bgm->Play(0, true);
    }

    if (m_resolutions.size() == 0) {
        int displayCount = SDL_GetNumDisplayModes(0);
        for (int i = 0; i < displayCount; i++) {
            SDL_DisplayMode mode;
			SDL_GetDisplayMode(0, i, &mode);

			m_resolutions.push_back(std::to_string(mode.w) + "x" + std::to_string(mode.h));	
        }

        // remove duplicate field
		m_resolutions.erase(std::unique(m_resolutions.begin(), m_resolutions.end()), m_resolutions.end());

        Window* window = Window::GetInstance();
        std::string currentResolution = std::to_string(window->GetWidth()) + "x" + std::to_string(window->GetHeight());
		
        // find index
		currentResolutionIndex = std::find(m_resolutions.begin(), m_resolutions.end(), currentResolution) - m_resolutions.begin();
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

    try {
        currentOffset = std::atoi(Configuration::Load("Game", "AudioOffset").c_str());
    }
    catch (std::invalid_argument) {
        currentOffset = 0;
    }

    try {
        currentVolume = std::atoi(Configuration::Load("Game", "AudioVolume").c_str());
    }
    catch (std::invalid_argument) {
        currentVolume = 0;
    }

    try {
		convertAutoSound = std::atoi(Configuration::Load("Game", "AutoSound").c_str()) != 0;
    }
	catch (std::invalid_argument) {
		convertAutoSound = true;
	}

    m_fps = { "30", "60", "75", "120", "144", "165", "180", "240", "360", "480", "600", "800", "1000", "Unlimited" };

    try {
        int value = std::atoi(Configuration::Load("Game", "FrameLimit").c_str());
		
        auto it = std::find(m_fps.begin(), m_fps.end(), std::to_string(value));
        if (it == m_fps.end()) {
            currentFPSIndex = 4;
        }
        else {
            currentFPSIndex = it - m_fps.begin();
        }
    }
    catch (std::invalid_argument) {
        currentFPSIndex = 4;
    }

    try {
        currentGuideLineIndex = std::atoi(Configuration::Load("Game", "GuideLine").c_str());
    }
    catch (std::invalid_argument) {
        currentGuideLineIndex = 2;
    }

    is_update_bgm = false;
    isWait = index != -1;
    waitTime = 0;

    if (!m_bgm) {
        m_bgm = std::make_unique<BGMPreview>();
        m_bgm->OnReady([&](bool start) {
            Audio* bgm = AudioManager::GetInstance()->Get("BGM");
            if (start) {
                if (bgm) {
                    bgm->FadeOut();
                }

                Sleep(1500);
                if (m_bgm) {
                    m_bgm->Play();
                }

                is_update_bgm = true;
            }
            else {
                if (bgm) {
                    bgm->FadeIn();
                }

                is_update_bgm = false;
            }
        });
    }

    is_departing = false;
    return true;
}

bool SongSelectScene::Detach() {
    Audio* bgm = AudioManager::GetInstance()->Get("BGM");
    if (bgm) {
        bgm->Stop();
    }

    if (m_bgm) {
        m_bgm->Stop();
    }

    isWait = false;

    return true;
}

void SongSelectScene::SaveConfiguration() {
    EnvironmentSetup::Set("SongRate", std::to_string(currentRate));
    Configuration::Set("Gameplay", "Notespeed", std::to_string(static_cast<int>(currentSpeed * 100.0)));
    Configuration::Set("Game", "AudioOffset", std::to_string(currentOffset));
	Configuration::Set("Game", "AudioVolume", std::to_string(currentVolume));
	Configuration::Set("Game", "AutoSound", std::to_string(convertAutoSound ? 1 : 0));
    Configuration::Set("Game", "FrameLimit", m_fps[currentFPSIndex]);
    Configuration::Set("Game", "GuideLine", std::to_string(currentGuideLineIndex));

    SceneManager::GetInstance()->SetFrameLimit(std::atof(m_fps[currentFPSIndex].c_str()));
}

void SongSelectScene::LoadChartImage() {
    std::lock_guard<std::mutex> lock(m_imageLock);

    if (index != -1) {
        m_songBackground.reset();

        DB_MusicItem* item = MusicDatabase::GetInstance()->Find(index);
        if (!item) {
            MsgBox::Show("DialogErr", "Error", "LoadChartImage()::item null!");
            return;
        }

        std::filesystem::path file = Configuration::Load("Music", "Folder");
        file /= "o2ma" + std::to_string(item->Id) + ".ojn";

        try {
            if (item->CoverSize < 1) {
                return;
            }

            Window* wnd = Window::GetInstance();

            auto buffer = O2::OJN::LoadOJNFile(file);
            buffer.seekg(item->CoverOffset);

			std::vector<uint8_t> coverData;
			coverData.resize(item->CoverSize);
            buffer.read((char*)coverData.data(), item->CoverSize);

            m_songBackground = std::make_unique<Texture2D>(coverData.data(), item->CoverSize);
            m_songBackground->Size = UDim2::fromOffset(wnd->GetBufferWidth(), wnd->GetBufferHeight());
        }
        catch (std::runtime_error& e) {
            MsgBox::Show("Selection_BgError", "Error", e.what());
        }
        catch (SDLException& e) {
            MsgBox::Show("Selection_BgError", "Error", e.what());
        }
    }
}
