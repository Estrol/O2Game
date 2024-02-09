#include "../../Env.h"
#include "LuaSkin.h"

#include <Exceptions/EstException.h>
#include <Graphics/NativeWindow.h>
#include <map>

static std::map<SkinGroup, std::string> ExpectedFiles = {
    { SkinGroup::Main, "Main.lua" },
    { SkinGroup::MainMenu, "MainMenu.lua" },
    { SkinGroup::Notes, "Notes.lua" },
    { SkinGroup::Playing, "Playing.lua" },
    { SkinGroup::Arena, "Arena.lua" },
    { SkinGroup::SongSelect, "SongSelect.lua" },
    { SkinGroup::Result, "Result.lua" },
    { SkinGroup::Audio, "Audio.lua" }
};

static std::map<SkinGroup, std::string> ExpectedGroups = {
    { SkinGroup::Main, "Main" },
    { SkinGroup::MainMenu, "MainMenu" },
    { SkinGroup::Notes, "Notes" },
    { SkinGroup::Playing, "Playing" },
    { SkinGroup::Arena, "Arena" },
    { SkinGroup::SongSelect, "SongSelect" },
    { SkinGroup::Result, "Result" },
    { SkinGroup::Audio, "Audio" }
};

inline void panic_handler(sol::optional<std::string> maybe_msg)
{
    if (maybe_msg) {
        throw Exceptions::EstException("LuaSkin fail: %s", maybe_msg.value().c_str());
    } else {
        throw Exceptions::EstException("An unexpected error occurred and panic has been invoked");
    }
}

void LuaSkin::LoadSkin(std::string skinName)
{
    CurrentPath = std::filesystem::current_path() / "Skins" / skinName;
    if (!std::filesystem::exists(CurrentPath)) {
        throw Exceptions::EstException("Skin %s does not exist", skinName.c_str());
    }

    auto path = CurrentPath / "GameSkin.ini";
    if (!std::filesystem::exists(path)) {
        throw Exceptions::EstException("GameSkin.ini does not exist");
    }

    Misc::mINI::INIFile file(path.string());
    file.read(ini);
}

void LuaSkin::LoadScript(SkinGroup group)
{
    auto file = ExpectedFiles[group];
    auto path = std::filesystem::path(GetPath()) / "Scripts" / file;
    if (!std::filesystem::exists(path)) {
        throw Exceptions::EstException("Script %s does not exist", ExpectedFiles[group].c_str());
    }

    CurrentGroup = group;

    Script.table.reset();
    Script.state = std::make_shared<sol::state>();
    sol::state &state = *Script.state.get();

    state.open_libraries(
        sol::lib::base,
        sol::lib::math,
        sol::lib::string,
        sol::lib::table,
        sol::lib::io,
        sol::lib::package);

    sol::table HeaderType = state.create_table();
    HeaderType["Playing"] = SkinGroup::Playing;
    HeaderType["SongSelect"] = SkinGroup::SongSelect;
    HeaderType["MainMenu"] = SkinGroup::MainMenu;
    HeaderType["Arena"] = SkinGroup::Arena;
    HeaderType["Notes"] = SkinGroup::Notes;
    HeaderType["Audio"] = SkinGroup::Audio;

    sol::table DataType = state.create_table();
    DataType["Numeric"] = SkinDataType::Numeric;
    DataType["Position"] = SkinDataType::Position;
    DataType["Rect"] = SkinDataType::Rect;
    DataType["Note"] = SkinDataType::Note;
    DataType["Sprite"] = SkinDataType::Sprite;
    DataType["Tween"] = SkinDataType::Tween;
    DataType["Audio"] = SkinDataType::Audio;

    sol::table TweenType = state.create_table();
    TweenType["Linear"] = TweenType::Linear;
    TweenType["Quadratic"] = TweenType::Quadratic;
    TweenType["Cubic"] = TweenType::Cubic;
    TweenType["Quartic"] = TweenType::Quartic;
    TweenType["Quintic"] = TweenType::Quintic;
    TweenType["Sinusoidal"] = TweenType::Sinusoidal;
    TweenType["Exponential"] = TweenType::Exponential;
    TweenType["Circular"] = TweenType::Circular;
    TweenType["Elastic"] = TweenType::Elastic;
    TweenType["Back"] = TweenType::Back;
    TweenType["Bounce"] = TweenType::Bounce;

    sol::table NumericDirection = state.create_table();
    NumericDirection["Left"] = SkinNumericDirection::Left;
    NumericDirection["Mid"] = SkinNumericDirection::Mid;
    NumericDirection["Right"] = SkinNumericDirection::Right;

    sol::table BGMType = state.create_table();
    BGMType["Lobby"] = SkinAudioType::BGM_Lobby;
    BGMType["Waiting"] = SkinAudioType::BGM_Waiting;
    BGMType["Result"] = SkinAudioType::BGM_Result;

    sol::table Enums = state.create_table();
    Enums["HeaderType"] = HeaderType;
    Enums["DataType"] = DataType;
    Enums["TweenType"] = TweenType;
    Enums["NumericDirection"] = NumericDirection;
    Enums["AudioType"] = BGMType;

    state["Enum"] = Enums;

    state.new_usertype<GameLua>(
        "GameLua",
        "new", sol::no_constructor,
        "GetArenaIndex", &GameLua::GetArenaIndex,
        "GetHitPosition", &GameLua::GetHitPosition,
        "GetLaneOffset", &GameLua::GetLaneOffset,
        "GetResolution", &GameLua::GetResolution,
        "GetKeyCount", &GameLua::GetKeyCount,
        "GetSkinPath", &GameLua::GetSkinPath,
        "GetScriptPath", &GameLua::GetScriptPath,
        "IsPathExist", &GameLua::IsPathExist);

    Script.game = std::make_shared<GameLua>();
    Script.game->__skin = this;
    Script.game->__group = ExpectedGroups[group];
    state["Game"] = Script.game.get();

    try {
        sol::table table = state.script_file(
            path.string(),
            sol::load_mode::text);

        Script.table = std::make_shared<sol::table>(std::move(table));

    } catch (const sol::error &e) {
        throw Exceptions::EstException("Failed to load script [%s]: %s", file.c_str(), e.what());
    }
}

const char *get_type_name(sol::type type)
{
    switch (type) {
        case sol::type::none:
            return "none";
        case sol::type::nil:
            return "nil";
        case sol::type::boolean:
            return "boolean";
        case sol::type::number:
            return "number";
        case sol::type::string:
            return "string";
        case sol::type::table:
            return "table";
        case sol::type::function:
            return "function";
        case sol::type::userdata:
            return "userdata";
        case sol::type::thread:
            return "thread";
        case sol::type::lightuserdata:
            return "lightuserdata";
        case sol::type::poly:
            return "poly";
        default:
            return "unknown";
    }
}

std::vector<NumericValue> LuaSkin::GetNumeric(std::string key)
{
    try {
        sol::table &script_table = *Script.table.get();
        sol::table  table = script_table["Data"];
        auto        luaFileName = ExpectedFiles[CurrentGroup];

        // check if the key exists
        if (table[SkinDataType::Numeric][key] == sol::nil || !table[SkinDataType::Numeric][key].is<sol::table>()) {
            throw Exceptions::EstException("[Numeric] [%s] Key '%s' does not exist or not table", luaFileName.c_str(), key.c_str());
        }

        sol::table key_array = table[SkinDataType::Numeric][key];

        std::vector<NumericValue> result;
        for (auto &value : key_array) {
            if (!value.second.is<sol::table>()) {
                throw Exceptions::EstException(
                    "[Numeric] %s at key: '%s', Value is not a table but got %s",
                    luaFileName.c_str(),
                    key.c_str(),
                    get_type_name(value.second.get_type()));
            }

            NumericValue numeric_value = {};
            sol::table   value_table = value.second;

            if (value_table["Files"] == sol::nil || !value_table["Files"].is<sol::table>()) {
                throw Exceptions::EstException(
                    "[Numeric] %s at key: '%s', Files field is not a table or nil but got %s",
                    luaFileName.c_str(),
                    key.c_str(),
                    get_type_name(value_table[1].get_type()));
            }

            sol::table files = value_table["Files"];
            for (int i = 1; i <= files.size(); i++) {
                if (!files[i].is<std::string>()) {
                    throw Exceptions::EstException(
                        "[Numeric] %s at key: '%s', File %d is not a string but got %s",
                        luaFileName.c_str(),
                        key.c_str(),
                        i, get_type_name(files[i].get_type()));
                }

                std::string file = files[i];
                numeric_value.Files.push_back(file);
            }

            if (value_table["Position"] == sol::nil || !value_table["Position"].is<sol::table>()) {
                throw Exceptions::EstException(
                    "[Numeric] %s at key: '%s', Position field is not a table or nil but got %s",
                    luaFileName.c_str(),
                    key.c_str(),
                    get_type_name(value_table["Position"].get_type()));
            }

            sol::table position = value_table["Position"];

            if (position[1] == sol::nil || !position[1].is<double>()) {
                throw Exceptions::EstException("[Numeric] %s at key: '%s', Position[1] is not a number or nil", luaFileName.c_str(), key.c_str());
            }

            if (position[2] == sol::nil || !position[2].is<double>()) {
                throw Exceptions::EstException("[Numeric] %s at key: '%s', Position[2] is not a number or nil", luaFileName.c_str(), key.c_str());
            }

            double x = position[1];
            double y = position[2];

            numeric_value.Position = UDim2::fromOffset(x, y);

            if (value_table["Size"] == sol::nil || !value_table["Size"].is<sol::table>()) {
                throw Exceptions::EstException(
                    "[Numeric] %s at key: '%s', Size field is not a table or nil but got %s",
                    luaFileName.c_str(),
                    key.c_str(),
                    get_type_name(value_table["Size"].get_type()));
            }

            sol::table size = value_table["Size"];

            if (size[1] == sol::nil || !size[1].is<double>()) {
                throw Exceptions::EstException("[Numeric] %s at key: '%s', Size[1] is not a number or nil", luaFileName.c_str(), key.c_str());
            }

            if (size[2] == sol::nil || !size[2].is<double>()) {
                throw Exceptions::EstException("[Numeric] %s at key: '%s', Size[2] is not a number or nil", luaFileName.c_str(), key.c_str());
            }

            double width = size[1];
            double height = size[2];

            numeric_value.Size = UDim2::fromOffset(width, height);

            if (value_table["MaxDigit"] == sol::nil || !value_table["MaxDigit"].is<int>()) {
                throw Exceptions::EstException(
                    "[Numeric] %s at key: '%s', MaxDigit field is not a number or nil but got %s",
                    luaFileName.c_str(),
                    key.c_str(),
                    get_type_name(value_table["MaxDigit"].get_type()));
            }

            numeric_value.MaxDigit = value_table["MaxDigit"];

            if (value_table["Direction"] == sol::nil || !value_table["Direction"].is<int>()) {
                throw Exceptions::EstException(
                    "[Numeric] %s at key: '%s', Direction field is not a number or nil but got %s",
                    luaFileName.c_str(),
                    key.c_str(),
                    get_type_name(value_table["Direction"].get_type()));
            }

            numeric_value.Direction = value_table["Direction"];

            if (value_table["FillWithZero"] == sol::nil || !value_table["FillWithZero"].is<bool>()) {
                throw Exceptions::EstException(
                    "[Numeric] %s at key: '%s', FillWithZero field is not a boolean or nil but got %s",
                    luaFileName.c_str(),
                    key.c_str(),
                    get_type_name(value_table["FillWithZero"].get_type()));
            }

            numeric_value.FillWithZero = value_table["FillWithZero"];

            if (value_table["Color"] == sol::nil || !value_table["Color"].is<sol::table>()) {
                throw Exceptions::EstException(
                    "[Numeric] %s at key: '%s', Color field is not a table or nil but got %s",
                    luaFileName.c_str(),
                    key.c_str(),
                    get_type_name(value_table["Color"].get_type()));
            }

            {
                sol::table rgb = value_table["Color"];

                if (rgb[1] == sol::nil || !rgb[1].is<int>()) {
                    throw Exceptions::EstException("[Numeric] %s at key: '%s', Color[1] is not a number or nil", luaFileName.c_str(), key.c_str());
                }

                if (rgb[2] == sol::nil || !rgb[2].is<int>()) {
                    throw Exceptions::EstException("[Numeric] %s at key: '%s', Color[2] is not a number or nil", luaFileName.c_str(), key.c_str());
                }

                if (rgb[3] == sol::nil || !rgb[3].is<int>()) {
                    throw Exceptions::EstException("[Numeric] %s at key: '%s', Color[3] is not a number or nil", luaFileName.c_str(), key.c_str());
                }

                numeric_value.Color = Color3::fromRGB(
                    rgb[1],
                    rgb[2],
                    rgb[3]);
            }

            result.push_back(numeric_value);
        }

        if (result.empty()) {
            throw Exceptions::EstException("[Numeric] %s at key: '%s', NumericValue is empty", luaFileName.c_str(), key.c_str());
        }

        return result;
    } catch (const sol::error &err) {
        throw Exceptions::EstException("[Numeric] %s, (key = %s)", err.what(), key.c_str());
    }
}

std::vector<PositionValue> LuaSkin::GetPosition(std::string key)
{
    try {
        sol::table &script_table = *Script.table.get();
        sol::table  table = script_table["Data"];
        auto        luaFileName = ExpectedFiles[CurrentGroup];

        // check if the key exists
        if (table[SkinDataType::Position][key] == sol::nil || !table[SkinDataType::Position][key].is<sol::table>()) {
            throw Exceptions::EstException("[Position] [%s] Key '%s' does not exist or not table", luaFileName.c_str(), key.c_str());
        }

        sol::table key_array = table[SkinDataType::Position][key];

        std::vector<PositionValue> result;
        for (auto &data : key_array) {
            if (!data.second.is<sol::table>()) {
                throw Exceptions::EstException(
                    "[Position] %s at key: '%s', Value is not a table but got %s",
                    luaFileName.c_str(),
                    key.c_str(),
                    get_type_name(data.second.get_type()));
            }

            PositionValue position_value = {};
            sol::table    value_table = data.second;

            if (value_table["Path"] == sol::nil || !value_table["Path"].is<std::string>()) {
                throw Exceptions::EstException(
                    "[Position] %s at key: '%s', Path field is not a string or nil but got %s",
                    luaFileName.c_str(),
                    key.c_str(),
                    get_type_name(value_table["Path"].get_type()));
            }

            std::string path = value_table["Path"];
            position_value.Path = path;

            if (value_table["Position"] == sol::nil || !value_table["Position"].is<sol::table>()) {
                throw Exceptions::EstException(
                    "[Position] %s at key: '%s', Position field is not a table or nil but got %s",
                    luaFileName.c_str(),
                    key.c_str(),
                    get_type_name(value_table["Position"].get_type()));
            }

            sol::table position = value_table["Position"];

            if (position[1] == sol::nil || !position[1].is<double>()) {
                throw Exceptions::EstException("[Position] %s at key: '%s', Position[1] is not a number or nil", luaFileName.c_str(), key.c_str());
            }

            if (position[2] == sol::nil || !position[2].is<double>()) {
                throw Exceptions::EstException("[Position] %s at key: '%s', Position[2] is not a number or nil", luaFileName.c_str(), key.c_str());
            }

            double x = position[1];
            double y = position[2];

            position_value.Position = UDim2::fromOffset(x, y);

            if (value_table["Size"] == sol::nil || !value_table["Size"].is<sol::table>()) {
                throw Exceptions::EstException(
                    "[Position] %s at key: '%s', Size field is not a table or nil but got %s",
                    luaFileName.c_str(),
                    key.c_str(),
                    get_type_name(value_table["Size"].get_type()));
            }

            sol::table size = value_table["Size"];

            if (size[1] == sol::nil || !size[1].is<double>()) {
                throw Exceptions::EstException("[Position] %s at key: '%s', Size[1] is not a number or nil", luaFileName.c_str(), key.c_str());
            }

            if (size[2] == sol::nil || !size[2].is<double>()) {
                throw Exceptions::EstException("[Position] %s at key: '%s', Size[2] is not a number or nil", luaFileName.c_str(), key.c_str());
            }

            double width = size[1];
            double height = size[2];

            position_value.Size = UDim2::fromOffset(width, height);

            if (value_table["AnchorPoint"] == sol::nil || !value_table["AnchorPoint"].is<sol::table>()) {
                throw Exceptions::EstException(
                    "[Position] %s at key: '%s', AnchorPoint field is not a table or nil but got %s",
                    luaFileName.c_str(),
                    key.c_str(),
                    get_type_name(value_table["AnchorPoint"].get_type()));
            }

            sol::table anchorPoint = value_table["AnchorPoint"];
            position_value.AnchorPoint = Vector2(anchorPoint[1], anchorPoint[2]);

            if (value_table["Color"] == sol::nil || !value_table["Color"].is<sol::table>()) {
                throw Exceptions::EstException(
                    "[Position] %s at key: '%s', Color field is not a table or nil but got %s",
                    luaFileName.c_str(),
                    key.c_str(),
                    get_type_name(value_table["Color"].get_type()));
            }

            {
                sol::table rgb = value_table["Color"];

                if (rgb[1] == sol::nil || !rgb[1].is<int>()) {
                    throw Exceptions::EstException("[Position] %s at key: '%s', Color[1] is not a number or nil", luaFileName.c_str(), key.c_str());
                }

                if (rgb[2] == sol::nil || !rgb[2].is<int>()) {
                    throw Exceptions::EstException("[Position] %s at key: '%s', Color[2] is not a number or nil", luaFileName.c_str(), key.c_str());
                }

                if (rgb[3] == sol::nil || !rgb[3].is<int>()) {
                    throw Exceptions::EstException("[Position] %s at key: '%s', Color[3] is not a number or nil", luaFileName.c_str(), key.c_str());
                }

                position_value.Color = Color3::fromRGB(
                    rgb[1],
                    rgb[2],
                    rgb[3]);
            }

            result.push_back(position_value);
        }

        return result;
    } catch (const sol::error &err) {
        throw Exceptions::EstException("[Position] %s, (key = %s)", err.what(), key.c_str());
    }
}

std::vector<RectInfo> LuaSkin::GetRect(std::string key)
{
    try {
        sol::table &script_table = *Script.table.get();
        sol::table  table = script_table["Data"];
        auto        luaFileName = ExpectedFiles[CurrentGroup];

        // check if the key exists
        if (table[SkinDataType::Rect][key] == sol::nil || !table[SkinDataType::Rect][key].is<sol::table>()) {
            throw Exceptions::EstException("[Rect] [%s] Key '%s' does not exist or not table", luaFileName.c_str(), key.c_str());
        }

        sol::table key_array = table[SkinDataType::Rect][key];

        std::vector<RectInfo> result;
        for (auto &value : key_array) {
            if (!value.second.is<sol::table>()) {
                throw Exceptions::EstException(
                    "[Rect] %s at key: '%s', Value is not a table but got %s",
                    luaFileName.c_str(),
                    key.c_str(),
                    get_type_name(value.second.get_type()));
            }

            RectInfo rect_info = {};

            sol::table value_table = value.second;

            if (value_table["Position"] == sol::nil || !value_table["Position"].is<sol::table>()) {
                throw Exceptions::EstException(
                    "[Rect] %s at key: '%s', Position field is not a table or nil but got %s",
                    luaFileName.c_str(),
                    key.c_str(),
                    get_type_name(value_table["Position"].get_type()));
            }

            sol::table position = value_table["Position"];

            if (position[1] == sol::nil || !position[1].is<double>()) {
                throw Exceptions::EstException("[Rect] %s at key: '%s', Position[1] is not a number or nil", luaFileName.c_str(), key.c_str());
            }

            if (position[2] == sol::nil || !position[2].is<double>()) {
                throw Exceptions::EstException("[Rect] %s at key: '%s', Position[2] is not a number or nil", luaFileName.c_str(), key.c_str());
            }

            double x = position[1];
            double y = position[2];

            rect_info.Position = UDim2::fromOffset(x, y);

            if (value_table["Size"] == sol::nil || !value_table["Size"].is<sol::table>()) {
                throw Exceptions::EstException(
                    "[Rect] %s at key: '%s', Size field is not a table or nil but got %s",
                    luaFileName.c_str(),
                    key.c_str(),
                    get_type_name(value_table["Size"].get_type()));
            }

            sol::table size = value_table["Size"];

            if (size[1] == sol::nil || !size[1].is<double>()) {
                throw Exceptions::EstException("[Rect] %s at key: '%s', Size[1] is not a number or nil", luaFileName.c_str(), key.c_str());
            }

            if (size[2] == sol::nil || !size[2].is<double>()) {
                throw Exceptions::EstException("[Rect] %s at key: '%s', Size[2] is not a number or nil", luaFileName.c_str(), key.c_str());
            }

            double width = size[1];
            double height = size[2];

            rect_info.Size = UDim2::fromOffset(width, height);

            result.push_back(rect_info);
        }

        if (result.empty()) {
            throw Exceptions::EstException("[Rect] %s at key: '%s', RectInfo is empty", luaFileName.c_str(), key.c_str());
        }

        return result;
    } catch (const sol::error &err) {
        throw Exceptions::EstException("[Rect] %s, (key = %s)", err.what(), key.c_str());
    }
}

std::vector<AudioInfo> LuaSkin::GetAudio()
{
    try {
        sol::table &script_table = *Script.table.get();
        sol::table  table = script_table["Data"];
        auto        luaFileName = ExpectedFiles[CurrentGroup];

        // check if the key exists
        if (table[SkinDataType::Audio] == sol::nil || !table[SkinDataType::Audio].is<sol::table>()) {
            throw Exceptions::EstException("[Audio] [%s] Key 'Audio' does not exist or not table", luaFileName.c_str());
        }

        sol::table key_array = table[SkinDataType::Audio];

        std::vector<AudioInfo> result;
        for (auto &value : key_array) {
            if (!value.second.is<sol::table>()) {
                throw Exceptions::EstException(
                    "[Audio] %s at key: 'Audio', Value is not a table but got %s",
                    luaFileName.c_str(),
                    get_type_name(value.second.get_type()));
            }

            AudioInfo audio_info = {};

            sol::table value_table = value.second;

            if (value_table["Path"] == sol::nil || !value_table["Path"].is<std::string>()) {
                throw Exceptions::EstException(
                    "[Audio] %s at key: Audio', Path field is not a string or nil but got %s",
                    luaFileName.c_str(),
                    get_type_name(value_table["Path"].get_type()));
            }

            std::string path = value_table["Path"];
            audio_info.Path = path;

            if (value_table["Type"] == sol::nil || !value_table["Type"].is<int>()) {
                throw Exceptions::EstException(
                    "[Audio] %s at key: Audio', Type field is not a number or nil but got %s",
                    luaFileName.c_str(),
                    get_type_name(value_table["Type"].get_type()));
            }

            audio_info.Type = value_table["Type"];

            result.push_back(audio_info);
        }

        if (result.empty()) {
            throw Exceptions::EstException("[Audio] %s at key: 'Audio', AudioInfo is empty", luaFileName.c_str());
        }

        return result;
    } catch (const sol::error &err) {
        throw Exceptions::EstException("[Audio] %s, (key = Audio)", err.what());
    }
}

NoteValue LuaSkin::GetNote(std::string key)
{
    try {
        sol::table &script_table = *Script.table.get();
        sol::table  table = script_table["Data"];
        auto        luaFileName = ExpectedFiles[CurrentGroup];

        // check if the key exists
        if (table[SkinDataType::Note][key] == sol::nil || !table[SkinDataType::Note][key].is<sol::table>()) {
            throw Exceptions::EstException("[%s] Key '%s' does not exist or not table", luaFileName.c_str(), key.c_str());
        }

        sol::table key_array = table[SkinDataType::Note][key];

        if (key_array["Files"] == sol::nil || !key_array["Files"].is<sol::table>()) {
            throw Exceptions::EstException(
                "[Note] %s at key: '%s', Files field is not a table or nil but got %s",
                luaFileName.c_str(),
                key.c_str(),
                get_type_name(key_array[1].get_type()));
        }

        NoteValue note_value = {};

        sol::table files = key_array["Files"];
        for (int i = 1; i <= files.size(); i++) {
            std::string file = files[i];
            note_value.Files.push_back(file);
        }

        if (key_array["Size"] == sol::nil || !key_array["Size"].is<sol::table>()) {
            throw Exceptions::EstException(
                "[Note] %s at key: '%s', Size field is not a table or nil but got %s",
                luaFileName.c_str(),
                key.c_str(),
                get_type_name(key_array["Size"].get_type()));
        }

        sol::table size = key_array["Size"];

        if (size[1] == sol::nil || !size[1].is<double>()) {
            throw Exceptions::EstException("[Note] %s at key: '%s', Size[1] is not a number or nil", luaFileName.c_str(), key.c_str());
        }

        if (size[2] == sol::nil || !size[2].is<double>()) {
            throw Exceptions::EstException("[Note] %s at key: '%s', Size[2] is not a number or nil", luaFileName.c_str(), key.c_str());
        }

        note_value.Size = UDim2::fromOffset(size[1], size[2]);

        if (key_array["FrameTime"] == sol::nil || !key_array["FrameTime"].is<double>()) {
            throw Exceptions::EstException(
                "[Note] %s at key: '%s', FrameTime field is not a number or nil but got %s",
                luaFileName.c_str(),
                key.c_str(),
                get_type_name(key_array["FrameTime"].get_type()));
        }

        note_value.FrameTime = key_array["FrameTime"];

        sol::table color = key_array["Color"];
        note_value.Color = Color3::fromRGB(
            color[1],
            color[2],
            color[3]);

        return note_value;
    } catch (const sol::error &err) {
        throw Exceptions::EstException("%s, (key = %s)", err.what(), key.c_str());
    }
}

SpriteValue LuaSkin::GetSprite(std::string key)
{
    try {
        sol::table &script_table = *Script.table.get();
        sol::table  table = script_table["Data"];
        auto        luaFileName = ExpectedFiles[CurrentGroup];

        // check if the key exists
        if (table[SkinDataType::Sprite][key] == sol::nil || !table[SkinDataType::Sprite][key].is<sol::table>()) {
            throw Exceptions::EstException("[Sprite] Key %s does not exist", key.c_str());
        }

        sol::table key_array = table[SkinDataType::Sprite][key];

        if (key_array["Files"] == sol::nil || !key_array["Files"].is<sol::table>()) {
            throw Exceptions::EstException(
                "[Sprite] %s at key: '%s', Files field is not a table or nil but got %s",
                luaFileName.c_str(),
                key.c_str(),
                get_type_name(key_array[1].get_type()));
        }

        SpriteValue sprite_value = {};
        sol::table  files = key_array["Files"];
        for (int i = 1; i <= files.size(); i++) {
            std::string file = files[i];
            sprite_value.Files.push_back(file);
        }

        if (key_array["Position"] == sol::nil || !key_array["Position"].is<sol::table>()) {
            throw Exceptions::EstException(
                "[Sprite] %s at key: '%s', Position field is not a table or nil but got %s",
                luaFileName.c_str(),
                key.c_str(),
                get_type_name(key_array["Position"].get_type()));
        }

        sol::table position = key_array["Position"];

        if (position[1] == sol::nil || !position[1].is<double>()) {
            throw Exceptions::EstException("[Sprite] %s at key: '%s', Position[1] is not a number or nil", luaFileName.c_str(), key.c_str());
        }

        if (position[2] == sol::nil || !position[2].is<double>()) {
            throw Exceptions::EstException("[Sprite] %s at key: '%s', Position[2] is not a number or nil", luaFileName.c_str(), key.c_str());
        }

        sprite_value.Position = UDim2::fromOffset(position[1], position[2]);

        if (key_array["Size"] == sol::nil || !key_array["Size"].is<sol::table>()) {
            throw Exceptions::EstException(
                "[Sprite] %s at key: '%s', Size field is not a table or nil but got %s",
                luaFileName.c_str(),
                key.c_str(),
                get_type_name(key_array["Size"].get_type()));
        }

        sol::table size = key_array["Size"];

        if (size[1] == sol::nil || !size[1].is<double>()) {
            throw Exceptions::EstException("[Sprite] %s at key: '%s', Size[1] is not a number or nil", luaFileName.c_str(), key.c_str());
        }

        if (size[2] == sol::nil || !size[2].is<double>()) {
            throw Exceptions::EstException("[Sprite] %s at key: '%s', Size[2] is not a number or nil", luaFileName.c_str(), key.c_str());
        }

        sprite_value.Size = UDim2::fromOffset(size[1], size[2]);

        sol::table anchorPoint = key_array["AnchorPoint"];
        sprite_value.AnchorPoint = Vector2(anchorPoint[1], anchorPoint[2]);

        sprite_value.FrameTime = key_array["FrameTime"];

        sol::table rgb = key_array["Color"];
        sprite_value.Color = Color3::fromRGB(
            rgb[1],
            rgb[2],
            rgb[3]);

        return sprite_value;
    } catch (const sol::error &err) {
        throw Exceptions::EstException("[Sprite] %s, (key = %s)", err.what(), key.c_str());
    }
}

TweenInfo LuaSkin::GetTween(std::string key)
{
    try {
        sol::table &script_table = *Script.table.get();
        sol::table  table = script_table["Data"];
        auto        luaFileName = ExpectedFiles[CurrentGroup];

        // check if the key exists
        if (table[SkinDataType::Tween][key] == sol::nil || !table[SkinDataType::Tween][key].is<sol::table>()) {
            throw Exceptions::EstException("[Tween] [%s] Key '%s' does not exist or not table", luaFileName.c_str(), key.c_str());
        }

        sol::table key_array = table[SkinDataType::Tween][key];

        TweenInfo tween_info = {};

        if (key_array["Destination"] == sol::nil || !key_array["Destination"].is<sol::table>()) {
            throw Exceptions::EstException(
                "[Tween] %s at key: '%s', Destination field is not a table or nil but got %s",
                luaFileName.c_str(),
                key.c_str(),
                get_type_name(key_array["Destination"].get_type()));
        }

        sol::table Destination = key_array["Destination"];

        if (Destination[1] == sol::nil || !Destination[1].is<double>()) {
            throw Exceptions::EstException("[Tween] %s at key: '%s', Destination[1] is not a number or nil", luaFileName.c_str(), key.c_str());
        }

        if (Destination[2] == sol::nil || !Destination[2].is<double>()) {
            throw Exceptions::EstException("[Tween] %s at key: '%s', Destination[2] is not a number or nil", luaFileName.c_str(), key.c_str());
        }

        double x = Destination[1];
        double y = Destination[2];

        tween_info.Destination = UDim2::fromOffset(x, y);

        if (key_array["Type"] == sol::nil || !key_array["Type"].is<int>()) {
            throw Exceptions::EstException(
                "[Tween] %s at key: '%s', Type field is not a number or nil but got %s",
                luaFileName.c_str(),
                key.c_str(),
                get_type_name(key_array["Type"].get_type()));
        }

        tween_info.Type = key_array["Type"];

        if (key_array["Duration"] == sol::nil || !key_array["Duration"].is<double>()) {
            throw Exceptions::EstException(
                "[Tween] %s at key: '%s', Duration field is not a number or nil but got %s",
                luaFileName.c_str(),
                key.c_str(),
                get_type_name(key_array["Duration"].get_type()));
        }

        tween_info.Duration = key_array["Duration"];

        return tween_info;
    } catch (const sol::error &err) {
        throw Exceptions::EstException("[Tween] %s, (key = %s)", err.what(), key.c_str());
    }
}

std::string LuaSkin::GetSkinProp(std::string group, std::string key, std::string defaultValue)
{
    return ini[group][key].empty() ? defaultValue : ini[group][key];
}

std::string LuaSkin::GetPath()
{
    return CurrentPath.string();
}

LuaSkin *LuaSkin::Get()
{
    static LuaSkin m_instance;

    return &m_instance;
}

LuaSkin::~LuaSkin()
{
    Script.state->collect_garbage();
}

int GameLua::GetArenaIndex()
{
    return Env::GetInt("CurrentArena");
}

int GameLua::GetHitPosition()
{
    std::string position = __skin->GetSkinProp("Game", "HitPos", "0");

    return std::stoi(position);
}

int GameLua::GetLaneOffset()
{
    std::string offset = __skin->GetSkinProp("Game", "LaneOffset", "0");

    return std::stoi(offset);
}

std::tuple<int, int> GameLua::GetResolution()
{
    auto rect = Graphics::NativeWindow::Get()->GetBufferSize();

    return {
        rect.Width,
        rect.Height
    };
}

int GameLua::GetKeyCount()
{
    return Env::GetInt("KeyCount");
}

std::string GameLua::GetSkinPath()
{
    return __skin->GetPath() + "/" + __group + "/";
}

std::string GameLua::GetScriptPath()
{
    return __skin->GetPath() + "/Scripts/";
}

bool GameLua::IsPathExist(std::string Path)
{
    return std::filesystem::exists(Path);
}