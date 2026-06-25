#include "mask.h"

bool MaskRegister::is_grayscale() const {
    return (bits & GREYSCALE) != 0;
}

bool MaskRegister::leftmost_8pxl_background() const {
    return (bits & LEFTMOST_8PXL_BACKGROUND) != 0;
}

bool MaskRegister::leftmost_8pxl_sprite() const {
    return (bits & LEFTMOST_8PXL_SPRITE) != 0;
}

bool MaskRegister::show_background() const {
    return (bits & SHOW_BACKGROUND) != 0;
}

bool MaskRegister::show_sprites() const {
    return (bits & SHOW_SPRITES) != 0;
}

std::vector<Color> MaskRegister::emphasise() const {
    std::vector<Color> result;
    if (bits & EMPHASISE_RED) {
        result.push_back(Color::Red);
    }
    if (bits & EMPHASISE_BLUE) {
        result.push_back(Color::Blue);
    }
    if (bits & EMPHASISE_GREEN) {
        result.push_back(Color::Green);
    }
    return result;
}

void MaskRegister::update(uint8_t data) {
    bits = data;
}
