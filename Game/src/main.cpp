#define _CRTDBG_MAP_ALLOC
#undef NDEBUG

// STD Headers
#include <iostream>
#include <filesystem>
#include <algorithm>

#if __linux__
#include <unistd.h>
#endif

// Game Headers
#include "MyGame.h"
#include "EnvironmentSetup.hpp"
#include "./Data/Util/Util.hpp"
#include "./Resources/DefaultConfiguration.h"

// Engine Headers
#include "Configuration.h"
#include "MsgBox.h"

#if _WIN32
extern "C" {
	__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

int Run(int argc, wchar_t** argv) {
	try {
		Configuration::SetDefaultConfiguration(defaultConfiguration);

		std::filesystem::path parentPath = std::filesystem::path(argv[0]).parent_path();
		if (parentPath.empty()) {
			parentPath = std::filesystem::current_path();
		}

#if _WIN32
		if (SetCurrentDirectoryW((LPWSTR)parentPath.wstring().c_str()) == FALSE) {
			std::cout << "GetLastError(): " << GetLastError() << ", with path: " << parentPath.string();

			MessageBoxA(NULL, "Failed to set directory!", "EstGame Error", MB_ICONERROR);
			return -1;
		}
#else	
		if (chdir(parentPath.string().c_str()) != 0) {
			std::cout << "Failed to set directory!" << std::endl;
			return -1;
		}
#endif

		for (int i = 1; i < argc; i++) {
			std::wstring arg = argv[i];

			// --autoplay, -a
			if (arg.find(L"--autoplay") != std::wstring::npos || arg.find(L"-a") != std::wstring::npos) {
				EnvironmentSetup::SetInt("ParameterAutoplay", 1);
			}

			// --rate, -r [float value range 0.5 - 2.0]
			if (arg.find(L"--rate") != std::wstring::npos || arg.find(L"-r") != std::wstring::npos) {
				if (i + 1 < argc) {
					float rate = std::stof(argv[i + 1]);
					rate = std::clamp(rate, 0.5f, 2.0f);

					EnvironmentSetup::Set("ParameterRate", std::to_string(rate));
				}
			}

			if (std::filesystem::exists(argv[i]) && EnvironmentSetup::GetPath("FILE").empty()) {
				std::filesystem::path path = argv[i];

				EnvironmentSetup::SetPath("FILE", path);
			}
		}
		
		MyGame game;
		if (game.Init()) {
			double frameLimit = std::atof(Configuration::Load("Game", "FrameLimit").c_str());
			game.Run(frameLimit);
		}

		return 0;
	}
	catch (std::exception& e) {
        MsgBox::ShowOut("EstGame Error", e.what(), MsgBoxType::OK, MsgBoxFlags::BTN_ERROR);
		return -1;
	}
}

#if _WIN32
int HandleStructualException(int code) {
	MessageBoxA(NULL, ("Uncaught exception: " + std::to_string(code)).c_str(), "FATAL ERROR", MB_ICONERROR);
	return EXCEPTION_EXECUTE_HANDLER;
}
#endif

int main(int argc, char* argv[]) {
#if _WIN32 && _DEBUG && MEM_LEAK_DEBUG
// msvc
#if _MSC_VER
#pragma message("Memory leak detection is enabled. This will cause performance issues.")
#else
#warning "Memory leak detection is enabled. This will cause performance issues."
#endif
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF); 
#endif
	const char* retVal = setlocale(LC_ALL, "en_US.UTF-8");
	if (retVal == nullptr) {
#if WIN32 
		MessageBoxA(NULL, "setlocale(): Failed to set locale!", "EstGame Error", MB_ICONERROR);
#else
		std::cout << "setlocale(): Failed to set locale!" << std::endl;
#endif
		return -1;
	}

    wchar_t** wargv = new wchar_t*[argc]; 
    for (int i = 0; i < argc; i++) {
		size_t len = mbstowcs(NULL, argv[i], 0) + 1;
        wargv[i] = new wchar_t[len];
        mbstowcs(wargv[i], argv[i], len);
    }

	int ret = 0;

#if 0// _WIN32 & _MSC_VER
	__try {
		ret = Run(argc, wargv);
	}

	__except (HandleStructualException(GetExceptionCode())) {
		ret = -1;
	}
#else
	ret = Run(argc, wargv);
#endif

    for (int i = 0; i < argc; i++) {
        delete[] wargv[i];
    }

	delete[] wargv;

    return ret;
}