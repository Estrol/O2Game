#include "SkinManager.hpp"

SkinManager* SkinManager::m_instance = nullptr;

void SkinManager::LoadSkin(std::string skinName) {
    if (m_currentSkin != skinName) {
        if (m_luaScripting) {
            m_luaScripting.reset();
        }

        m_currentSkin = skinName;
    }

    auto skinPath = std::filesystem::current_path() / "Skins";
    auto selectedSkin = skinPath / skinName; 

    mINI::INIFile f(selectedSkin / "GameSkin.ini");
    ini = {};
    f.read(ini);

    m_expected_directory = {
        { SkinGroup::Playing, "Playing" },
        { SkinGroup::MainMenu, "MainMenu" },
        { SkinGroup::Notes, "Notes" },
        { SkinGroup::SongSelect, "SongSelect" },
    };

    m_expected_skin_config = {
        { SkinGroup::Playing, "Playing.ini" },
        { SkinGroup::MainMenu, "MainMenu.ini" },
        { SkinGroup::Notes, "Notes.ini" },
        { SkinGroup::SongSelect, "SongSelect.ini" },
    };

    m_useLua = false;
    try {
        auto value = ini["Game"]["UseLua"];

        m_useLua = std::stoi(value) == 1;
    } catch (const std::invalid_argument&) {
        std::cout << "Invalid argument for UseLua" << std::endl;
    }

    if (m_useLua) {
        m_luaScripting = std::make_unique<LuaScripting>(selectedSkin / "Scripts");
    }
}

void SkinManager::ReloadSkin() {
    m_skinConfigs.clear();

    LoadSkin(m_currentSkin);
}

LaneInfo SkinManager::GetLaneInfo() {
    LaneInfo info = {};

    try {
        info.HitPosition = std::stoi(ini["Game"]["HitPos"]);
        info.LaneOffset = std::stoi(ini["Game"]["LaneOffset"]);
    } catch (const std::invalid_argument&) {
        std::cout << "Invalid argument for HitPos or LaneOffset" << std::endl;
    }

    return info;
}

std::string SkinManager::GetSkinProp(std::string group, std::string key, std::string defaultValue) {
    auto value = ini[group][key];

    return value.size() ? value : defaultValue;
}

std::filesystem::path SkinManager::GetPath() {
    return std::filesystem::current_path() / "Skins" / m_currentSkin;
}

void SkinManager::SetKeyCount(int key) {
    m_keyCount = key;
}

std::vector<NumericValue> SkinManager::GetNumeric(SkinGroup group, std::string key) {
    if (m_luaScripting) {
        return m_luaScripting->GetNumeric(group, key);
    } else {
        if (m_skinConfigs.find(group) == m_skinConfigs.end()) {
            TryLoadGroup(group);
        }

        return m_skinConfigs[group]->GetNumeric(key);
    }
}

std::vector<PositionValue> SkinManager::GetPosition(SkinGroup group, std::string key) {
    if (m_luaScripting) {
        return m_luaScripting->GetPosition(group, key, m_keyCount);
    } else {
        if (m_skinConfigs.find(group) == m_skinConfigs.end()) {
            TryLoadGroup(group);
        }

        return m_skinConfigs[group]->GetPosition(key);
    }
}

std::vector<RectInfo> SkinManager::GetRect(SkinGroup group, std::string key) {
    if (m_luaScripting) {
        return m_luaScripting->GetRect(group, key);
    } else {
        if (m_skinConfigs.find(group) == m_skinConfigs.end()) {
            TryLoadGroup(group);
        }

        return m_skinConfigs[group]->GetRect(key);
    }
}

NoteValue SkinManager::GetNote(SkinGroup group, std::string key) {
    if (m_luaScripting) {
        return m_luaScripting->GetNote(group, key, m_keyCount);
    } else {
        if (m_skinConfigs.find(group) == m_skinConfigs.end()) {
            TryLoadGroup(group);
        }

        return m_skinConfigs[group]->GetNote(key);
    }
}

SpriteValue SkinManager::GetSprite(SkinGroup group, std::string key) {
    if (m_luaScripting) {
        return m_luaScripting->GetSprite(group, key);
    } else {
        if (m_skinConfigs.find(group) == m_skinConfigs.end()) {
            TryLoadGroup(group);
        }

        return m_skinConfigs[group]->GetSprite(key);
    }
}

void SkinManager::Arena_SetIndex(int index) {
    m_arena = index;

    if (m_luaScripting) {
        m_luaScripting->Arena_SetIndex(index);
    } else {
        m_arenaConfig = std::make_unique<SkinConfig>(
            GetPath() 
            / m_expected_directory[SkinGroup::Playing] 
            / "Arena"
            / std::to_string(index) 
            / "Arena.ini", 
            m_keyCount
        );
    }
}

std::vector<NumericValue> SkinManager::Arena_GetNumeric(std::string key) {
    if (m_luaScripting) {
        return m_luaScripting->Arena_GetNumeric(key);
    } else {
        return m_arenaConfig->GetNumeric(key);
    }
}

std::vector<PositionValue> SkinManager::Arena_GetPosition(std::string key) {
    if (m_luaScripting) {
        return m_luaScripting->Arena_GetPosition(key, m_keyCount);
    } else {
        return m_arenaConfig->GetPosition(key);
    }
}

std::vector<RectInfo> SkinManager::Arena_GetRect(std::string key) {
    if (m_luaScripting) {
        return m_luaScripting->Arena_GetRect(key);
    } else {
        return m_arenaConfig->GetRect(key);
    }
}

SpriteValue SkinManager::Arena_GetSprite(std::string key) {
    if (m_luaScripting) {
        return m_luaScripting->Arena_GetSprite(key);
    } else {
        return m_arenaConfig->GetSprite(key);
    }
}

SkinManager *SkinManager::GetInstance() {
    if (!m_instance) {
        m_instance = new SkinManager;
    }

    return m_instance;
}

void SkinManager::Release() {
    if (m_instance) {
        delete m_instance;
    }
}

SkinManager::SkinManager() {

}

SkinManager::~SkinManager() {

}

void SkinManager::TryLoadGroup(SkinGroup group) {
    m_skinConfigs[group] = std::make_unique<SkinConfig>(
        GetPath() 
        / m_expected_directory[group] 
        / m_expected_skin_config[group]
        , m_keyCount
    );
}

void SkinManager::Update(double delta) {
    if (m_luaScripting) {
        m_luaScripting->Update(delta);
    }
}
