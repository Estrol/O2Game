#pragma once
#include <string>
#include <map>

class ImFont;

namespace FontResources {
	void PreloadFontCaches();
	ImFont* Load(std::string name, int size);
};