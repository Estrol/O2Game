cmake_minimum_required(VERSION 3.0.0)
project(Game VERSION 0.1.0 LANGUAGES C CXX)

add_executable(Game 
    # MAIN
	"src/main.cpp"
    "src/MyGame.cpp"
    "src/EnvironmentSetup.cpp"

	# Data
    "src/Data/AutoReplay.cpp"
    "src/Data/bms.cpp"
    "src/Data/Chart.cpp"
    "src/Data/MusicDatabase.cpp"
    "src/Data/OJM.cpp"
    "src/Data/OJN.cpp"
    "src/Data/osu.cpp"
    "src/Data/Util/md5.cpp"
    "src/Data/Util/Util.cpp"

    # Engine
	"src/Engine/BGMPreview.cpp"
    "src/Engine/Button.cpp"
    "src/Engine/DrawableNote.cpp"
    "src/Engine/DrawableTile.cpp"
    "src/Engine/FrameTimer.cpp"
    "src/Engine/GameAudioSampleCache.cpp"
    "src/Engine/GameTrack.cpp"
    "src/Engine/Lodepng.cpp"
    "src/Engine/Note.cpp"
    "src/Engine/NoteImageCacheManager.cpp"
    "src/Engine/O2NumericTexture.cpp"
    "src/Engine/O2Texture.cpp"
    "src/Engine/RhythmEngine.cpp"
    "src/Engine/ScoreManager.cpp"
    "src/Engine/TimingLine.cpp"
    "src/Engine/TimingLineManager.cpp"
	"src/Engine/Timing/StaticTiming.cpp"
	"src/Engine/Timing/TimingBase.cpp"
    "src/Engine/Timing/VelocityTiming.cpp"
    "src/Engine/Judgements/BeatBasedJudge.cpp"
    "src/Engine/Judgements/JudgeBase.cpp"
    "src/Engine/Judgements/MsBasedJudge.cpp"

	# Resources
	"src/Resources/GameResources.cpp"
    "src/Resources/SkinConfig.cpp"

    # Scenes
    "src/Scenes/Converters/ToOsu.cpp"
	"src/Scenes/EditorScene.cpp"
    "src/Scenes/GameplayScene.cpp"
    "src/Scenes/IntroScene.cpp"
    "src/Scenes/LoadingScene.cpp"
    "src/Scenes/ResultScene.cpp"
    "src/Scenes/SongSelectScene.cpp"
    "src/Scenes/MainMenu.cpp"

    # Overlays
    "src/Scenes/Overlays/Settings.cpp"
)

#include "../Engine/include"
target_include_directories(Game PRIVATE "../Engine/include")

if (WIN32)
	add_custom_command(TARGET Game POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/third-party/bin/x64-windows/Debug
        $<TARGET_FILE_DIR:Game>)

    # Create directory $<TARGET_FILE_DIR:Game>/Skins/Default then
    # copy from ${CMAKE_SOURCE_DIR}/GameResources/Resources to $<TARGET_FILE_DIR:Game>/Skins/Default

    add_custom_command(TARGET Game POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
        $<TARGET_FILE_DIR:Game>/Skins/Default
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/GameResources/Resources
        $<TARGET_FILE_DIR:Game>/Skins/Default)
else ()
    # add_custom_command(TARGET Game POST_BUILD
	# 	COMMAND ${CMAKE_COMMAND} -E copy_directory
	# 	${CMAKE_SOURCE_DIR}/third-party/bin/x64-linux/Debug
	# 	$<TARGET_FILE_DIR:Game>)
endif ()

if (MSVC)
    target_link_options(Game PRIVATE $<$<CONFIG:Debug>:/INCREMENTAL>)
    target_compile_options(Game PRIVATE $<$<CONFIG:Debug>:/ZI> "/MP")
endif()

# set preprocessor MEM_LEAK_DEBUG
target_compile_definitions(Game PRIVATE MEM_LEAK_DEBUG=1 _CRT_SECURE_NO_WARNINGS)

target_link_libraries(Game PRIVATE EstEngine ${O2GAME_LIBRARIES})