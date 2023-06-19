#include "IntroScene.hpp"
#include <fstream>

#include "../../Engine/Keys.h"
#include "../../Engine/SceneManager.hpp"

#include "../../Engine/Configuration.hpp"
#include "../Data/Util/Util.hpp"
#include "../Data/MusicDatabase.h"
#include "../Data/OJN.h"

#include "../GameScenes.h"

#define SAFE_DELETE(x) if (x) { delete x; x = nullptr; }

IntroScene::IntroScene() {
	m_text = nullptr;
}

void IntroScene::Render(double delta) {
	m_text->Position = UDim2::fromOffset(5, 5);
	m_text->Draw("Unnamed O2 Clone (Beta 5)");

	m_text->Position = UDim2::fromOffset(5, 15);
	if (IsReady) {
		m_text->Draw("Press Any key to continue");
	}
	else {
		auto db = MusicDatabase::GetInstance();
		
		waitFrame += delta;
		if (nextIndex < m_songFiles.size()) {
			auto& path = m_songFiles[nextIndex];

			m_text->Draw("Processing file: " + path.string());

			if (waitFrame > 0.025) {
				auto fs = O2::OJN::LoadOJNFile(path);
				OJNHeader Header = {};
				fs.read((char*)&Header, sizeof(OJNHeader));

				DB_MusicItem item = {};
				item.Id = Header.songid;
				
				auto title = CodepageToUtf8((const char*)Header.title, sizeof(Header.title), 949);
				auto noter = CodepageToUtf8((const char*)Header.noter, sizeof(Header.noter), 949);
				auto artist = CodepageToUtf8((const char*)Header.artist, sizeof(Header.artist), 949);
				
				memcpy(item.Title, title.c_str(), std::clamp((int)title.size(), 0, (int)(sizeof(item.Title) - 1)));
				memcpy(item.Noter, noter.c_str(), std::clamp((int)noter.size(), 0, (int)(sizeof(item.Noter) - 1)));
				memcpy(item.Artist, artist.c_str(), std::clamp((int)artist.size(), 0, (int)(sizeof(item.Artist) - 1)));

				item.CoverOffset = Header.data_offset[3];
				item.CoverSize = Header.cover_size; 
				item.ThumbnailSize = Header.bmp_size;

				db->Insert(nextIndex++, item);

				waitFrame = 0;
			}
		}
		else {
			std::filesystem::path musicPath = Configuration::Load("Music", "Folder");

			db->Save(std::filesystem::current_path() / "Game.db");
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
	m_text = new Text(13);

	std::filesystem::path musicPath = Configuration::Load("Music", "Folder");

	auto db = MusicDatabase::GetInstance();
	std::filesystem::path dbPath = std::filesystem::current_path() / "Game.db";
	if (std::filesystem::exists(dbPath)) {
		try {
			db->Load(dbPath);

			IsReady = true;
		}
		catch (std::runtime_error) {
			goto prepare_db;
		}
	}
	else {
		prepare_db:
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
