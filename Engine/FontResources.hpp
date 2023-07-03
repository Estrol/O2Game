#pragma once
#include <string>
#include <map>

struct ImFont;

namespace FontResources {
	void PreloadFontCaches();
	void Rebuild();
	bool ShouldRebuild();
	void DoRebuild();

	ImFont* GetButtonFont();
	ImFont* GetReallyBigFontForSlider();
};