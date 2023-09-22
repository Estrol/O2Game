#pragma once
#include <string>
#include <map>

struct ImFont;

struct FontResolution {
    int Index;
    int Width;
    int Height;

    ImFont* Font;
    ImFont* ButtonFont;
    ImFont* SliderFont;
};

enum class TextRegion {
    Unknown,
	
	Japanese,
    Korean,
	Chinese,
};

namespace FontResources {
	void RegisterFontIndex(int idx, int width, int height);
	void ClearFontIndex();

	void PreloadFontCaches(); 
	bool ShouldRebuild(); // Only do this, if window size is updated, not buffer size
	void DoRebuild();

	void LoadFontRegion(TextRegion region);

	void SetFontIndex(int idx);
	FontResolution* GetFontIndex(int idx);

	ImFont* GetFont();
	ImFont* GetButtonFont();
	ImFont* GetReallyBigFontForSlider();
};