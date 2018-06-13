cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(arkanoid2k18)

set(APP_NAME arkanoid)
set(INSTALL_DIR /usr/bin)
set(GAME_ASSETS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/assets)
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
find_package(OpenAL REQUIRED)
find_package(glm REQUIRED)

configure_file("${SHARED_INCLUDE_PATH}/config.h.in" "${SHARED_INCLUDE_PATH}/config.hh")

add_executable(${APP_NAME} ${SOURCES} ${HEADERS})

#set_target_properties(${APP_NAME} PROPERTIES
#    CXX_STANDARD 17
#    CXX_STANDARD_REQUIRED YES
#    CXX_EXTENSIONS NO
#)

target_include_directories(${APP_NAME} PUBLIC
    ${SHARED_INCLUDE_PATH}
    ${SDL2_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIRS}
    ${OPENAL_INCLUDE_DIR}
    external
)

target_compile_definitions(${APP_NAME} PUBLIC
    USING_SDL
)

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
    ${SDL2_LIBRARIES}
    ${OPENAL_LIBRARY}
)

install(TARGETS ${APP_NAME} RUNTIME DESTINATION ${INSTALL_DIR})
