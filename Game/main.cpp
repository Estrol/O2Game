#define _CRTDBG_MAP_ALLOC
#undef NDEBUG
#include <windows.h>
#include <commdlg.h>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include "MyGame.h"
#include "EnvironmentSetup.hpp"
#include "Data/bms.hpp"
#include <filesystem>
#include "../Engine/Win32ErrorHandling.h"
#include "Data/Util/Util.hpp"

extern "C" {
	__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

std::filesystem::path prompt(std::wstring rootName) {
	OPENFILENAME ofn;
	wchar_t szFile[MAX_PATH] = { 0 };

	ZeroMemory(&ofn, sizeof(ofn));   // clear the structure
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;   // set the parent window to be the desktop
	ofn.lpstrFilter = L"O2-JAM files (*.ojn)\0*.ojn\0Osu files (*.osu)\0*.osu\0BMS files (*.bms|*.bme|*.bml)\0*.bms;*.bme;*.bml\0All files (*.*)\0*.*\0";   // set the file filter
	ofn.lpstrFile = szFile;   // set the buffer for the selected filename
	ofn.nMaxFile = MAX_PATH;   // set the maximum size of the filename buffer
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;   // set the dialog flags

	if (GetOpenFileNameW(&ofn)) {
		if (SetCurrentDirectoryW((LPWSTR)rootName.c_str()) == FALSE) {
			MessageBoxA(NULL, "Failed to set directory!", "EstGame Warning", MB_ICONWARNING);
		}
		
		return szFile;
	}
	else {
		return "";
	}
}

bool API_Query() {
	try {
		curlpp::Cleanup myCleanup;
		curlpp::Easy myRequest;

		curlpp::options::Url url("https://cdn.estrol.dev/~estrol-game/authorize-access");
		myRequest.setOpt(url);
		
		std::ostringstream os;
		os << myRequest;
		std::string response = os.str();

		return response == "{\"error\": 200, \"message\": \"OK\"}";
	}
	catch (curlpp::RuntimeError) {
		return false;
	}
}

int Run(int argc, char* argv[]) {
	try {
		auto hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
		Win32Exception::ThrowIfError(hr);

		std::filesystem::path filePath;
		for (int i = 1; i < argc; i++) {
			// --autoplay -a
			// --vulkan -v
			// --resolution 

			if (strcmp(argv[i], "--autoplay") == 0 || strcmp(argv[i], "-a") == 0) {
				EnvironmentSetup::Set("Autoplay", "1");
			}
			else if (strcmp(argv[i], "--vulkan") == 0 || strcmp(argv[i], "-v") == 0) {
				EnvironmentSetup::Set("Vulkan", "1");
			}
			else if (strcmp(argv[i], "--resolution") == 0) {
				// parse next 2 argv and check
				// if they are numbers

				try {
					auto splits = splitString(argv[i + 1], 'x');
					int width = std::stoi(splits[0]);
					int height = std::stoi(splits[1]);
					EnvironmentSetup::Set("Resolution", std::to_string(width) + "x" + std::to_string(height)); // probably useless thing
				}
				catch (std::invalid_argument) {
					MessageBoxA(NULL, "Invalid resolution!", "EstGame Warning", MB_ICONWARNING);
					return -1;
				}
			}
			else if (strcmp(argv[i], "--rate") == 0) {
				try {
					float rate = std::stof(argv[i + 1]);
					EnvironmentSetup::Set("Rate", std::to_string(rate));
				}
				catch (std::invalid_argument) {
					MessageBoxA(NULL, "Invalid rate!", "EstGame Warning", MB_ICONWARNING);
					return -1;
				}
			}
			else {
				// check if there any file
				std::filesystem::path arg = argv[i];
				if (std::filesystem::exists(arg)) {
					filePath = arg;
				}
			}
		}

		if (!API_Query()) {
			MessageBoxA(NULL, "Failed to authenticate!", "EstGame Error", MB_ICONERROR);
			return -1;
		}

		if (!filePath.has_extension() || filePath.empty()) {
			filePath = prompt(std::filesystem::current_path().wstring());
		}

		if (!filePath.has_extension() || filePath.empty()) {
			MessageBoxA(NULL, "No file selected!", "EstGame Warning", MB_ICONWARNING);
			return -1;
		}
		
		EnvironmentSetup::SetPath("FILE", filePath);

		MyGame game;

		if (game.Init()) {
			game.Run(360.0);
		}

		CoUninitialize();
		return 0;
	}
	catch (std::exception& e) {
		MessageBoxA(NULL, e.what(), "EstGame Error", MB_ICONERROR);
		return -1;
	}
}

int HandleStructualException(int code) {
	MessageBoxA(NULL, ("StructureExceptionHandlingOccured: " + std::to_string(code)).c_str(), "FATAL ERROR", MB_ICONERROR);
	return EXCEPTION_EXECUTE_HANDLER;
}

int main(int argc, char* argv[]) {
	__try {
		return Run(argc, argv);
	}
	__except (HandleStructualException(GetExceptionCode())) {
		return -1;
	}
}