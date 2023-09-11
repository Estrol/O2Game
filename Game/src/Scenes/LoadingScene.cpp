#include "LoadingScene.h"

#include <fstream>
#include <iostream>
#include <filesystem>

#include "MsgBox.h"
#include "SceneManager.h"
#include "Configuration.h"
#include "Rendering/Window.h"
#include "Exception/SDLException.h"

#include "../Data/osu.hpp"
#include "../Data/Chart.hpp"

#include "../EnvironmentSetup.hpp"
#include "../GameScenes.h"
#include "../Resources/GameDatabase.h"

LoadingScene::LoadingScene() {
	m_background = nullptr;
	m_counter = 0;
}

LoadingScene::~LoadingScene() {
	
}

void LoadingScene::Update(double delta) {
	if (is_ready) m_counter += delta;
	int songId = EnvironmentSetup::GetInt("Key");
	int diffIndex = EnvironmentSetup::GetInt("Difficulty");
	bool IsO2Jam = false;

	Chart* chart = (Chart*)EnvironmentSetup::GetObj("SONG");
	if (chart == nullptr || chart->GetO2JamId() != songId) {
		if (!fucked) {
			std::filesystem::path file;

			if (songId != -1) {
				file = GameDatabase::GetInstance()->GetPath();
				file /= "o2ma" + std::to_string(songId) + ".ojn";
			}
			else {
				file = EnvironmentSetup::GetPath("FILE");

				auto autoplay = EnvironmentSetup::GetInt("ParameterAutoplay");
				auto rate = EnvironmentSetup::Get("ParameterRate");

				EnvironmentSetup::SetInt("Autoplay", autoplay);
				EnvironmentSetup::Set("SongRate", rate);
			}

			const char* bmsfile[] = { ".bms", ".bme", ".bml", ".bmsc" };
			const char* ojnfile = ".ojn";

			if (file.extension() == bmsfile[0] || file.extension() == bmsfile[1] || file.extension() == bmsfile[2] || file.extension() == bmsfile[3]) {
				BMS::BMSFile beatmap;
				beatmap.Load(file);

				if (!beatmap.IsValid()) {
					MsgBox::Show("FailChart", "Error", "Failed to BMS chart!", MsgBoxType::OK);
					fucked = true;
					return;
				}

				chart = new Chart(beatmap);
			}
			else if (file.extension() == ojnfile) {
				O2::OJN o2jamFile;
				o2jamFile.Load(file);

				if (!o2jamFile.IsValid()) {
					std::string msg = "Failed to load OJN: " + ("o2ma" + std::to_string(songId) + ".ojn");

					MsgBox::Show("FailChart", "Error", msg.c_str(), MsgBoxType::OK);
					fucked = true;
					return;
				}

				chart = new Chart(o2jamFile, diffIndex);

				IsO2Jam = true;
			}
			else {
				Osu::Beatmap beatmap(file);

				if (!beatmap.IsValid()) {
					MsgBox::Show("FailChart", "Error", "Failed to load osu beatmap!", MsgBoxType::OK);
					fucked = true;
					return;
				}

				chart = new Chart(beatmap);
			}

			EnvironmentSetup::SetObj("SONG", chart);
		}
	} else {
		IsO2Jam = chart->GetO2JamId() == songId; // TODO: refactor this
	}

	std::filesystem::path dirPath = chart->m_beatmapDirectory;
	dirPath /= chart->m_backgroundFile;

	if (IsO2Jam) {
		auto item = GameDatabase::GetInstance()->Find(songId);

		bool hashFound = strcmp(item.Hash[diffIndex], chart->MD5Hash.c_str()) == 0;
		if (!hashFound) {
			MsgBox::Show("FailChart", "Error", "Invalid map identifier, please refresh music list using F5 key!", MsgBoxType::OK);
			fucked = true;
			return;
		}
	}

	try {
		if (m_background == nullptr) {
			GameWindow* window = GameWindow::GetInstance();
			if (chart->m_backgroundFile.size() > 0 && std::filesystem::exists(dirPath)) {
				m_background = new Texture2D(dirPath.string());
				m_background->Size = UDim2::fromOffset(window->GetBufferWidth(), window->GetBufferHeight());
			}

			if (chart->m_backgroundBuffer.size() > 0 && m_background == nullptr) {
				m_background = new Texture2D((uint8_t*)chart->m_backgroundBuffer.data(), chart->m_backgroundBuffer.size());
				m_background->Size = UDim2::fromOffset(window->GetBufferWidth(), window->GetBufferHeight());
			}

			if (m_background == nullptr) {
				auto SkinName = Configuration::Load("Game", "Skin");
				auto skinPath = Configuration::Skin_GetPath(SkinName);
				auto noImage = skinPath / "Playing" / "NoImage.png";

				if (std::filesystem::exists(noImage)) {
					m_background = new Texture2D(noImage);
					m_background->Size = UDim2::fromOffset(window->GetBufferWidth(), window->GetBufferHeight());
				}
			}
		}
	}
	catch (SDLException& e) {
		MsgBox::Show("FailChart", "Error", "Failed to create texture: " + std::string(e.what()));
		fucked = true;
	}

	if (m_counter > 2.5 && chart != nullptr) {
		SceneManager::ChangeScene(GameScene::GAME);
	}
	else {
		if (fucked) {
			std::string songId = EnvironmentSetup::Get("Key");
			if (songId.size() > 0) {
				if (m_counter > 1) {
					SceneManager::ChangeScene(GameScene::MAINMENU);
				}
			}
			else {
				if (MsgBox::GetResult("FailChart") == 4) {
					SceneManager::GetInstance()->StopGame();
				}
			}
		}
	}
}

void LoadingScene::Render(double delta) {
	if (m_background && is_ready) {
		m_background->Draw();
	}
}

bool LoadingScene::Attach() {
	fucked = false;
	is_shown = false;
	is_ready = true;
	m_counter = 0;

	m_background = (Texture2D*)EnvironmentSetup::GetObj("SongBackground");
	dont_dispose = m_background != nullptr;

	EnvironmentSetup::SetObj("SongBackground", nullptr);
	return true;
}

bool LoadingScene::Detach() {
	if (m_background && !dont_dispose) {
		delete m_background;
		m_background = nullptr;
	}

	return true;
}
