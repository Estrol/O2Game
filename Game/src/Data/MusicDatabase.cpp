#include "MusicDatabase.h"
#include <iostream>
#include <fstream>
#include <string.h>

MusicDatabase* MusicDatabase::m_instance = nullptr;

MusicDatabase* MusicDatabase::GetInstance() {
	if (m_instance == nullptr) {
		m_instance = new MusicDatabase();
	}

	return m_instance;
}

void MusicDatabase::Release() {
	if (m_instance) {
		delete m_instance;
	}
}

MusicDatabase::MusicDatabase() {
	Items = {};
}

void MusicDatabase::Load(std::filesystem::path path) {
	if (!std::filesystem::exists(path)) {
		throw std::runtime_error("Invalid path file!");
	}

	std::fstream fs(path, std::ios::binary | std::ios::in);
	
	DB_Header header = {};
	fs.read((char*)&header, sizeof(DB_Header));

	if (memcmp(header.Signature, signature, 2) != 0) {
		throw std::runtime_error("Invalid signature!");
	}

	if (header.Version != version) {
		throw std::runtime_error("Invalid version!");
	}

	char8_t* buffer = new char8_t[header.MusicPathSize];
	fs.read((char*)buffer, header.MusicPathSize);

	std::u8string _path(buffer, header.MusicPathSize);
	delete[] buffer;

	Path = _path;

	Items.resize(header.MusicCount);
	for (int i = 0; i < header.MusicCount; i++) {
		DB_MusicItem item = {};
		fs.read((char*)&item, sizeof(DB_MusicItem));

		Items[i] = item;
	}

	fs.close();
}

int MusicDatabase::GetMusicCount() {
	return static_cast<int>(Items.size());
}

DB_MusicItem& MusicDatabase::GetMusicItem(int index) {
	return Items[index];
}

DB_MusicItem* MusicDatabase::GetArrayItem() {
	return &Items[0];
}

DB_MusicItem* MusicDatabase::Find(int ojn) {
	for (auto& item : Items) {
		if (item.Id == ojn) {
			return &item;
		}
	}

	return nullptr;
}

void MusicDatabase::Insert(int index, DB_MusicItem item) {
	Items[index] = item;
}

void MusicDatabase::Resize(int size) {
	Items.resize(size);
}

void MusicDatabase::SetPath(std::u8string path) {
	Path = path;
}

std::u8string MusicDatabase::GetPath() {
	return Path;
}

void MusicDatabase::Save(std::filesystem::path path) {
	std::fstream fs(path, std::ios::binary | std::ios::out);

	DB_Header header = {};
	memcpy((char*)&header.Signature, signature, 2);
	header.Version = version;
	header.MusicCount = static_cast<int>(Items.size());
	header.MusicPathSize = Path.size();
	
	fs.write((char*)&header, sizeof(DB_Header));
	fs.write((char*)Path.c_str(), Path.size());

	for (int i = 0; i < header.MusicCount; i++) {
		DB_MusicItem item = Items[i];
		fs.write((char*)&item, sizeof(DB_MusicItem));
	}

	fs.close();
}
