# File: `saiboVolleyball/CMakeLists.txt` (子目录)
cmake_minimum_required(VERSION 3.10)
project(saiboVolleyball)

set(CMAKE_CXX_STANDARD 20)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-finput-charset=utf-8 -fexec-charset=utf-8)
    add_definitions(-DUNICODE -D_UNICODE)
endif()

# 源文件（相对本子目录）
set(SOURCES
        main.cpp
        player.cpp
        game.cpp
        serve.cpp
        receiveServe.cpp
        setBall.cpp
        spike.cpp
        block.cpp
        defense.cpp
        supportCal.cpp
        mentalCalculation.cpp
        gameDisplay.cpp
)

# 避免 Windows 弹出控制台窗口（使用 GUI 子系统）
if (WIN32)
    add_executable(VolleyballSimulation WIN32 ${SOURCES})
else()
    add_executable(VolleyballSimulation ${SOURCES})
endif()

# MinGW 下显式去除控制台窗口
if (MINGW)
    target_link_options(VolleyballSimulation PRIVATE -mwindows)
endif()

target_include_directories(VolleyballSimulation PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# 链接 SDL2：优先使用 CONFIG targets（由根目录 find_package 找到），否则使用回退的库变量
if (TARGET SDL2::SDL2 AND TARGET SDL2_ttf::SDL2_ttf)
    target_link_libraries(VolleyballSimulation PRIVATE SDL2::SDL2main SDL2::SDL2 SDL2_ttf::SDL2_ttf)
else()

    # 回退到手动找到的库变量（确保 SDL2MAIN_LIBRARY 已被 find_library 查到）
    target_link_libraries(VolleyballSimulation PRIVATE
            ${SDL2MAIN_LIBRARY}
            ${SDL2_LIBRARY}
            ${SDL2_TTF_LIBRARY}
    )
endif()

# 复制 SDL2 DLL 文件到输出目录
add_custom_command(TARGET VolleyballSimulation POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${SDL2_PREFIX}/bin/SDL2.dll"
    $<TARGET_FILE_DIR:VolleyballSimulation>/SDL2.dll
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${SDL2_PREFIX}/bin/SDL2_ttf.dll"
    $<TARGET_FILE_DIR:VolleyballSimulation>/SDL2_ttf.dll
    COMMENT "复制 SDL2 DLL 文件到输出目录"
)
