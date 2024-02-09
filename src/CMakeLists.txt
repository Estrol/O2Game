cmake_minimum_required(VERSION 3.8.0)

file(GLOB_RECURSE HEADERS "./*.h" "./*.hpp")

add_executable(o2game ${HEADERS}
    "main.cpp"

    # Game
    "Game/O2Game.cpp"
    "Game/Env.cpp"
    "Game/MsgBoxEx.cpp"

    # Scenes
    "Scenes/Gameplay.cpp" 
    "Scenes/Loading.cpp"
    "Scenes/SongSelect.cpp"
    "Scenes/Result.cpp"

    # Core
    "Game/Core/RhythmEngine.cpp"
    "Game/Core/Resources/NoteImages.cpp"
    "Game/Core/Replay/Autoplay.cpp"
    "Game/Core/Notes/Note.cpp"
    "Game/Core/Notes/NoteCacheManager.cpp"
    "Game/Core/Notes/TimingLine.cpp"
    "Game/Core/Notes/TimingLineManager.cpp"
    "Game/Core/Notes/Track.cpp"
    "Game/Core/Judgements/JudgeBase.cpp"
    "Game/Core/Judgements/BeatBasedJudge.cpp"
    "Game/Core/Judgements/MsBasedJudge.cpp"
    "Game/Core/Drawable/Sprite.cpp"
    "Game/Core/Drawable/Image.cpp" 
    "Game/Core/Drawable/ImageGenerators.cpp" 
    "Game/Core/Drawable/NumberSprite.cpp" 
    "Game/Core/Drawable/ButtonImage.cpp"
    "Game/Core/Audio/SampleManager.cpp"
    "Game/Core/Scoring/ScoreManager.cpp"
    "Game/Core/Timings/StaticTiming.cpp"
    "Game/Core/Timings/TimingBase.cpp"
    "Game/Core/Timings/VelocityTiming.cpp"
    "Game/Core/Skinning/LuaSkin.cpp"
    # "Game/Core/Skinning/LegacySkin.cpp" 
    # "Game/Core/Skinning/LuaScripting.cpp" 
    # "Game/Core/Skinning/SkinManager.cpp" 
    "Game/Core/Database/GameDatabase.cpp"

    # Data
    "Game/Data/chart.cpp"
    "Game/Data/bms.cpp"
    "Game/Data/ojn.cpp"
    "Game/Data/ojm.cpp"
    "Game/Data/osu.cpp"
    "Game/Data/Util/Util.cpp"
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
    set_property(TARGET o2game PROPERTY CXX_STANDARD 20)
endif()

if (WIN32)
    add_custom_command(TARGET o2game POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
        $<TARGET_FILE_DIR:o2game>/Skins/Default
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources/Resources
        $<TARGET_FILE_DIR:o2game>/Skins/Default)

    add_custom_command(TARGET o2game POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${EST_AUDIO_FILES}
        $<TARGET_FILE_DIR:o2game>
    )
else ()
    add_custom_command(TARGET o2game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
    $<TARGET_FILE_DIR:o2game>/Skins/Default
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/resources/Resources
    $<TARGET_FILE_DIR:o2game>/Skins/Default)
endif ()

target_include_directories(o2game PRIVATE "../lib/include")
target_link_libraries(o2game PRIVATE 
    EstEngine 
    ${EST_AUDIO_LIBRARIES}
    spirv-cross-core 
    spirv-cross-glsl 
    SDL2::SDL2 
    SDL2_image::SDL2_image 
    Iconv::Iconv
    sol2
    lua
    lodepng
    Freetype::Freetype 
    unofficial::sqlite3::sqlite3
)