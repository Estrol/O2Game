#include "SongSelectScene.h"
#include <iostream>
#include <array>

#include "MsgBox.h"
#include "Inputs/Keys.h"
#include "SceneManager.h"
#include "Configuration.h"
#include "Rendering/Window.h"
#include "Texture/MathUtils.h"
#include "Audio/AudioManager.h"

#include "Imgui/imgui.h"
#include "Imgui/ImguiUtil.h"
#include "Imgui/imgui_internal.h"
#include "Imgui/imgui_extends.h"

#include "Fonts/FontResources.h"
#include "Exception/SDLException.h"

#include "../GameScenes.h"
#include "../EnvironmentSetup.hpp"
#include "../Resources/SkinConfig.hpp"
#include "../Resources/GameDatabase.h"
#include "../Resources/MusicListMaker.h"

#include "../Data/Chart.hpp"
#include "../Data/Util/Util.hpp"

static std::array<std::string, 6> Mods = { "Mirror", "Random", "Rearrange", "Autoplay", "Hidden", "Flashlight" };
static std::array<std::string, 13> Arena = { "Random", "Arena 1", "Arena 2", "Arena 3", "Arena 4", "Arena 5", "Arena 6", "Arena 7", "Arena 8", "Arena 9", "Arena 10", "Arena 11", "Arena 12" };

SongSelectScene::SongSelectScene() {
    index = -1;
    memset(lanePos, 0, sizeof(lanePos));

    int* data = new int[7];
    EnvironmentSetup::SetObj("LaneData", data);
}

SongSelectScene::~SongSelectScene() {
    if (m_bgm) {
        m_bgm.reset();
    }

    m_songBackground.reset();
    m_background.reset();

    delete[] EnvironmentSetup::GetObj("LaneData");
    EnvironmentSetup::SetObj("LaneData", nullptr);
}

void SongSelectScene::Render(double delta) {
    ImguiUtil::NewFrame();

    auto window = GameWindow::GetInstance();
	
    bPlay = false;
    bExitPopup = false;
    bOptionPopup = false;
    bSelectNewSong = false;
    bOpenSongContext = false;
    bOpenEditor = false;
    bOpenRearrange = false;
    bScaleOutput = window->IsScaleOutput();

    auto windowNextSz = ImVec2((float)window->GetBufferWidth(), (float)window->GetBufferHeight());
    if (m_songBackground) {
        m_songBackground->Draw();
    }
    else {
        if (m_background) {
            m_background->Draw();
        }
    }

    ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(MathUtil::ScaleVec2(windowNextSz));

    ImGui::GetStyle().DisabledAlpha = std::clamp(currentAlpha / 100.0f, 0.00000001f, 1.0f);
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
            if (ImGui::Button("Back", MathUtil::ScaleVec2(ImVec2(50, 0)))) {
                bExitPopup = true;
            }

            if (ImGui::Button("Options", MathUtil::ScaleVec2(ImVec2(50, 0)))) {
                bOptionPopup = true;
            }

            ImGuiIO& io = ImGui::GetIO();

            ImGui::Text("Song Selection....");

            std::string text = "No Account!";
			auto textWidth = ImGui::CalcTextSize(text.c_str()).x;

			ImGui::SameLine(MathUtil::ScaleVec2(ImVec2(windowNextSz.x, 0)).x - textWidth - 15);
			ImGui::Text(text.c_str());

            ImGui::EndMenuBar();
        }

        switch (scene_index) {
            case 0: {
                OnGameSelectMusic(delta);
                break;
            }
            case 1: {
                OnGameLoadMusic(delta);
                break;
            }
        }
	
        // fuck it, whatever rebuild the font, this thing is required!
        if (ImGui::GetCurrentWindow()->Flags & ImGuiWindowFlags_ChildWindow) {
            ImGui::EndChild();
        }

        ImGui::End();
    }

    if (ImGui::IsKeyPressed(ImGuiKey_F5, false)) {
        MsgBox::Show("DialogResetList", "Notice", "Do you want refresh map list?", MsgBoxType::YESNO);
    }

    if (ImGui::IsKeyPressed(ImGuiKey_F3, false)) {
        bPlay = true;
    }

    if (bOptionPopup) {
        SceneManager::OverlayShow(GameOverlay::SETTINGS);
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
        ImGui::Text("%s", text.c_str());

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

    ImGui::EndDisabled();

    EnvironmentSetup::SetInt("Key", index);

    if (!is_update_bgm && index != -1 && isWait) {
        waitTime += (float)delta;

        if (waitTime > 0.25f) {
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

        m_bgm->Load();
    }

    if (bPlay && index != -1 && !is_departing) {
        is_departing = true;
        SaveConfiguration();

        if (m_songBackground) {
            EnvironmentSetup::SetObj("SongBackground", m_songBackground.get());
        }

        nextAlpha = 0;
        SceneManager::ExecuteAfter(600, [this]() {
            SceneManager::ChangeScene(GameScene::LOADING);
        });
    }

    if (bOpenEditor) {
        is_departing = true;
        SaveConfiguration();
		
        if (m_songBackground) {
            EnvironmentSetup::SetObj("SongBackground", m_songBackground.get());
        }

        SceneManager::DisplayFade(100, [this]() {
            SceneManager::ChangeScene(GameScene::EDITOR);
        });
    }

    if (bExitPopup) {
        SaveConfiguration();
        SceneManager::DisplayFade(100, [this]() {
            SceneManager::ChangeScene(GameScene::MAINMENU);
        });
    }
}

void SongSelectScene::OnGameLoadMusic(double delta) {
    auto db = GameDatabase::GetInstance();
	auto path = db->GetPath();
    auto& io = ImGui::GetIO();

    m_lastTime += delta;
	
    bool bExit = false;
    if (m_tempMusicIndex < m_tempMusicLists.size()) {
        if (m_lastTime >= 0.0025) {
            m_lastTime = 0;

            file = m_tempMusicLists[m_tempMusicIndex];
            MusicListMaker::Insert(file);

            m_tempMusicIndex++;
        }
    } else {
        scene_index = 0;
        m_tempMusicLists.clear();
        m_musicList = db->FindAll();
    }

    if (!file.empty()) {
        ImGui::OpenPopup("###imgui_ui_load_music");
    }

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize;
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(MathUtil::ScaleVec2(250, 80));
    if (ImGui::BeginPopupModal("Rebuild database....###imgui_ui_load_music", nullptr, flags)) {
        ImGui::NewLine();

        std::string text = "Processing file: " + file.filename().string();
        imgui_extends::TextAligment(text.c_str(), 0.5f);

        if (scene_index == 0) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::NewLine();
        ImGui::EndPopup();
    }
}

void SongSelectScene::OnGameSelectMusic(double delta) {
    auto music = GameDatabase::GetInstance();
    auto window = GameWindow::GetInstance();
    auto windowNextSz = ImVec2((float)window->GetBufferWidth(), (float)window->GetBufferHeight());
    int currentDifficulty = EnvironmentSetup::GetInt("Difficulty");

    // create child window
    if (ImGui::BeginChild("#Container1", MathUtil::ScaleVec2(ImVec2(200, 500)))) {
        if (ImGui::BeginChild("#SongSelectChild2", MathUtil::ScaleVec2(ImVec2(200, 200)), true)) {
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
            ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0, 0));

            DB_MusicItem item = {};
            if (index != -1) {
                auto it = std::find_if(m_musicList.begin(), m_musicList.end(), [&](const DB_MusicItem& item) {
                    return item.Id == index;
                });

                if (it != m_musicList.end()) {
                    item = *it;
                }
            }

            ImVec4 color = ImGui::GetStyleColorVec4(ImGuiCol_Button);

            ImGui::Text("Title\r");
            imgui_extends::TextBackground(color, MathUtil::ScaleVec2(340, 0), "%s", (const char*)item.Title);

            ImGui::Text("Artist\r");
            imgui_extends::TextBackground(color, MathUtil::ScaleVec2(340, 0), "%s", (const char*)item.Artist);

            ImGui::Text("Notecharter\r");
            imgui_extends::TextBackground(color, MathUtil::ScaleVec2(340, 0), "%s", (const char*)item.Noter);

            ImGui::Text("Note count\r");

            int difficulty = EnvironmentSetup::GetInt("Difficulty");
            int count = item.Id == -1 ? 0 : item.MaxNotes[difficulty];
            imgui_extends::TextBackground(color, MathUtil::ScaleVec2(340, 0), "%d", count);

            ImGui::Text("BPM\r");
            float bpm = item.Id == -1 ? 0.0f : item.BPM;
            imgui_extends::TextBackground(color, MathUtil::ScaleVec2(340, 0), "%.2f", bpm);

            ImGui::PopItemFlag();
            ImGui::PopStyleVar();

            ImGui::EndChild();
        }

        if (ImGui::BeginChild("#test2", MathUtil::ScaleVec2(ImVec2(200, 290)), true)) {
            std::vector<std::string> difficulty = { "EX", "NX", "HX" };

            ImGui::Text("Note difficulty");
            for (int i = 0; i < difficulty.size(); i++) {
                int index = currentDifficulty;

                if (index == i) {
                    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
                }

                if (ImGui::Button(difficulty[i].c_str(), MathUtil::ScaleVec2(ImVec2(30, 30)))) {
                    EnvironmentSetup::SetInt("Difficulty", i);
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
            ImGui::PushItemWidth(ImGui::GetCurrentWindow()->Size.x - 15);

            ImGui::Text("Notespeed");
            {
                ImGui::InputFloat("###NoteSpeed", &currentSpeed, 0.05f, 0.1f, "%.2f");
                currentSpeed = std::clamp(currentSpeed, 0.1f, 4.0f);
            }

            ImGui::Text("Rate");
            {
                ImGui::InputFloat("###Rate", &currentRate, 0.05f, 0.1f, "%.2f");
                currentRate = std::clamp(currentRate, 0.5f, 2.0f);
            }

            ImGui::PopItemWidth();

            ImGui::Text("Mods");

            for (int i = 0; i < Mods.size(); i++) {
                auto& mod = Mods[i];
                
                int value = EnvironmentSetup::GetInt(mod);
                if (value == 1) {
                    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.9f);
                    ImVec4 color = ImGui::GetStyleColorVec4(ImGuiCol_Button);

                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(color.x * 1.2f, color.y * 1.2f, color.z * 1.2f, 1.0f));
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

                        case 4: { 
                            EnvironmentSetup::SetInt(Mods[5], 0);
                            break;
                        }

                        case 5: {
                            EnvironmentSetup::SetInt(Mods[4], 0);
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
    auto xPos = MathUtil::ScaleVec2(windowNextSz.x - size.x - 5, 0);

    ImGui::SetCursorPosX(xPos.x);

    const int MAX_ROWS = 18;

    if (ImGui::BeginChild("#SongSelectChild", MathUtil::ScaleVec2(size), true)) {
        static char search[256] = {};
        static char previous[256] = {};

        if (ImGui::BeginChild("#SongSelectChild2", MathUtil::ScaleVec2(245, 500), false, ImGuiWindowFlags_AlwaysVerticalScrollbar)) {
            ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0, 0));

            if (m_musicList.empty() || strcmp(search, previous) != 0) {
                memset(previous, 0, sizeof(previous));

                auto lists = music->FindQuery(search);
                m_musicList = lists;

                strcpy(previous, search);
            }

            for (int i = 0; i < m_musicList.size(); i++) {
                DB_MusicItem& item = m_musicList[i];

                std::string Id = "###Button" + std::to_string(i);
                bool isSelected = item.Id == index;

                ImVec4 previousColor = ImGui::GetStyleColorVec4(ImGuiCol_Button);

                if (isSelected) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
                }

                char buffer[256];
                sprintf(buffer, "Lv.%03d | %s", item.Difficulty[currentDifficulty], (char*)item.Title);

                std::string content = buffer + Id;
                auto cursorPos = ImGui::GetCursorPos();

                if (ImGui::ButtonEx(
                    content.c_str(), 
                    MathUtil::ScaleVec2(245, 25), ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight)) {
                    
                    if (item.Id != index) {
                        index = item.Id;

                        bSelectNewSong = true;
                    }
                }

                /*
                    FIXME: Goddamit!, this is hack because for some reason
                    ImGui::IsMouseClicked won't work if ImGui::ButtonEx handled it.
                */
                content += "_Context";
                if (ImGui::BeginPopupContextWindow((const char*)content.c_str())) {
                    ImGui::CloseCurrentPopup();
                    ImGui::EndPopup();

                    bOpenSongContext = true;
                }

                if (isSelected) {
                    ImGui::PopStyleColor();
                }
            }
            ImGui::PopStyleVar();
            ImGui::EndChild();
        }

        // set cursor pos Y at bottom of window
        ImGui::SetCursorPosY(cursorPos.y - 20);
        ImGui::Separator();

        if (ImGui::BeginChild("###CHILD", ImVec2(0,0), false, 0)) {
            ImGui::PushItemWidth(ImGui::GetWindowSize().x);
            ImGui::Text("Search:");
            ImGui::InputTextEx("###Search", "Search by Title, Artist, Noter or Id....", search, sizeof(search), ImVec2(0, 0), ImGuiInputTextFlags_AutoSelectAll);

            // if press Enter
            if (ImGui::IsKeyPressed(ImGuiKey_Enter)) {
                // set focus to InputText
                ImGui::SetKeyboardFocusHere();
            }

            ImGui::PopItemWidth();
            ImGui::EndChild();
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
                currentAlpha = std::clamp(currentAlpha + (float)increment, 0.0f, 100.0f);
            }
            else {
                currentAlpha = std::clamp(currentAlpha - (float)increment, 0.0f, 100.0f);
            }
        }
    }

    if (is_update_bgm) {
        m_bgm->Update(delta);
    }

    if (MsgBox::GetResult("DialogResetList") == 1) {
        auto db = GameDatabase::GetInstance();
        db->Reset();

        auto path = db->GetPath();
        if (db->FindAll().size() == 0 && std::filesystem::exists(path)) {
			m_tempMusicLists = MusicListMaker::Prepare(path);

            db->Reset();
            scene_index = 1;
            m_tempMusicIndex = 0;
		}
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
    m_lastTime = 0;

    auto db = GameDatabase::GetInstance();
	bool music_reload_required = db->FindAll().size() == 0;

    if (music_reload_required) {
        scene_index = 1;

        auto path = db->GetPath();
        if (db->FindAll().size() == 0 && std::filesystem::exists(path)) {
			m_tempMusicLists = MusicListMaker::Prepare(path);
            m_tempMusicIndex = 0;
		}
    } else {
        if (index == -1) {
            bSelectNewSong = true;
            index = db->Random().Id;
        }
    }

    auto path = Configuration::Skin_GetPath();

    if (SceneManager::GetInstance()->GetCurrentSceneIndex() != GameScene::MAINMENU) {
        Audio* bgm = AudioManager::GetInstance()->Get("BGM");
        if (!bgm) {
            auto BGMPath = path / "Audio";
            BGMPath /= "BGM.ogg";

            if (std::filesystem::exists(BGMPath)) {
                AudioManager::GetInstance()->Create("BGM", BGMPath, &bgm);
            }
        }

        if (bgm && !bgm->IsPlaying()) {
            bgm->SetVolume(50);
            bgm->Play(0, true);
        }
    }

    auto bgPath = path / "Menu" / "MenuBackground.png";
    if (std::filesystem::exists(bgPath) && !m_background) {
        GameWindow* wnd = GameWindow::GetInstance();

        m_background = std::make_unique<Texture2D>(bgPath);
        m_background->Size = UDim2::fromOffset(wnd->GetBufferWidth(), wnd->GetBufferHeight());
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
        int value = std::stoi(noteValue.c_str());

		currentSpeed = std::clamp(static_cast<float>(value) / 100.0f, 0.1f, 4.0f);
	} 
    catch (std::invalid_argument) {
        currentSpeed = 2.2f;
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

                SDL_Delay(1500);
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

    EnvironmentSetup::SetInt("FontIndex", ResolutionWindowScene::GAMEPLAY);

    is_departing = false;
    return true;
}

bool SongSelectScene::Detach() {
    if (m_bgm) {
        m_bgm->Stop();
    }

    if (SceneManager::GetInstance()->GetCurrentSceneIndex() != GameScene::MAINMENU) {
        Audio* bgm = AudioManager::GetInstance()->Get("BGM");
        if (bgm) {
            bgm->Stop();
        }
    }

    isWait = false;

    return true;
}

void SongSelectScene::SaveConfiguration() {
    EnvironmentSetup::Set("SongRate", std::to_string(currentRate));
    Configuration::Set("Gameplay", "Notespeed", std::to_string(static_cast<int>(::round(currentSpeed * 100.0))));
}

void SongSelectScene::LoadChartImage() {
    std::lock_guard<std::mutex> lock(m_imageLock);

    if (index != -1) {
        m_songBackground.reset();

        DB_MusicItem item = GameDatabase::GetInstance()->Find(index);
        if (item.Id == -1) {
            MsgBox::Show("DialogErr", "Error", "LoadChartImage()::item null!");
            return;
        }

        std::filesystem::path file = GameDatabase::GetInstance()->GetPath();
        file /= "o2ma" + std::to_string(item.Id) + ".ojn";

        try {
            if (item.CoverSize < 1) {
                return;
            }

            GameWindow* wnd = GameWindow::GetInstance();

            auto buffer = O2::OJN::LoadOJNFile(file);
            buffer.seekg(item.CoverOffset);

			std::vector<uint8_t> coverData;
			coverData.resize(item.CoverSize);
            buffer.read((char*)coverData.data(), item.CoverSize);

            m_songBackground = std::make_unique<Texture2D>(coverData.data(), item.CoverSize);
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
