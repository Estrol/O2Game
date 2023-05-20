#include "LoadingScene.h"
#include <iostream>
#include <filesystem>
#include <fstream>

#include "../Resources/Configuration.hpp"

#include "../Data/osu.hpp"
#include "../Data/Chart.hpp"

#include "../EnvironmentSetup.hpp"
#include "../GameScenes.h"
#include "../Data/MusicDatabase.h"
#include "../../Engine/MsgBox.hpp"

LoadingScene::LoadingScene() {
	m_background = nullptr;
	m_counter = 0;
}

LoadingScene::~LoadingScene() {
	
}

void LoadingScene::Update(double delta) {
	m_counter += delta;

	auto obj = EnvironmentSetup::GetObj("SONG");
	if (obj == nullptr) {
		if (!fucked) {
			std::string songId = EnvironmentSetup::Get("Key");
			std::filesystem::path file;

			if (songId.size() > 0) {
				file = Configuration::Load("Music", "Folder");
				file /= "o2ma" + songId + ".ojn";
			}
			else {
				file = EnvironmentSetup::GetPath("FILE");
			}

			const char* bmsfile[] = { ".bms", ".bme", ".bml" };
			const char* ojnfile = ".ojn";

			Chart* chart = nullptr;
			if (file.extension() == bmsfile[0] || file.extension() == bmsfile[1] || file.extension() == bmsfile[2]) {
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
					std::string msg = "Failed to load OJN: " + ("o2ma" + songId + ".ojn");

					MsgBox::Show("FailChart", "Error", msg.c_str(), MsgBoxType::OK);
					fucked = true;
					return;
				}

				int diffIndex = 2;
				std::string diffValue = EnvironmentSetup::Get("Difficulty");
				if (diffValue.size() > 0) {
					diffIndex = std::atoi(diffValue.c_str());
				}

				chart = new Chart(o2jamFile, diffIndex);
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

			std::filesystem::path dirPath = chart->m_beatmapDirectory;
			dirPath /= chart->m_backgroundFile;

			Window* window = Window::GetInstance();
			if (chart->m_backgroundFile.size() > 0 && std::filesystem::exists(dirPath)) {
				m_background = new Texture2D(dirPath.string());
				m_background->Size = UDim2::fromOffset(window->GetBufferWidth(), window->GetBufferHeight());
			}

			if (chart->m_backgroundBuffer.size() > 0) {
				m_background = new Texture2D((uint8_t*)chart->m_backgroundBuffer.data(), chart->m_backgroundBuffer.size());
				m_background->Size = UDim2::fromOffset(window->GetBufferWidth(), window->GetBufferHeight());
			}

			EnvironmentSetup::SetObj("SONG", chart);
		}
	}

	if (m_counter > 2.5 && obj != nullptr) {
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
	if (m_background) {
		m_background->Draw();
	}
}

bool LoadingScene::Attach() {
	fucked = false;
	is_shown = false;
	m_counter = 0;

	std::string songId = EnvironmentSetup::Get("Key");
	if (songId.size()) {
		std::filesystem::path file = Configuration::Load("Music", "Folder");
		file /= "o2ma" + songId + ".ojn";

		if (std::filesystem::exists(file)) {
			DB_MusicItem* item = MusicDatabase::GetInstance()->Find(std::atoi(songId.c_str()));
			if (!item) {
				MsgBox::Show("FailChart", "Error", "Failed to find the Id: " + songId, MsgBoxType::OK);

				return true;
			}

			std::fstream fs(file, std::ios::binary | std::ios::in);
			if (!fs.is_open()) {
				std::string msg = "Failed to open file: " + file.string();
				MsgBox::Show("FailChart", "Error", msg, MsgBoxType::OK);

				return true;
			}

			fs.seekg(item->CoverOffset, std::ios::beg);
			char* buffer = new char[item->CoverSize];
			fs.read(buffer, item->CoverSize);
			fs.close();

			m_background = new Texture2D((uint8_t*)buffer, item->CoverSize);

			delete[] buffer;
		}
		else {
			MsgBox::Show("FailChart", "Error", "Failed to find note file, please re-regenerate your note database!", MsgBoxType::OK);
			return true;
		}
	}

	return true;
}

bool LoadingScene::Detach() {
	if (m_background) {
		delete m_background;
		m_background = nullptr;
	}

	return true;
}
