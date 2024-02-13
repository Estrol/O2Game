#pragma once

#include "SkinStructs.h"
#include <Misc/ini.h>
#include <filesystem>
#include <sol/sol.hpp>
#include <tuple>
#include <vector>

class LuaSkin;
struct GameLua
{
    int                  GetArenaIndex();
    int                  GetHitPosition();
    int                  GetLaneOffset();
    std::tuple<int, int> GetResolution();
    int                  GetKeyCount();
    std::string          GetSkinPath();
    std::string          GetScriptPath();

    /* Utility */

    bool                IsPathExist(std::string Path);
    std::pair<int, int> GetImageSize(std::string Path);

    LuaSkin    *__skin;
    std::string __group;
};

struct LuaState
{
    std::shared_ptr<sol::state> state;
    std::shared_ptr<sol::table> table;
    std::shared_ptr<GameLua>    game;
};

class LuaSkin
{
public:
    void LoadSkin(std::string skinName);
    void LoadScript(SkinGroup group);

    std::vector<NumericValue>  GetNumeric(std::string key);
    std::vector<PositionValue> GetPosition(std::string key);
    std::vector<RectInfo>      GetRect(std::string key);
    std::vector<AudioInfo>     GetAudio();
    //NoteValue                  GetNote(std::string key);
    SpriteValue                GetSprite(std::string key);
    TweenInfo                  GetTween(std::string key);

    std::string GetSkinProp(std::string group, std::string key, std::string defaultValue = "");
    std::string GetPath();

    static LuaSkin *Get();

private:
    LuaSkin() = default;
    ~LuaSkin();

    SkinGroup                CurrentGroup;
    std::filesystem::path    CurrentPath;
    LuaState                 Script;
    Misc::mINI::INIStructure ini;
};