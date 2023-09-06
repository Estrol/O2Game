#pragma once
#include <filesystem>
#include <vector>

constexpr int kMaxPath = 260;
const char signature[2] = { 'D', 'B' };
const int version = 3;

struct DB_Header {
	// DB signature
	char8_t Signature[2];
	// Current DB version
	short Version;
	// Number of music items
	int MusicCount;
	// Music path string size
	int MusicPathSize;
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
	void SetPath(std::u8string Path);
	std::u8string GetPath();

	void Save(std::filesystem::path path);

	static MusicDatabase* GetInstance();
	static void Release();
private:
	static MusicDatabase* m_instance;
	MusicDatabase();

	std::u8string Path;
	std::vector<DB_MusicItem> Items;
};