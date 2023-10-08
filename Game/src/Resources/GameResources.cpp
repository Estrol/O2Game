#include "GameResources.hpp"

#include "Configuration.h"
#include "Rendering/Renderer.h"
#include "Exception/SDLException.h"
#include "../Engine/SkinManager.hpp"

#include <fstream>
#include <filesystem>
#include <algorithm>
#include <SDL2/SDL_image.h>
#include "MsgBox.h"

#if __linux__
#include <unistd.h>
#define MAX_PATH 256
#endif

#if _MSC_VER
#define STRCPY_F strcpy_s
#else
#define STRCPY_F strcpy
#endif

#include "../Engine/SkinConfig.hpp"

#pragma warning(disable:26451)

uint8_t OPI_MAGIC_FILE[] = { 0x02, 0x00, 0x00, 0x00 };
uint8_t OPI_FILES_MAGIC[] = { 0x01, 0x00, 0x00, 0x00 };
uint8_t OJS_MAGIC_FILE[] = { 0x01, 0x00, 0x55, 0x05 };
uint8_t BND_MAGIC_FILE[] = { 0xFF, 0xFF, 0xFF, 0xFF };

OJSFrame::OJSFrame(int X, int Y, int Width, int Height, short TransColor, int FrameOffset, int FrameSize) {
	this->X = X;
	this->Y = Y;
	this->Width = Width;
	this->Height = Height;
	this->TransparencyColor = TransColor;
	this->FrameOffset = FrameOffset;
	this->FrameSize = FrameSize;
	this->Buffer = new uint8_t[FrameSize];
}

OJSFrame::~OJSFrame() {
	if (Buffer == nullptr) return;

	delete[] Buffer;
}

OPIFile* InternalGetFile(std::string fileName, std::vector<OPIFile>& files) {
	for (auto& f : files) {
		std::string name(f.Name);
		std::transform(name.begin(), name.end(), name.begin(), ::tolower);
		std::transform(fileName.begin(), fileName.end(), fileName.begin(), ::tolower);

		if (name == fileName) {
			return &f;
		}
	}

	return nullptr;
}

ESTHANDLE* InternalLoadFileData(OPIFile* file, std::ifstream* stream) {
	if (file == nullptr) {
		MsgBox::ShowOut("Error", "InternalLoadFileData::file == nullptr", MsgBoxType::OK, MsgBoxFlags::BTN_ERROR);
		return nullptr;
	}

	stream->seekg(file->FileOffset, std::ios::beg);

	if (!stream->good()) {
		MsgBox::ShowOut("Error", "InternalLoadFileData::stream::good == false", MsgBoxType::OK, MsgBoxFlags::BTN_ERROR);
		return nullptr;
	}

	if (memcmp(file->Name + strlen(file->Name) - 4, ".ojs", 4) == 0
		|| memcmp(file->Name + strlen(file->Name) - 4, ".oja", 4) == 0
		|| memcmp(file->Name + strlen(file->Name) - 4, ".oji", 4) == 0
		|| memcmp(file->Name + strlen(file->Name) - 4, ".ojt", 4) == 0) {

		uint8_t* buffer = new uint8_t[file->FileSize];
		stream->read((char*)buffer, file->FileSize);

		OJS* ojs = new OJS();
		STRCPY_F(ojs->Name, file->Name);

		ojs->RGBFormat = *(uint32_t*)(buffer);
		ojs->FrameCount = *(uint16_t*)(buffer + 4);
		ojs->TransparencyCode = *(uint16_t*)(buffer + 6);

		int headerSize = 2 + 2 + 2 + 2 + 4 + 4 + 4;
		int tmpStartOffset = 8 + (ojs->FrameCount * headerSize);
		for (int i = 0; i < ojs->FrameCount; i++) {
			int offset = 8 + (i * headerSize);

			OJSFrame frame = {
				*(short*)(buffer + offset + 0), // X
				*(short*)(buffer + offset + 2), // Y
				*(short*)(buffer + offset + 4), // Width
				*(short*)(buffer + offset + 6), // Height
				ojs->TransparencyCode,
				*(int*)(buffer + offset + 8), // FrameOffset
				*(int*)(buffer + offset + 12), // FrameSize
			};

			ojs->Frames.push_back(std::make_unique<OJSFrame>(frame));

			ojs->Frames[i].get()->Buffer = new uint8_t[ojs->Frames[i].get()->FrameSize];
			memcpy(ojs->Frames[i].get()->Buffer, buffer + tmpStartOffset + ojs->Frames[i].get()->FrameOffset, ojs->Frames[i].get()->FrameSize);
		}

		delete[] buffer;
		return (ESTHANDLE*)ojs;
	}

	// check if file->Name ends with .bnd
	if (memcmp(file->Name + strlen(file->Name) - 4, ".bnd", 4) == 0) {
		BND* bnd = new BND();

		int MAGIC_NUMBER = 0;
		stream->read((char*)&MAGIC_NUMBER, 4);
		if (MAGIC_NUMBER != -1) {
			return (ESTHANDLE*)bnd;
		}

		stream->read((char*)&bnd->Count, 2);

		for (int i = 0; i < bnd->Count; i++) {
			Boundary bound = {};

			stream->read((char*)&bound.X, 4);
			stream->read((char*)&bound.Y, 4);
			stream->read((char*)&bound.Width, 4);
			stream->read((char*)&bound.Height, 4);

			bnd->Coordinates.push_back(bound);
		}

		return (ESTHANDLE*)bnd;
	}

	return nullptr;
}

std::tuple<std::vector<OPIFile>, std::ifstream*> LoadOPI(std::filesystem::path& path) {
	std::vector<OPIFile> files;

	if (std::filesystem::exists(path)) {
		throw std::runtime_error("Missing: " + path.string());
	}

	std::fstream file(path, std::ios::in | std::ios::binary);
	if (!file.good()) {
		throw std::runtime_error("Failed to open: " + path.string());
	}

	size_t bufferSize = 0;
	file.seekg(0, std::ios::end);
	bufferSize = file.tellg();
	file.seekg(0, std::ios::beg);

	char opi_magic[4];
	file.read(opi_magic, 4);

	if (memcmp(opi_magic, "OPI", 3) != 0) {
		throw std::runtime_error("Invalid OPI file: " + path.string());
	}

	int fileCount = 0;
	file.read((char*)&fileCount, 4);

	if ((size_t)(fileCount * 152) > bufferSize) {
		throw std::runtime_error("Invalid OPI file: " + path.string());
	}

	file.seekg((bufferSize - ((size_t)fileCount * 152)), std::ios::beg);

	for (int i = 0; i < fileCount; i++) {
		char file_magic[4] = {};
		file.read(file_magic, 4);

		if (memcmp(file_magic, OPI_FILES_MAGIC, 4) != 0) {
			std::cout << "Unknown file magic: " << std::hex << file_magic << " at index: " << file.tellg() << std::endl;
		}

		OPIFile idx = {};

		char fileName[128] = {};
		file.read(fileName, 128);
		STRCPY_F(idx.Name, fileName);

		file.read((char*)&idx.FileOffset, 4);
		file.read((char*)&idx.FileSize, 4);

		int FileSize2 = 0;
		file.read((char*)&FileSize2, 4);

		idx.FileSize = (std::max)(idx.FileSize, FileSize2);

		file.seekg(8, std::ios::cur);

		files.push_back(idx);
	}

	// not really got idea but whatever
	return { files, new std::ifstream(path, std::ios::binary) };
}

namespace GameAvatarResource {
	std::vector<OPIFile> files;
	std::ifstream* file;

	bool Load() {
		char path[MAX_PATH];
#if _WIN32
		GetModuleFileNameA(NULL, path, MAX_PATH);
#else
		int len = readlink("/proc/self/exe", path, sizeof(path) - 1);
		if (len != -1) {
			path[len] = '\0';
		}
#endif

		// get path to .opi file
		std::string opiPath = std::string(path);
		std::filesystem::path _path = opiPath.substr(0, opiPath.find_last_of("\\/")) + "\\Image\\Avatar.opi";

		if (!std::filesystem::exists(_path)) {
			MsgBox::ShowOut("Error", "Missing: " + _path.string(), MsgBoxType::OK, MsgBoxFlags::BTN_ERROR);
			return false;
		}

		try {
			auto res = LoadOPI(_path);

			files = std::move(std::get<0>(res));
			file = std::get<1>(res);
		}
		catch (std::runtime_error& e) {
			MsgBox::ShowOut("Error", e.what(), MsgBoxType::OK, MsgBoxFlags::BTN_ERROR);
			return false;
		}

		return true;
	}

	bool Dispose() {
		file->close();
		files.clear();

		return true;
	}

	OPIFile* GetFile(std::string name) {
		return InternalGetFile(name, files);
	}

	ESTHANDLE* LoadFileData(OPIFile* opiFile) {
		return InternalLoadFileData(opiFile, file);
	}
}

namespace GameInterfaceResource {
	std::vector<OPIFile> files;
	std::ifstream* file;

	bool Load() {
		char path[MAX_PATH];
#if _WIN32
		GetModuleFileNameA(NULL, path, MAX_PATH);
#else
		int len = readlink("/proc/self/exe", path, sizeof(path) - 1);
		if (len != -1) {
			path[len] = '\0';
		}
#endif

		// get path to .opi file
		std::string opiPath = std::string(path);
		std::filesystem::path _path = opiPath.substr(0, opiPath.find_last_of("\\/")) + "\\Image\\Interface.opi";

		if (!std::filesystem::exists(_path)) {
			MsgBox::ShowOut("Error", "Missing: " + _path.string(), MsgBoxType::OK, MsgBoxFlags::BTN_ERROR);
			return false;
		}

		try {
			auto res = LoadOPI(_path);

			files = std::move(std::get<0>(res));
			file = std::get<1>(res);
		}
		catch (std::runtime_error& e) {
			MsgBox::ShowOut("Error", e.what(), MsgBoxType::OK, MsgBoxFlags::BTN_ERROR);
			return false;
		}

		return true;
	}

	bool Dispose() {
		file->close();
		files.clear();

		return true;
	}

	OPIFile* GetFile(std::string name) {
		return InternalGetFile(name, files);
	}

	ESTHANDLE* LoadFileData(OPIFile* opiFile) {
		return InternalLoadFileData(opiFile, file);
	}
}

namespace GamePlayingResource {
	std::vector<OPIFile> files;
	std::ifstream* file;

	bool Load() {
		char path[MAX_PATH];
#if _WIN32
		GetModuleFileNameA(NULL, path, MAX_PATH);
#else
		int len = readlink("/proc/self/exe", path, sizeof(path) - 1);
		if (len != -1) {
			path[len] = '\0';
		}
#endif

		// get path to .opi file
		std::string opiPath = std::string(path);
		std::filesystem::path _path = opiPath.substr(0, opiPath.find_last_of("\\/")) + "\\Image\\Playing.opi";

		if (!std::filesystem::exists(_path)) {
			MsgBox::ShowOut("Error", "Missing: " + _path.string(), MsgBoxType::OK, MsgBoxFlags::BTN_ERROR);
			return false;
		}

		try {
			auto res = LoadOPI(_path);

			files = std::move(std::get<0>(res));
			file = std::get<1>(res);
		}
		catch (std::runtime_error& e) {
			MsgBox::ShowOut("Error", e.what(), MsgBoxType::OK, MsgBoxFlags::BTN_ERROR);
			return false;
		}

		return true;
	}

	bool Dispose() {
		file->close();
		files.clear();

		return true;
	}

	OPIFile* GetFile(std::string name) {
		return InternalGetFile(name, files);
	}

	ESTHANDLE* LoadFileData(OPIFile* opiFile) {
		return InternalLoadFileData(opiFile, file);
	}
}

namespace GameNoteResource {
	std::unordered_map<NoteImageType, NoteImage*> noteTextures;
	bool Loaded = false;

	bool Load() {
		if (Loaded) {
			throw std::runtime_error("NoteResource already loaded");
		}

		bool IsVulkan = Renderer::GetInstance()->IsVulkan();
		
		auto skinPath = SkinManager::GetInstance()->GetPath();
		auto skinNotePath = skinPath / "Notes";

		auto manager = SkinManager::GetInstance();

		for (int i = 0; i < 7; i++) {
			NoteValue note = manager->GetNote(SkinGroup::Notes, "LaneHit" + std::to_string(i));
			NoteValue hold = manager->GetNote(SkinGroup::Notes, "LaneHold" + std::to_string(i));

			NoteImage* noteImage = new NoteImage();
			NoteImage* holdImage = new NoteImage();

			noteImage->MaxFrames = note.numOfFiles;
			holdImage->MaxFrames = hold.numOfFiles;

			noteImage->Surface.resize(note.numOfFiles);
			holdImage->Surface.resize(hold.numOfFiles);

			noteImage->Texture.resize(note.numOfFiles);
			holdImage->Texture.resize(hold.numOfFiles);

			noteImage->VulkanTexture.resize(note.numOfFiles);
			holdImage->VulkanTexture.resize(hold.numOfFiles);

			for (int j = 0; j < note.numOfFiles; j++) {
				auto path = skinNotePath / (note.fileName + std::to_string(j) + ".png");
				if (!std::filesystem::exists(path)) {
					throw std::runtime_error("File: " + path.string() + " is not found!");
				}

				if (IsVulkan) {
					auto tex_data = vkTexture::TexLoadImage(path);
					if (tex_data == nullptr) {
						throw std::runtime_error("Failed to load image: " + path.string() + "!");
					}

					noteImage->VulkanTexture[j] = tex_data;

					if (j == 0) {
						auto& rect = noteImage->TextureRect;
						rect.left = 0;
						rect.top = 0;
						
						vkTexture::QueryTexture(tex_data, rect.right, rect.bottom);
					}
				}
				else {
					noteImage->Surface[j] = IMG_Load(path.string().c_str());
					if (noteImage->Surface[j] == nullptr) {
						throw std::runtime_error("Failed to load image: " + path.string() + "!");
					}

					noteImage->Texture[j] = SDL_CreateTextureFromSurface(Renderer::GetInstance()->GetSDLRenderer(), noteImage->Surface[j]);
					if (noteImage->Texture[j] == nullptr) {
						throw std::runtime_error("Failed to create texture from image: " + path.string() + "!");
					}

					if (j == 0) {
						// query texture size
						auto& rect = noteImage->TextureRect;
						rect.left = 0;
						rect.top = 0;

						SDL_QueryTexture(noteImage->Texture[j], NULL, NULL, (int*)&rect.right, (int*)&rect.bottom);
					}
				}
			}

			for (int j = 0; j < hold.numOfFiles; j++) {
				auto path = skinNotePath / (hold.fileName + std::to_string(j) + ".png");
				if (!std::filesystem::exists(path)) {
					throw std::runtime_error("File: " + path.string() + " is not found!");
				}

				if (IsVulkan) {
					auto tex_data = vkTexture::TexLoadImage(path);
					if (tex_data == nullptr) {
						throw std::runtime_error("Failed to load image: " + path.string() + "!");
					}

					holdImage->VulkanTexture[j] = tex_data;

					if (j == 0) {
						auto& rect = holdImage->TextureRect;
						rect.left = 0;
						rect.top = 0;

						vkTexture::QueryTexture(tex_data, rect.right, rect.bottom);
					}
				}
				else {
					holdImage->Surface[j] = IMG_Load((const char*)path.u8string().c_str());
					if (holdImage->Surface[j] == nullptr) {
						throw std::runtime_error("Failed to load image: " + path.string() + "!");
					}

					holdImage->Texture[j] = SDL_CreateTextureFromSurface(Renderer::GetInstance()->GetSDLRenderer(), holdImage->Surface[j]);
					if (holdImage->Texture[j] == nullptr) {
						throw std::runtime_error("Failed to create texture from image: " + path.string() + "!");
					}

					if (j == 0) {
						// query texture size
						auto& rect = holdImage->TextureRect;
						rect.left = 0;
						rect.top = 0;

						SDL_QueryTexture(holdImage->Texture[j], NULL, NULL, (int*)&rect.right, (int*)&rect.bottom);
					}
				}
			}

			noteTextures[(NoteImageType)i] = noteImage;
			noteTextures[(NoteImageType)(i + 7)] = holdImage;
		}

		NoteValue trailUp = manager->GetNote(SkinGroup::Notes, "NoteTrailUp");
		NoteValue trailDown = manager->GetNote(SkinGroup::Notes, "NoteTrailDown");
		
		NoteImage* trailUpImg = new NoteImage();
		trailUpImg->Texture.resize(trailUp.numOfFiles);
		trailUpImg->Surface.resize(trailUp.numOfFiles);
		trailUpImg->VulkanTexture.resize(trailUp.numOfFiles);

		NoteImage* trailDownImg = new NoteImage();
		trailDownImg->Texture.resize(trailDown.numOfFiles);
		trailDownImg->Surface.resize(trailDown.numOfFiles);
		trailDownImg->VulkanTexture.resize(trailUp.numOfFiles);

		for (int i = 0; i < trailUp.numOfFiles; i++) {
			auto path = skinNotePath / (trailUp.fileName + std::to_string(i) + ".png");
			if (!std::filesystem::exists(path)) {
				throw std::runtime_error("File: " + path.string() + " is not found!");
			}

			if (IsVulkan) {
				auto tex_data = vkTexture::TexLoadImage(path);
				if (tex_data == nullptr) {
					throw std::runtime_error("Failed to load image: " + path.string() + "!");
				}

				trailUpImg->VulkanTexture[i] = tex_data;

				if (i == 0) {
					auto& rect = trailUpImg->TextureRect;
					rect.left = 0;
					rect.top = 0;

					vkTexture::QueryTexture(tex_data, rect.right, rect.bottom);
				}
			}
			else {
				trailUpImg->Surface[i] = IMG_Load((const char*)path.u8string().c_str());
				if (trailUpImg->Surface[i] == nullptr) {
					throw std::runtime_error("Failed to load image: " + path.string() + "!");
				}

				trailUpImg->Texture[i] = SDL_CreateTextureFromSurface(Renderer::GetInstance()->GetSDLRenderer(), trailUpImg->Surface[i]);
				if (trailUpImg->Texture[i] == nullptr) {
					throw std::runtime_error("Failed to create texture from image: " + path.string() + "!");
				}

				if (i == 0) {
					// query texture size
					auto& rect = trailUpImg->TextureRect;
					rect.left = 0;
					rect.top = 0;

					SDL_QueryTexture(trailUpImg->Texture[i], NULL, NULL, (int*)&rect.right, (int*)&rect.bottom);
				}
			}
		}

		for (int i = 0; i < trailDown.numOfFiles; i++) {
			auto path = skinNotePath / (trailDown.fileName + std::to_string(i) + ".png");
			if (!std::filesystem::exists(path)) {
				throw std::runtime_error("File: " + path.string() + " is not found!");
			}

			if (IsVulkan) {
				auto tex_data = vkTexture::TexLoadImage(path);
				if (tex_data == nullptr) {
					throw std::runtime_error("Failed to load image: " + path.string() + "!");
				}

				trailDownImg->VulkanTexture[i] = tex_data;

				if (i == 0) {
					auto& rect = trailDownImg->TextureRect;
					rect.left = 0;
					rect.top = 0;

					vkTexture::QueryTexture(tex_data, rect.right, rect.bottom);
				}
			}
			else {
				trailDownImg->Surface[i] = IMG_Load((const char*)path.u8string().c_str());
				if (trailDownImg->Surface[i] == nullptr) {
					throw std::runtime_error("Failed to load image: " + path.string() + "!");
				}

				trailDownImg->Texture[i] = SDL_CreateTextureFromSurface(Renderer::GetInstance()->GetSDLRenderer(), trailDownImg->Surface[i]);
				if (trailDownImg->Texture[i] == nullptr) {
					throw std::runtime_error("Failed to create texture from image: " + path.string() + "!");
				}

				if (i == 0) {
					// query texture size
					auto& rect = trailDownImg->TextureRect;
					rect.left = 0;
					rect.top = 0;

					SDL_QueryTexture(trailDownImg->Texture[i], NULL, NULL, (int*)&rect.right, (int*)&rect.bottom);
				}
			}
		}

		noteTextures[NoteImageType::TRAIL_UP] = trailUpImg;
		noteTextures[NoteImageType::TRAIL_DOWN] = trailDownImg;
		
		Loaded = true;
		return true;
	}

	bool Dispose() {
		bool IsVulkan = Renderer::GetInstance()->IsVulkan();

		for (auto& it : noteTextures) {
			if (IsVulkan) {
				for (auto& tex : it.second->VulkanTexture) {
					vkTexture::ReleaseTexture(tex);
					tex = nullptr;
				}
			}
			else {
				for (auto& tex : it.second->Texture) {
					SDL_DestroyTexture(tex);
					tex = nullptr;
				}

				for (auto& sur : it.second->Surface) {
					SDL_FreeSurface(sur);
					sur = nullptr;
				}
			}

			delete it.second;
		}

		noteTextures.clear();
		Loaded = false;
		return true;
	}

	NoteImage* GetNoteTexture(NoteImageType noteType) {
		return noteTextures[noteType];
	}
}
