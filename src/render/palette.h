#pragma once

#include <cstdint>
#include <array>

class Palette {
public:
    static const std::array<std::array<uint8_t, 3>, 64> SYSTEM_PALETTE;
};
