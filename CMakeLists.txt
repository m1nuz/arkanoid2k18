cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(arkanoid2k18)

option(OPENAL_BACKEND "Build with OpenAL" OFF)
option(SDL_MIXER_BACKEND "Build with SDL Audio" ON)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include(glm)
include(assets)

set(APP_NAME arkanoid)
set(INSTALL_DIR /usr/bin)
set(GAME_ASSETS_DIR ${CMAKE_CURRENT_BINARY_DIR}/assets)
set(GAME_ASSETS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src/assets.json)
set(GAME_CONF_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src/game.conf)
set(GAME_LEVELS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src/levels.json)
set(GAME_TITLE "Arkanoid 2k18")

set(SHARED_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SOURCES
    src/game.cc
    src/level.cc
    src/resources.cc
    src/video.cc
    src/audio.cc
    src/collisions.cc
    src/particle_emitter.cc
    src/targa.cc
    src/wave.cc
    src/glcore.c
    src/main.cc)

find_package(SDL2 REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(OpenAL REQUIRED)

configure_file("${SHARED_INCLUDE_PATH}/config.h.in" "${SHARED_INCLUDE_PATH}/config.hh")

set(APP_INCLUDES ${SDL2_INCLUDE_DIRS} ${GLM_INCLUDE_DIRS})
set(APP_LIBRARIES ${SDL2_LIBRARY})
set(APP_DEFINES USING_SDL)

if(OPENAL_BACKEND)
    list(APPEND APP_DEFINES OPENAL_BACKEND)
    list(APPEND APP_LIBRARIES ${OPENAL_LIBRARY})
    list(APPEND APP_INCLUDES ${OPENAL_INCLUDE_DIR})
elseif(SDL_MIXER_BACKEND)
    list(APPEND APP_DEFINES SDL_MIXER_BACKEND)
    list(APPEND APP_LIBRARIES ${SDL2MIXER_LIBRARY})
    list(APPEND APP_INCLUDES ${SDL2MIXER_INCLUDE_DIR})
endif()

add_executable(${APP_NAME} ${SOURCES} ${HEADERS})

add_dependencies(${APP_NAME} demo_assets)

#set_target_properties(${APP_NAME} PROPERTIES
#    CXX_STANDARD 17
#    CXX_STANDARD_REQUIRED YES
#    CXX_EXTENSIONS NO
#)

target_include_directories(${APP_NAME} PUBLIC
    ${SHARED_INCLUDE_PATH}
    ${APP_INCLUDES}
    external
)

target_compile_definitions(${APP_NAME} PUBLIC ${APP_DEFINES})

if (UNIX)
    target_compile_options(${APP_NAME} PUBLIC
        $<$<COMPILE_LANGUAGE:CXX>:-std=c++17>
        -pthread
        -pedantic
        -Wall
        -Wextra
        -Wshadow
        -Wpointer-arith
        -Wcast-qual
        -Wunused-result
        -g
        )
elseif (MSVC)
    target_compile_options(${APP_NAME} PUBLIC
        /W3
#        /WX
        )
endif()

target_link_libraries(${APP_NAME} PUBLIC
    ${APP_LIBRARIES}
)

install(TARGETS ${APP_NAME} RUNTIME DESTINATION ${INSTALL_DIR})
