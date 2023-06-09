#pragma once
#include <filesystem>

const char signature[4] = { 'D', 'B', '0', '1' };
 
struct DB_Header {
	char8_t Signature[4];
	int MusicCount;
};

struct DB_MusicItem {
	int Id;
	char8_t Title[64];
	char8_t Artist[32];
	char8_t Noter[32];
	char Hash[32];

	int Difficulty[3];
	int MaxNotes[3];

	int CoverOffset;
	int ThumbnailSize;
	int CoverSize;
};

class MusicDatabase {
public:
	void Load(std::filesystem::path path);

	int GetMusicCount();
	DB_MusicItem& GetMusicItem(int index);
	DB_MusicItem* GetArrayItem();
	DB_MusicItem* Find(int ojn);

	void Insert(int index, DB_MusicItem item);
	void Resize(int size);
	void Save(std::filesystem::path path);

	static MusicDatabase* GetInstance();
	static MusicDatabase* Release();
private:
	static MusicDatabase* m_instance;
	MusicDatabase();

	std::vector<DB_MusicItem> Items;
};