#include "GameResources.hpp"
#include "../../Engine/EstEngine.hpp"

#include <filesystem>
#include <fstream>
#include <directxtk/WICTextureLoader.h>

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
		MessageBoxA(0, "InternalLoadFileData::file == nullptr", "Error", MB_ICONERROR);
		return nullptr;
	}

	stream->seekg(file->FileOffset, std::ios::beg);

	if (!stream->good()) {
		MessageBoxA(0, "InternalLoadFileData::stream::good == false", "Error", MB_ICONERROR);
		return nullptr;
	}

	if (memcmp(file->Name + strlen(file->Name) - 4, ".ojs", 4) == 0
		|| memcmp(file->Name + strlen(file->Name) - 4, ".oja", 4) == 0
		|| memcmp(file->Name + strlen(file->Name) - 4, ".oji", 4) == 0
		|| memcmp(file->Name + strlen(file->Name) - 4, ".ojt", 4) == 0) {

		uint8_t* buffer = new uint8_t[file->FileSize];
		stream->read((char*)buffer, file->FileSize);

		OJS* ojs = new OJS();
		strcpy_s(ojs->Name, file->Name);

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

			ojs->Frames.emplace_back(std::make_unique<OJSFrame>(frame));

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

			bnd->Coordinates.emplace_back(bound);
		}

		return (ESTHANDLE*)bnd;
	}

	return nullptr;
}

namespace GameInterfaceResource {
	std::vector<OPIFile> files;
	std::ifstream* file;

	uint8_t* buffer;
	size_t bufferSize;

	bool Load() {
		// get current path
		char path[MAX_PATH];
		GetModuleFileNameA(NULL, path, MAX_PATH);

		// get path to .opi file
		std::string opiPath = std::string(path);
		opiPath = opiPath.substr(0, opiPath.find_last_of("\\/")) + "\\Image\\Interface.opi";

		if (!std::filesystem::exists(opiPath)) {
			MessageBoxA(NULL, "Missing interface.opi!", "Error", MB_ICONERROR);
			return false;
		}

		file = new std::ifstream(opiPath, std::ios::binary);
		if (!file->is_open()) {
			MessageBoxA(NULL, "Failed to open interface.opi!", "Error", MB_ICONERROR);

			Dispose();
			return false;
		}

		file->seekg(0, std::ios::end);

		bufferSize = file->tellg();
		if (bufferSize < (size_t)(4 * 2)) {
			MessageBoxA(NULL, "Invalid opi header [1]", "Error", MB_ICONERROR);
			return false;
		}

		file->seekg(0, std::ios::beg);

		buffer = new uint8_t[bufferSize];
		file->read((char*)buffer, bufferSize);

		if (buffer == nullptr) {
			MessageBoxA(NULL, "Failed to allocate memory for opi file!", "Error", MB_ICONERROR);
			return false;
		}

		file->seekg(0, std::ios::beg);

		char opi_magic[4];
		file->read(opi_magic, 4);
		if (memcmp(opi_magic, OPI_MAGIC_FILE, 4) != 0) {
			MessageBoxA(NULL, "Invalid opi header [2]", "Error", MB_ICONERROR);
			return false;
		}

		int fileCount;
		file->read((char*)&fileCount, 4);

		if ((size_t)(fileCount * 152) > bufferSize) {
			MessageBoxA(NULL, "Invalid opi header [3]", "Error", MB_ICONERROR);
			return false;
		}

		file->seekg(bufferSize - (fileCount * 152), std::ios::beg);

		for (int i = 0; i < fileCount; i++) {
			char file_magic[4];
			file->read(file_magic, 4);

			if (memcmp(file_magic, OPI_FILES_MAGIC, 4) != 0) {
				std::cout << "Unknown file magic: " << std::hex << file_magic << " at index: " << file->tellg() << std::endl;
			}

			OPIFile idx = {};

			char fileName[128];
			file->read(fileName, 128);
			strcpy_s(idx.Name, fileName);

			//std::cout << "[Debug] Load: " << idx.Name << std::endl;

			file->read((char*)&idx.FileOffset, 4);
			file->read((char*)&idx.FileSize, 4);

			int FileSize2 = 0;
			file->read((char*)&FileSize2, 4);

			idx.FileSize = (std::max)(idx.FileSize, FileSize2);

			file->seekg(8, std::ios::cur);

			files.emplace_back(idx);
		}

		return true;
	}

	bool Dispose() {
		file->close();
		files.clear();

		if (buffer != nullptr) delete[] buffer;

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

	uint8_t* buffer;
	size_t bufferSize;

	bool Load() {
		// get current path
		char path[MAX_PATH];
		GetModuleFileNameA(NULL, path, MAX_PATH);

		// get path to .opi file
		std::string opiPath = std::string(path);
		opiPath = opiPath.substr(0, opiPath.find_last_of("\\/")) + "\\Image\\Playing.opi";

		if (!std::filesystem::exists(opiPath)) {
			MessageBoxA(NULL, "Missing Playing.opi!", "Error", MB_ICONERROR);
			return false;
		}

		file = new std::ifstream(opiPath, std::ios::binary);
		if (!file->is_open()) {
			MessageBoxA(NULL, "Failed to open interface.opi!", "Error", MB_ICONERROR);

			Dispose();
			return false;
		}

		file->seekg(0, std::ios::end);

		bufferSize = file->tellg();
		if (bufferSize < (size_t)(4 * 2)) {
			MessageBoxA(NULL, "Invalid opi header [1]", "Error", MB_ICONERROR);
			return false;
		}

		file->seekg(0, std::ios::beg);

		buffer = new uint8_t[bufferSize];
		file->read((char*)buffer, bufferSize);

		if (buffer == nullptr) {
			MessageBoxA(NULL, "Failed to allocate memory for opi file!", "Error", MB_ICONERROR);
			return false;
		}

		file->seekg(0, std::ios::beg);

		char opi_magic[4];
		file->read(opi_magic, 4);
		if (memcmp(opi_magic, OPI_MAGIC_FILE, 4) != 0) {
			MessageBoxA(NULL, "Invalid opi header [2]", "Error", MB_ICONERROR);
			return false;
		}

		int fileCount;
		file->read((char*)&fileCount, 4);

		if ((size_t)(fileCount * 152) > bufferSize) {
			MessageBoxA(NULL, "Invalid opi header [3]", "Error", MB_ICONERROR);
			return false;
		}

		file->seekg(bufferSize - (fileCount * 152), std::ios::beg);

		for (int i = 0; i < fileCount; i++) {
			char file_magic[4];
			file->read(file_magic, 4);

			if (memcmp(file_magic, OPI_FILES_MAGIC, 4) != 0) {
				std::cout << "Unknown file magic: " << std::hex << file_magic << " at index: " << file->tellg() << std::endl;
			}

			OPIFile idx = {};

			char fileName[128];
			file->read(fileName, 128);
			strcpy_s(idx.Name, fileName);

			file->read((char*)&idx.FileOffset, 4);
			file->read((char*)&idx.FileSize, 4);

			int FileSize2 = 0;
			file->read((char*)&FileSize2, 4);

			idx.FileSize = (std::max)(idx.FileSize, FileSize2);

			file->seekg(8, std::ios::cur);

			files.emplace_back(idx);
		}

		return true;
	}

	bool Dispose() {
		file->close();
		files.clear();

		if (buffer != nullptr) delete[] buffer;

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

	bool Load() {
		using namespace DirectX;
		
		const char* textures[] = {
			"mania-note1.png",
			"mania-note2.png",
			"mania-note3.png",

			"mania-hold1.png",
			"mania-hold2.png",
			"mania-hold3.png",
		};
		
		for (int i = 0; i < 6; i++) {
			std::filesystem::path path = std::filesystem::current_path() / "Skins" / "Default" / "Notes" / textures[i];
			if (!std::filesystem::exists(path)) {
				std::cout << "Missing: " << path << std::endl;

				MessageBoxA(NULL, "Missing note texture!", "Error", MB_ICONERROR);
				return false;
			}
			
			ID3D11Device* device = Renderer::GetInstance()->GetDevice();
			ID3D11DeviceContext* context = Renderer::GetInstance()->GetImmediateContext();

			ID3D11ShaderResourceView* pTexture = nullptr;
			ID3D11Resource* pResource;
			HRESULT hr = CreateWICTextureFromFileEx(
				device,
				path.wstring().c_str(),
				0,
				D3D11_USAGE_DEFAULT,
				D3D11_BIND_SHADER_RESOURCE,
				0,
				0,
				WIC_LOADER_FORCE_RGBA32 | WIC_LOADER_IGNORE_SRGB,
				&pResource,
				&pTexture
			);

			if (FAILED(hr)) {
				MessageBoxA(NULL, "Failed to load note texture!", "Error", MB_ICONERROR);
				return false;
			}
			
			// query interface
			ID3D11Texture2D* texture = nullptr;
			hr = pResource->QueryInterface<ID3D11Texture2D>(&texture);

			if (FAILED(hr)) {
				MessageBoxA(NULL, "Failed to load note texture!", "Error", MB_ICONERROR);
				return false;
			}

			// get image size
			D3D11_TEXTURE2D_DESC desc;
			texture->GetDesc(&desc);

			NoteImage* noteImage = new NoteImage();
			noteImage->Texture = pTexture;
			noteImage->TextureRect = { 0, 0, (LONG)desc.Width, (LONG)desc.Height };

			noteTextures[(NoteImageType)i] = noteImage;

			texture->Release();
			pResource->Release();
		}
		
		return true;
	}

	bool Dispose() {
		for (auto& it : noteTextures) {
			it.second->Texture->Release();
			delete it.second;
		}

		return true;
	}

	NoteImage* GetNoteTexture(NoteImageType noteType) {
		return noteTextures[noteType];
	}
}
