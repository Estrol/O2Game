#define _CRTDBG_MAP_ALLOC
#include <windows.h>
#include <commdlg.h>
#include "MyGame.h"
#include "EnvironmentSetup.hpp"
#include <filesystem>

extern "C" {
	__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

std::string prompt() {
	OPENFILENAME ofn;
	wchar_t szFile[MAX_PATH] = { 0 };

	ZeroMemory(&ofn, sizeof(ofn));   // clear the structure
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;   // set the parent window to be the desktop
	ofn.lpstrFilter = L"Osu files (*.osu)\0*.osu\0All files (*.*)\0*.*\0";   // set the file filter
	ofn.lpstrFile = szFile;   // set the buffer for the selected filename
	ofn.nMaxFile = MAX_PATH;   // set the maximum size of the filename buffer
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;   // set the dialog flags

	if (GetOpenFileName(&ofn)) {
		std::wstring ws(szFile);
		std::string str(ws.begin(), ws.end());
		return str;
	}
	else {
		return "";
	}
}

int main(int argc, char* argv[]) {
	std::string file;

	if (argc > 1) {
		file = argv[1];
	}

	if (!file.ends_with(".osu") || !std::filesystem::exists(file)) {
		file = prompt();

		if (!file.ends_with(".osu")) {
			MessageBoxA(NULL, "Not an .osu file!", "EstGame Error", MB_ICONERROR);
			return -1;
		}
	}

	std::filesystem::path currentDir = argv[0];
	std::string rootName = currentDir.parent_path().string();

	if (SetCurrentDirectoryA((LPSTR)rootName.c_str()) == FALSE) {
		MessageBoxA(NULL, "Failed to set directory!", "EstGame Error", MB_ICONERROR);
		return -1;
	}

	EnvironmentSetup::Set("FILE", file);
	MyGame game;
	
	if (game.Init()) {
		game.Run(1000.0);
	}
}