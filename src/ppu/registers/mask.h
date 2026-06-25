#pragma once

#include <cstdint>
#include <vector>

enum class Color {
    Red,
    Green,
    Blue,
};

class MaskRegister {
public:
    static constexpr uint8_t GREYSCALE = 0b00000001;
    static constexpr uint8_t LEFTMOST_8PXL_BACKGROUND = 0b00000010;
    static constexpr uint8_t LEFTMOST_8PXL_SPRITE = 0b00000100;
    static constexpr uint8_t SHOW_BACKGROUND = 0b00001000;
    static constexpr uint8_t SHOW_SPRITES = 0b00010000;
    static constexpr uint8_t EMPHASISE_RED = 0b00100000;
    static constexpr uint8_t EMPHASISE_GREEN = 0b01000000;
    static constexpr uint8_t EMPHASISE_BLUE = 0b10000000;

    MaskRegister() : bits(0) {}

    bool is_grayscale() const;
    bool leftmost_8pxl_background() const;
    bool leftmost_8pxl_sprite() const;
    bool show_background() const;
    bool show_sprites() const;
    std::vector<Color> emphasise() const;
    void update(uint8_t data);
    uint8_t snapshot() const { return bits; }

private:
    uint8_t bits;
};
