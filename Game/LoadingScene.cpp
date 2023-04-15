#include "LoadingScene.h"
#include <iostream>
#include <filesystem>

#include "EnvironmentSetup.hpp"
#include "GameScenes.h"
#include "Data/osu.hpp"
#include "Data/Chart.hpp"

LoadingScene::LoadingScene() {
	m_background = nullptr;
	m_counter = 0;
}

LoadingScene::~LoadingScene() {
	
}

void LoadingScene::Update(double delta) {
	m_counter += delta;

	if (m_counter > 2.5) {
		SceneManager::ChangeScene(GameScene::GAME);
	}
}

void LoadingScene::Render(double delta) {
	if (m_background) {
		m_background->Draw();
	}
}

bool LoadingScene::Attach() {
	std::string file = EnvironmentSetup::Get("FILE");
	const char* bmsfile[] = { ".bms", ".bme", ".bml" };
	const char* ojnfile = ".ojn";

	Chart* chart = nullptr;
	if (file.find(bmsfile[0]) != std::string::npos || file.find(bmsfile[1]) != std::string::npos || file.find(bmsfile[2]) != std::string::npos) {
		BMS::BMSFile beatmap;
		beatmap.Load(file);

		if (!beatmap.IsValid()) {
			MessageBoxA(NULL, "Failed to load BMS!", "EstEngine Error", MB_ICONERROR);
			return false;
		}

		chart = new Chart(beatmap);
	}
	else if (file.find(ojnfile) != std::string::npos) {
		O2::OJN o2jamFile;
		o2jamFile.Load(file);

		if (!o2jamFile.IsValid()) {
			MessageBoxA(NULL, "Failed to load O2Jam!", "EstEngine Error", MB_ICONERROR);
			return false;
		}

		chart = new Chart(o2jamFile);
	}
	else {
		Osu::Beatmap beatmap(file);

		if (!beatmap.IsValid()) {
			MessageBoxA(NULL, "Failed to load beatmap!", "EstEngine Error", MB_ICONERROR);
			return false;
		}

		std::string title = "EstRhythm - " + beatmap.Artist + " - " + beatmap.Title;
		Window::GetInstance()->SetWindowTitle(title);

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

	std::string subTitle = chart->m_artist + " - " + chart->m_title;
	Window::GetInstance()->SetWindowSubTitle(subTitle);

	EnvironmentSetup::SetObj("SONG", chart);
	return true;
}

bool LoadingScene::Detach() {
	if (m_background) {
		delete m_background;
		m_background = nullptr;
	}

	return true;
}
