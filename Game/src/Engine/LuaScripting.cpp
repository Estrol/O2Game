#include "LuaScripting.h"
#include "../Data/Util/Util.hpp"
#include "../EnvironmentSetup.hpp"
#include "SkinManager.hpp" // wtf recursive

struct IGame
{
    int GetArenaIndex()
    {
        return EnvironmentSetup::GetInt("CurrentArena");
    }

    int GetHitPosition()
    {
        auto instance = SkinManager::GetInstance();
        if (!instance) {
            return -1;
        }

        auto value = instance->GetSkinProp("Game", "HitPos", "480");
        return std::stoi(value);
    }

    int GetLaneOffset()
    {
        auto instance = SkinManager::GetInstance();
        if (!instance) {
            return -1;
        }

        auto value = instance->GetSkinProp("Game", "LaneOffset", "5");
        return std::stoi(value);
    }

    std::tuple<int, int> GetResolution()
    {
        auto instance = SkinManager::GetInstance();
        if (!instance) {
            return std::make_tuple(-1, -1);
        }

        auto value = instance->GetSkinProp("Window", "NativeSize", "800x600");
        auto split = splitString(value, 'x');

        auto width = std::stoi(split[0]);
        auto height = std::stoi(split[1]);

        return std::make_tuple(width, height);
    }
};

LuaScripting::LuaScripting()
{
    Reset();
}

LuaScripting::LuaScripting(std::filesystem::path lua_dir_path) : LuaScripting::LuaScripting()
{
    m_lua_dir_path = lua_dir_path;
}

LuaScripting::~LuaScripting()
{
    m_dispose = true;
    Reset();
}

void LuaScripting::Reset()
{
    for (auto &[group, state] : m_states) {
        if (state.game_state) {
            delete state.game_state;
        }
    }

    if (m_arena_states) {
        if (m_arena_states->game_state) {
            delete m_arena_states->game_state;
        }
    }

    m_states.clear();
    m_arena_states.reset();

    if (m_dispose) {
        return;
    }

    m_expected_files = {
        { SkinGroup::SongSelect, "SongSelect.lua" },
        { SkinGroup::MainMenu, "MainMenu.lua" },
        { SkinGroup::Playing, "Playing.lua" },
        { SkinGroup::Notes, "Notes.lua" }
    };
}

sol::table LuaScripting::LoadLua(sol::state &state, std::filesystem::path path)
{
    sol::table result = state.script_file(path.string(), sol::load_mode::text);

    return result;
}

void SetupLuaLibrary(ScriptState &script)
{
    script.state.open_libraries(sol::lib::base, sol::lib::package);

    sol::table HeaderType = script.state.create_table();
    HeaderType["Playing"] = SkinGroup::Playing;
    HeaderType["SongSelect"] = SkinGroup::SongSelect;
    HeaderType["MainMenu"] = SkinGroup::MainMenu;
    HeaderType["Notes"] = SkinGroup::Notes;

    sol::table DataType = script.state.create_table();
    DataType["Numeric"] = SkinDataType::Numeric;
    DataType["Position"] = SkinDataType::Position;
    DataType["Rect"] = SkinDataType::Rect;
    DataType["Note"] = SkinDataType::Note;
    DataType["Sprite"] = SkinDataType::Sprite;

    script.state["HeaderType"] = HeaderType;
    script.state["DataType"] = DataType;

    script.state.new_usertype<IGame>("IGame",
                                     "new", sol::no_constructor,
                                     "GetArenaIndex", &IGame::GetArenaIndex,
                                     "GetHitPosition", &IGame::GetHitPosition,
                                     "GetLaneOffset", &IGame::GetLaneOffset,
                                     "GetResolution", &IGame::GetResolution);

    script.game_state = new IGame();
    script.state["Game"] = script.game_state;
}

void GetLuaState(ScriptState &state, sol::table &table)
{
    if (table["type"].get_type() != sol::type::number) {
        throw std::runtime_error("Script didn't return the expected value");
    }

    state.type = table["type"];

    if (table["init"].get_type() != sol::type::function) {
        throw std::runtime_error("Script did not have init function");
    }

    state.init = table["init"];
}

void LuaScripting::TryLoadGroup(SkinGroup group)
{
    auto fullPath = m_lua_dir_path / m_expected_files[group];
    if (!std::filesystem::exists(fullPath)) {
        throw std::runtime_error("Missing file: " + fullPath.string());
    }

    m_states[group] = {};
    auto &state = m_states[group];
    state.state = sol::state();

    if (state.state.lua_state() == nullptr) {
        throw std::runtime_error("Failed to create lua state");
    }

    SetupLuaLibrary(state);
    sol::table script = LoadLua(state.state, fullPath);
    GetLuaState(state, script);
}

void LuaScripting::TryLoadArena()
{
    auto fullPath = m_lua_dir_path / "Arena.lua";
    if (!std::filesystem::exists(fullPath)) {
        throw std::runtime_error("Missing file: " + fullPath.string());
    }

    m_arena_states = std::make_unique<ScriptState>();
    m_arena_states->state = {};

    if (m_arena_states->state.lua_state() == nullptr) {
        throw std::runtime_error("Failed to create lua state");
    }

    SetupLuaLibrary(*m_arena_states);
    sol::table script = LoadLua(m_arena_states->state, fullPath);

    GetLuaState(*m_arena_states.get(), script);
}

void LuaScripting::Update(double delta)
{
    for (auto &[key, value] : m_states) {
        value.update(delta);
    }
}

std::vector<NumericValue> LuaScripting::GetNumeric(SkinGroup group, std::string key)
{
    try {
        if (m_states[group].init.lua_state() == NULL) {
            throw std::runtime_error("Lua state is null");
        }

        sol::table result_table = m_states[group].init();
        sol::table key_array = result_table[SkinDataType::Numeric][key];

        std::vector<NumericValue> result;
        for (auto &value : key_array) {
            NumericValue numeric_value = {};

            sol::table value_table = value.second;
            numeric_value.X = value_table[1];
            numeric_value.Y = value_table[2];
            numeric_value.MaxDigit = value_table[3];
            std::string direction = value_table[4];
            std::transform(direction.begin(), direction.end(), direction.begin(), ::tolower);

            if (direction == "mid")
                numeric_value.Direction = 0;
            else if (direction == "left")
                numeric_value.Direction = -1;
            else if (direction == "right")
                numeric_value.Direction = 1;
            numeric_value.FillWithZero = value_table[5];

            result.push_back(numeric_value);
        }

        return result;
    } catch (const sol::error &err) {
        throw std::runtime_error(err.what());
    }
}

std::vector<PositionValue> LuaScripting::GetPosition(SkinGroup group, std::string key, int KeyCount)
{
    if (m_states.find(group) == m_states.end()) {
        TryLoadGroup(group);
    }

    try {
        if (m_states[group].init.lua_state() == NULL) {
            throw std::runtime_error("Lua state is null");
        }

        sol::table result_table = m_states[group].init();
        sol::table key_array = result_table[SkinDataType::Position][KeyCount][key];

        std::vector<PositionValue> result;
        for (auto &value : key_array) {
            PositionValue position_value = {};

            sol::table value_table = value.second;
            position_value.X = value_table[1];
            position_value.Y = value_table[2];
            position_value.AnchorPointX = value_table[3];
            position_value.AnchorPointY = value_table[4];
            position_value.RGB[0] = value_table[5];
            position_value.RGB[1] = value_table[6];
            position_value.RGB[2] = value_table[7];

            result.push_back(position_value);
        }

        return result;
    } catch (const sol::error &err) {
        throw std::runtime_error(err.what());
    }
}

std::vector<RectInfo> LuaScripting::GetRect(SkinGroup group, std::string key)
{
    if (m_states.find(group) == m_states.end()) {
        TryLoadGroup(group);
    }

    try {
        if (m_states[group].init.lua_state() == NULL) {
            throw std::runtime_error("Lua state is null");
        }

        std::vector<RectInfo> result;

        sol::table result_table = m_states[group].init();
        sol::table key_array = result_table[SkinDataType::Rect][key];

        for (auto &value : key_array) {
            RectInfo rect_info = {};

            sol::table value_table = value.second;
            rect_info.X = value_table[1];
            rect_info.Y = value_table[2];
            rect_info.Width = value_table[3];
            rect_info.Height = value_table[4];

            result.push_back(rect_info);
        }

        return result;
    } catch (const sol::error &err) {
        throw std::runtime_error(err.what());
    }
}

NoteValue LuaScripting::GetNote(SkinGroup group, std::string key, int KeyCount)
{
    if (m_states.find(group) == m_states.end()) {
        TryLoadGroup(group);
    }

    try {
        if (m_states[group].init.lua_state() == NULL) {
            throw std::runtime_error("Lua state is null");
        }

        sol::table result_table = m_states[group].init();
        sol::table key_array = result_table[SkinDataType::Note][KeyCount][key];

        NoteValue note_value = {};
        note_value.numOfFiles = key_array[1];
        note_value.fileName = key_array[2];

        return note_value;
    } catch (const sol::error &err) {
        throw std::runtime_error(err.what());
    }
}

SpriteValue LuaScripting::GetSprite(SkinGroup group, std::string key)
{
    if (m_states.find(group) == m_states.end()) {
        TryLoadGroup(group);
    }

    try {
        if (m_states[group].init.lua_state() == NULL) {
            throw std::runtime_error("Lua state is null");
        }

        sol::table result_table = m_states[group].init();
        sol::table key_array = result_table[SkinDataType::Sprite][key];

        SpriteValue sprite_value = {};
        sprite_value.numOfFrames = key_array[1];
        sprite_value.X = key_array[2];
        sprite_value.Y = key_array[3];
        sprite_value.AnchorPointX = key_array[4];
        sprite_value.AnchorPointY = key_array[5];
        sprite_value.FrameTime = key_array[6];

        return sprite_value;
    } catch (const sol::error &err) {
        throw std::runtime_error(err.what());
    }
}

void LuaScripting::Arena_SetIndex(int index)
{
    if (m_arena != index) {
        m_arena_states.reset();
    }

    m_arena = index;
}

std::vector<NumericValue> LuaScripting::Arena_GetNumeric(std::string key)
{
    if (!m_arena_states) {
        TryLoadArena();
    }

    try {
        sol::table result_table = m_arena_states->init();
        if (sol::type::table != result_table.get_type()) {
            throw std::runtime_error("expected returned function is table!");
        }

        if (sol::type::table != result_table[SkinDataType::Numeric].get_type()) {
            throw std::runtime_error("The table missing DataType.Numeric");
        }

        sol::table key_array = result_table[SkinDataType::Numeric][key];
        if (sol::type::table != key_array.get_type()) {
            throw std::runtime_error("The table has no key: " + key);
        }

        std::vector<NumericValue> result;
        for (auto &value : key_array) {
            if (value.second.get_type() != sol::type::table) {
                throw std::runtime_error("Array key is not table");
            }

            NumericValue numeric_value = {};

            sol::table value_table = value.second;
            if (value_table.size() < 5) {
                throw std::runtime_error("Array key table size is less than 7");
            }

            auto x = value_table[1];
            if (x.get_type() != sol::type::number) {
                throw std::runtime_error("Numeric::X is not a number, Key: " + key);
            }

            auto y = value_table[2];
            if (y.get_type() != sol::type::number) {
                throw std::runtime_error("Numeric::Y is not a number, Key: " + key);
            }

            auto maxDigit = value_table[3];
            if (maxDigit.get_type() != sol::type::number) {
                throw std::runtime_error("Numeric::MaxDigit is not a number, Key: " + key);
            }

            numeric_value.X = x;
            numeric_value.Y = y;
            numeric_value.MaxDigit = maxDigit;

            auto direct = value_table[4];
            if (direct.get_type() == sol::type::string) {
                std::string direction = value_table[4];
                std::transform(direction.begin(), direction.end(), direction.begin(), ::tolower);

                if (direction == "mid")
                    numeric_value.Direction = 0;
                else if (direction == "left")
                    numeric_value.Direction = -1;
                else if (direction == "right")
                    numeric_value.Direction = 1;
            } else if (direct.get_type() == sol::type::number) {
                numeric_value.Direction = std::clamp((int)direct, -1, 1);
            } else {
                throw std::runtime_error("Numeric::Direction is not either LEFT, MID, RIGHT or -1, 0, 1, Key: " + key);
            }

            auto fillWithZero = value_table[5];
            if (fillWithZero.get_type() != sol::type::number && fillWithZero.get_type() != sol::type::boolean) {
                throw std::runtime_error("Numeric::FillWithZero is not number or boolean, Key: " + key);
            }

            if (fillWithZero.get_type() == sol::type::boolean) {
                numeric_value.FillWithZero = fillWithZero;
            } else {
                int value = std::clamp((int)fillWithZero, 0, 1);
                numeric_value.FillWithZero = value > 0;
            }

            result.push_back(numeric_value);
        }

        return result;
    } catch (const sol::error &err) {
        throw std::runtime_error(err.what());
    }
}

std::vector<PositionValue> LuaScripting::Arena_GetPosition(std::string key, int KeyCount)
{
    try {
        if (!m_arena_states) {
            TryLoadArena();
        }

        sol::table result_table = m_arena_states->init();
        if (sol::type::table != result_table.get_type()) {
            throw std::runtime_error("expected returned function is table, but got other than table");
        }

        if (sol::type::table != result_table[SkinDataType::Position].get_type()) {
            throw std::runtime_error("The table missing DataType.Position");
        }

        sol::table key_array = result_table[SkinDataType::Position][KeyCount];
        if (sol::type::table != key_array.get_type()) {
            throw std::runtime_error("The position table has no item for KeyCount: " + std::to_string(KeyCount));
        }

        key_array = key_array[key];
        if (sol::type::table != key_array.get_type()) {
            throw std::runtime_error("The table has no key: " + key);
        }

        std::string prefix = "Positions::" + std::to_string(KeyCount) + "::";

        std::vector<PositionValue> result;
        for (auto &value : key_array) {
            if (value.second.get_type() != sol::type::table) {
                throw std::runtime_error("Array key is not table");
            }

            PositionValue position_value = {};

            sol::table value_table = value.second;
            if (value_table.size() < 7) {
                throw std::runtime_error("Array key table size is less than 7");
            }

            auto item1 = value_table[1];
            auto item2 = value_table[2];
            auto item3 = value_table[3];
            auto item4 = value_table[4];

            if (item1.get_type() != sol::type::number) {
                throw std::runtime_error(prefix + "X is not a number");
            }

            if (item2.get_type() != sol::type::number) {
                throw std::runtime_error(prefix + "Y is not a number");
            }

            if (item3.get_type() != sol::type::number) {
                throw std::runtime_error(prefix + "AnchorPointX is not a number");
            }

            if (item4.get_type() != sol::type::number) {
                throw std::runtime_error(prefix + "AnchorPointY is not a number");
            }

            position_value.X = item1;
            position_value.Y = item2;
            position_value.AnchorPointX = item3;
            position_value.AnchorPointY = item4;

            auto item5 = value_table[5];
            auto item6 = value_table[6];
            auto item7 = value_table[7];

            if (item5.get_type() != sol::type::number) {
                throw std::runtime_error(prefix + "RGB index Red is not a number");
            }

            if (item6.get_type() != sol::type::number) {
                throw std::runtime_error(prefix + "RGB index Green is not a number");
            }

            if (item7.get_type() != sol::type::number) {
                throw std::runtime_error(prefix + "RGB index Blue is not a number");
            }

            int r = std::clamp((int)item5, 0, 255);
            int g = std::clamp((int)item6, 0, 255);
            int b = std::clamp((int)item7, 0, 255);

            position_value.RGB[0] = r;
            position_value.RGB[1] = g;
            position_value.RGB[2] = b;

            result.push_back(position_value);
        }

        return result;
    } catch (const sol::error &err) {
        throw std::runtime_error(err.what());
    }
}

std::vector<RectInfo> LuaScripting::Arena_GetRect(std::string key)
{
    try {
        if (!m_arena_states) {
            TryLoadArena();
        }

        sol::table result_table = m_arena_states->init();
        if (sol::type::table != result_table.get_type()) {
            throw std::runtime_error("expected returned function is table!");
        }

        if (sol::type::table != result_table[SkinDataType::Rect].get_type()) {
            throw std::runtime_error("The table missing DataType.Rect");
        }

        sol::table key_array = result_table[SkinDataType::Rect][key];
        if (sol::type::table != key_array.get_type()) {
            throw std::runtime_error("The table has no key: " + key);
        }

        std::vector<RectInfo> result;

        for (auto &value : key_array) {
            if (value.second.get_type() != sol::type::table) {
                throw std::runtime_error("Array key is not table");
            }

            RectInfo rect_info = {};

            sol::table value_table = value.second;
            if (value_table.size() < 4) {
                throw std::runtime_error("Array key table size is less than 4");
            }

            auto item1 = value_table[1];
            auto item2 = value_table[2];
            auto item3 = value_table[3];
            auto item4 = value_table[4];

            if (item1.get_type() != sol::type::number) {
                throw std::runtime_error("Array key table item 1 is not number");
            }

            if (item2.get_type() != sol::type::number) {
                throw std::runtime_error("Array key table item 2 is not number");
            }

            if (item3.get_type() != sol::type::number) {
                throw std::runtime_error("Array key table item 3 is not number");
            }

            if (item4.get_type() != sol::type::number) {
                throw std::runtime_error("Array key table item 4 is not number");
            }

            rect_info.X = item1;
            rect_info.Y = item2;
            rect_info.Width = item3;
            rect_info.Height = item4;

            result.push_back(rect_info);
        }

        return result;
    } catch (const sol::error &err) {
        throw std::runtime_error(err.what());
    }
}

SpriteValue LuaScripting::Arena_GetSprite(std::string key)
{
    try {
        if (!m_arena_states) {
            TryLoadArena();
        }

        sol::table result_table = m_arena_states->init();
        if (sol::type::table != result_table.get_type()) {
            throw std::runtime_error("expected returned function is table!");
        }

        if (sol::type::table != result_table[SkinDataType::Sprite].get_type()) {
            throw std::runtime_error("The table missing DataType.Sprite");
        }

        sol::table key_array = result_table[SkinDataType::Sprite][key];
        if (sol::type::table != key_array.get_type()) {
            throw std::runtime_error("The table has no key: " + key);
        }

        if (key_array.size() < 6) {
            throw std::runtime_error("Expect table size is 6 at key: " + key);
        }

        SpriteValue sprite_value = {};

        auto item1 = key_array[1];
        if (sol::type::number != item1.get_type()) {
            throw std::runtime_error("Sprite value 1 is not a number");
        }

        sprite_value.numOfFrames = item1;

        auto item2 = key_array[2];
        if (sol::type::number != item2.get_type()) {
            throw std::runtime_error(key + ":Sprite value 2 is not a number");
        }

        sprite_value.X = item2;

        auto item3 = key_array[3];
        if (sol::type::number != item3.get_type()) {
            throw std::runtime_error(key + ":Sprite value 3 is not a number");
        }
        sprite_value.Y = item3;

        auto item4 = key_array[4];
        if (sol::type::number != item4.get_type()) {
            throw std::runtime_error(key + ":Sprite value 4 is not a number");
        }
        sprite_value.AnchorPointX = item4;

        auto item5 = key_array[5];
        if (sol::type::number != item5.get_type()) {
            throw std::runtime_error(key + ":Sprite value 5 is not a number");
        }
        sprite_value.AnchorPointY = item5;

        auto item6 = key_array[6];
        if (sol::type::number != item6.get_type()) {
            throw std::runtime_error(key + ":Sprite value 6 is not a number");
        }
        sprite_value.FrameTime = item6;

        return sprite_value;
    } catch (const sol::error &err) {
        throw std::runtime_error(err.what());
    }
}
