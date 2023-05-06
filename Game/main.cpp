#define _CRTDBG_MAP_ALLOC
#undef NDEBUG
#include <algorithm>
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
#include "Resources/Configuration.hpp"

//extern "C" {
//	__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
//	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
//}

std::filesystem::path prompt() {
	OPENFILENAME ofn;
	wchar_t szFile[MAX_PATH] = { 0 };

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFilter = L"All Supported Files (*.ojn;*.osu;*.bms;*.bme;*.bml;*)\0*.ojn;*.osu;*.bms;*.bme;*.bml\0";
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileNameW(&ofn)) {
		return szFile;
	}
	else {
		return "";	
	}
}

bool API_Query() {
	std::cout << "[Auth] Authenticating session to /~estrol-game/authorize-access" << std::endl;

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
			std::filesystem::path arg = argv[i];
			if (std::filesystem::exists(arg)) {
				filePath = arg;
				break;
			}
		}

		if (!API_Query()) {
			MessageBoxA(NULL, "Failed to authenticate!", "EstGame Error", MB_ICONERROR);
			return -1;
		}

		if (!filePath.has_extension() || filePath.empty()) {
			filePath = prompt();
		}

		if (!filePath.has_extension() || filePath.empty()) {
			MessageBoxA(NULL, "No file selected!", "EstGame Warning", MB_ICONWARNING);
			return -1;
		}
		
		EnvironmentSetup::SetPath("FILE", filePath);

		std::filesystem::path parentPath = std::filesystem::path(argv[0]).parent_path();
		if (SetCurrentDirectoryW((LPWSTR)parentPath.wstring().c_str()) == FALSE) {
			MessageBoxA(NULL, "Failed to set directory!", "EstGame Warning", MB_ICONWARNING);
		}
		
		MyGame game;

		if (game.Init()) {
			double frameLimit = std::atof(Configuration::Load("Game", "FrameLimit").c_str());
			game.Run(frameLimit);
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