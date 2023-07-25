#include <iostream>
#include <cstring>

#if _WIN32
#include <windows.h>
#elif __linux__
#include <dlfcn.h>
#else
#error "Unsupported platform"
#endif

typedef int (*local_main)(int argc, wchar_t** argv);

#if _WIN32
int wmain(int argc, wchar_t** argv) {
	const char* debug_path = "F:\\VisualStudio2022\\O2Game\\x64\\Debug\\Game.dll";

	HMODULE hModule = LoadLibraryA(debug_path);
	if (hModule == NULL) {
		std::cout << "Failed to load Game.dll, Err: " << GetLastError() << std::endl;
		return -1;
	}

	local_main main = (local_main)GetProcAddress(hModule, "local_main");
	if (main == NULL) {
		std::cout << "Failed to load local_main from Game.dll" << std::endl;
		return -1;
	}

	return main(argc, argv);
}
#elif __linux__
int main(int argc, char** argv) {
	wchar_t* wargv[argc];

	for (int i = 0; i < argc; i++) {
		wargv[i] = (wchar_t*)malloc(sizeof(wchar_t) * strlen(argv[i]));
		mbstowcs(wargv[i], argv[i], strlen(argv[i]));
	}

	void* handle = dlopen("Game.so", RTLD_LAZY);
	if (handle == NULL) {
		std::cout << "Failed to load Game.so" << std::endl;
		return -1;
	}

	local_main main = (local_main)dlsym(handle, "local_main");
	if (main == NULL) {
		std::cout << "Failed to load local_main from Game.so" << std::endl;
		return -1;
	}

	return main(argc, wargv);
}
#else
#error "Unsupported platform"
#endif