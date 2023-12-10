#pragma once
#include "Overlay.h"
#include <vector>

class SettingsOverlay : public Overlay
{
public:
    SettingsOverlay();
    ~SettingsOverlay() = default;

    void Render(double delta) override;
    bool Attach() override;
    bool Detach() override;

private:
    void LoadConfiguration();
    void SaveConfiguration();
    void PreloadSkin();
    bool bSave = false;

    int  startOffset = 0;
    int  currentVolume = 100;
    int  currentFPSIndex = 0;
    int  currentOffset = 0;
    int  currentResolutionIndex = 0;
    int  currentGuideLineIndex = 0;
    bool LongNoteLighting = false;
    bool LongNoteOnHitPos = false;
    bool convertAutoSound = false;

    std::string              currentSkin = "";
    std::vector<std::string> skins = {};
};