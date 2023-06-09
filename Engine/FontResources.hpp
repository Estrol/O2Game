#pragma once
#include <string>
#include <map>

class ImFont;

namespace FontResources {
	void PreloadFontCaches();
	void Rebuild();
	bool ShouldRebuild();
	void DoRebuild();
};