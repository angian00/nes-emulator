cmake_minimum_required(VERSION 3.5.1)

project(angian-nes-emu)


set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)
add_compile_definitions(_CRT_SECURE_NO_WARNINGS)


# Add SDL2 CMake modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/sdl2)

#file(GLOB MAIN_SOURCES "src/*.cpp")

set(MAIN_SOURCES
    src/bus.cpp
    src/cartridge.cpp
    src/instructions.cpp
    src/cpu.cpp
    src/cpu_opcodes.cpp
    src/cpu_addr_modes.cpp
    src/ppu.cpp
    src/bit_operations.cpp
    src/display.cpp
    src/keyboard.cpp
    src/emulator.cpp
)

add_executable(${PROJECT_NAME} ${MAIN_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE include)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)


set(VIEWER_EXE chr-viewer)
set(VIEWER_SOURCES
    src/cartridge.cpp
    src/sprite_viewer.cpp
)
add_executable(${VIEWER_EXE} ${VIEWER_SOURCES})
target_include_directories(${VIEWER_EXE} PRIVATE include)
set_property(TARGET ${VIEWER_EXE} PROPERTY CXX_STANDARD 23)



set(TEST_UTILS_EXE test_utils)
set(TEST_UTILS_SOURCES
    src/bit_operations.cpp
    src/test_utils.cpp
)
add_executable(${TEST_UTILS_EXE} ${TEST_UTILS_SOURCES})
target_include_directories(${TEST_UTILS_EXE} PRIVATE include)
set_property(TARGET ${TEST_UTILS_EXE} PROPERTY CXX_STANDARD 23)


# Add compiler errors/warnings flags
#target_compile_options(${PROJECT_NAME} PRIVATE $<$<C_COMPILER_ID:MSVC>:/W4 /WX>)
#target_compile_options(${PROJECT_NAME} PRIVATE $<$<NOT:$<C_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic -Werror>)


# Add SDL2 library
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME}  ${SDL2_LIBRARIES})
target_link_libraries(${VIEWER_EXE}  ${SDL2_LIBRARIES})

add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_SOURCE_DIR}/2C02G_wiki.pal
                ${CMAKE_CURRENT_BINARY_DIR}/2C02G_wiki.pal)

IF (WIN32)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$ENV{USERPROFILE}/tools/sdl2/SDL2.dll"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>)
ENDIF()

# Add SDL2_image library
#find_package(SDL2_image REQUIRED)
#target_link_libraries(${PROJECT_NAME} SDL2::Image)

# Add SDL2_ttf library
#find_package(SDL2_ttf REQUIRED)
#target_link_libraries(${PROJECT_NAME} SDL2::TTF)

# Add SDL2_net library
#find_package(SDL2_net REQUIRED)
#target_link_libraries(${PROJECT_NAME} SDL2::Net)

# Add SDL2_mixer library
#find_package(SDL2_mixer REQUIRED)
#target_link_libraries(${PROJECT_NAME} SDL2::Mixer)

# Add SDL2_gfx library
#find_package(SDL2_gfx REQUIRED)
#target_link_libraries(${PROJECT_NAME} SDL2::GFX)
