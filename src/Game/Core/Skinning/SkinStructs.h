/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#pragma once
#include <Math/Color3.h>
#include <Math/Tween.h>
#include <Math/Vector2.h>
#include <filesystem>
#include <glm/glm.hpp>
#include <map>
#include <string>
#include <vector>

enum class SkinGroup {
    Main,
    MainMenu,
    Notes,
    Playing,
    Arena,
    SongSelect,
    Result,
    Audio
};

enum class SkinDataType {
    Numeric,
    Position,
    Rect,
    Note,
    Sprite,
    Tween,
    Audio
};

enum class SkinNumericDirection {
    Left = -1,
    Mid = 0,
    Right = 1
};

struct LaneInfo
{
    int HitPosition;
    int LaneOffset;
};

// Value Format: X, Y, AnchorPointX?, AnchorPointY?, TintColor?
struct PositionValue
{
    std::filesystem::path  Path;
    std::vector<glm::vec2> TexCoord;
    UDim2                  Position;
    UDim2                  Size;
    Vector2                AnchorPoint;
    Color3                 Color;
};

// Value format: X, Y, MaxDigits?, Direction?, FillWithZero?
struct NumericValue
{
    std::filesystem::path               Path;
    std::vector<std::vector<glm::vec2>> TexCoords;
    UDim2                               Position;
    UDim2                               Size;
    int                                 MaxDigit;
    int                                 Direction;
    bool                                FillWithZero;
    Color3                              Color;
};

// Value format: X, Y, AnchorPointX, AnchorPointY
struct SpriteValue
{
    std::filesystem::path               Path;
    std::vector<std::vector<glm::vec2>> TexCoords;
    UDim2                               Position;
    UDim2                               Size;
    Vector2                             AnchorPoint;
    float                               FrameTime;
    Color3                              Color;
};

// Value format: NumOfFiles, FileName
struct NoteValue
{
    std::vector<std::filesystem::path> Files;
    UDim2                              Size;
    float                              FrameTime;
    Color3                             Color;
};

struct RectInfo
{
    UDim2 Position;
    UDim2 Size;
};

struct TweenInfo
{
    UDim2     Destination;
    TweenType Type;
    float     Duration;
};

enum class SkinAudioType {
    BGM_Lobby,
    BGM_Waiting,
    BGM_Result
};

static std::map<SkinAudioType, std::string> AudioTypeString = {
    { SkinAudioType::BGM_Lobby, "BGM_Lobby" },
    { SkinAudioType::BGM_Waiting, "BGM_Waiting" },
    { SkinAudioType::BGM_Result, "BGM_Result" }
};

struct AudioInfo
{
    std::filesystem::path Path;
    SkinAudioType         Type;
};