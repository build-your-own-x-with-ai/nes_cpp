cmake_minimum_required(VERSION 3.10)
project(nes_emulator CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra)

include_directories(${CMAKE_SOURCE_DIR}/src)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(nes_emulator
    src/main.cpp
    src/cartridge.cpp
    src/bus.cpp
    src/cpu.cpp
    src/opcodes.cpp
    src/trace.cpp
    src/joypad.cpp
    src/ppu/ppu.cpp
    src/ppu/registers/control.cpp
    src/ppu/registers/mask.cpp
    src/ppu/registers/status.cpp
    src/ppu/registers/scroll.cpp
    src/ppu/registers/addr.cpp
    src/render/frame.cpp
    src/render/palette.cpp
    src/render/render.cpp
)

target_link_libraries(nes_emulator ${SDL2_LIBRARIES})
