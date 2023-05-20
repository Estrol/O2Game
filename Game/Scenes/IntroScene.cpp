#include "IntroScene.hpp"
#include <fstream>

#include "../../Engine/Keys.h"
#include "../../Engine/SceneManager.hpp"

#include "../Resources/Configuration.hpp"
#include "../Data/MusicDatabase.h"
#include "../Data/OJN.h"

#include "../GameScenes.h"

#define SAFE_DELETE(x) if (x) { delete x; x = nullptr; }

IntroScene::IntroScene() {
	m_text = nullptr;
}

void IntroScene::Render(double delta) {
	m_text->Position = UDim2::fromOffset(5, 50);
	m_text->Draw("Unnamed O2 Clone Alpha version 0.5");

	m_text->Position = UDim2::fromOffset(5, 65);
	if (IsReady) {
		m_text->Draw("Press Any key to continue");
	}
	else {
		auto db = MusicDatabase::GetInstance();
		
		waitFrame += delta;
		if (nextIndex < m_songFiles.size()) {
			auto& path = m_songFiles[nextIndex];

			m_text->Draw("Processing file: " + path.string());

			if (waitFrame > 0.05) {
				std::fstream fs(path, std::ios::in | std::ios::binary);
				OJNHeader Header = {};
				fs.read((char*)&Header, sizeof(OJNHeader));

				DB_MusicItem item = {};
				item.Id = Header.songid;
				memcpy(&item.Title, &Header.title, sizeof(item.Title));
				memcpy(&item.Artist, &Header.artist, sizeof(item.Artist));
				memcpy(&item.Noter, &Header.noter, sizeof(item.Noter));
				memcpy(&item.Difficulty, &Header.level, sizeof(item.Difficulty));
				memcpy(&item.MaxNotes, &Header.note_count, sizeof(item.MaxNotes));
				
				item.CoverOffset = Header.data_offset[3];
				item.CoverSize = Header.cover_size; 
				item.ThumbnailSize = Header.bmp_size;

				db->Insert(nextIndex++, item);
				fs.close();

				waitFrame = 0;
			}
		}
		else {
			std::filesystem::path musicPath = Configuration::Load("Music", "Folder");
			std::string dbName = Configuration::Load("Music", "Database");

			db->Save(std::filesystem::current_path() / dbName);
			IsReady = true;
		}
	}
}

void IntroScene::OnKeyDown(const KeyState& state) {
	if (IsReady) {
		SceneManager::ChangeScene(GameScene::MAINMENU);
	}
}

bool IntroScene::Attach() {
	m_text = new Text("Arial", 13);

	std::filesystem::path musicPath = Configuration::Load("Music", "Folder");
	std::string dbName = Configuration::Load("Music", "Database");

	auto db = MusicDatabase::GetInstance();
	std::filesystem::path dbPath = std::filesystem::current_path() / dbName;
	if (std::filesystem::exists(dbPath)) {
		db->Load(dbPath);

		IsReady = true;
	}
	else {
		for (const auto& dir_entry : std::filesystem::recursive_directory_iterator(musicPath)) {
			if (dir_entry.is_regular_file()) {
				std::string fileName = dir_entry.path().filename().string();
				if (fileName.starts_with("o2ma") && fileName.ends_with(".ojn")) {
					m_songFiles.push_back(dir_entry.path());
				}
			}
		}

		std::sort(m_songFiles.begin(), m_songFiles.end(), [](auto& path1, auto& path2) {
			std::string file1 = path1.filename().string(), file2 = path2.filename().string();

			// remove .ojn from file1 and file2 then remove o2ma from both
			file1 = file1.substr(0, file1.find_first_of("."));
			file2 = file2.substr(0, file2.find_first_of("."));
			file1 = file1.substr(4, file1.size());
			file2 = file2.substr(4, file2.size());

			int id1 = std::atoi(file1.c_str());
			int id2 = std::atoi(file2.c_str());

			return id1 < id2;
		});

		db->Resize(m_songFiles.size());
	}

	return true;
}

bool IntroScene::Detach() {
	SAFE_DELETE(m_text);
	return true;
}
