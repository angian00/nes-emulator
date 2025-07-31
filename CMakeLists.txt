project(angian-nes-emu.x)

cmake_minimum_required(VERSION 3.5.1)

set(CMAKE_BUILD_TYPE Debug)

# Add SDL2 CMake modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/sdl2)

file(GLOB SOURCES "src/*.cpp")
add_executable(${PROJECT_NAME} ${SOURCES})
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)

target_include_directories(${PROJECT_NAME} PRIVATE include)

# Add compiler errors/warnings flags
#target_compile_options(${PROJECT_NAME} PRIVATE $<$<C_COMPILER_ID:MSVC>:/W4 /WX>)
#target_compile_options(${PROJECT_NAME} PRIVATE $<$<NOT:$<C_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic -Werror>)


# Add SDL2 library
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME}  ${SDL2_LIBRARIES})

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
