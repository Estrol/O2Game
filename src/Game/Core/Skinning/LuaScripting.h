/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#pragma once
#include "SkinStructs.h"
#include <filesystem>
#include <map>
#include <vector>

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

struct IGame;
struct ScriptState
{
    IGame        *game_state;
    sol::state    state;
    SkinGroup     type;
    sol::function init;
    sol::function update;
};

class LuaScripting
{
public:
    LuaScripting();
    LuaScripting(std::filesystem::path lua_dir_path);
    ~LuaScripting();

    void Reset();

    std::vector<NumericValue>  GetNumeric(SkinGroup group, std::string key);
    std::vector<PositionValue> GetPosition(SkinGroup group, std::string key, int KeyCount);
    std::vector<RectInfo>      GetRect(SkinGroup group, std::string key);
    NoteValue                  GetNote(SkinGroup group, std::string key, int KeyCount);
    SpriteValue                GetSprite(SkinGroup group, std::string key);

    void                       Arena_SetIndex(int index);
    std::vector<NumericValue>  Arena_GetNumeric(std::string key);
    std::vector<PositionValue> Arena_GetPosition(std::string key, int KeyCount);
    std::vector<RectInfo>      Arena_GetRect(std::string key);
    SpriteValue                Arena_GetSprite(std::string key);

    void Update(double delta);

private:
    sol::table LoadLua(sol::state &state, std::filesystem::path path);
    void       TryLoadGroup(SkinGroup group);
    void       TryLoadArena();

    std::map<SkinGroup, ScriptState> m_states;
    std::unique_ptr<ScriptState>     m_arena_states;
    std::map<SkinGroup, std::string> m_expected_files;
    std::filesystem::path            m_lua_dir_path;

    int  m_arena = 0;
    bool m_dispose = false;
};