#include "status.h"

void StatusRegister::set_vblank_status(bool status) {
    if (status) {
        bits |= VBLANK_STARTED;
    } else {
        bits &= ~VBLANK_STARTED;
    }
}

void StatusRegister::set_sprite_zero_hit(bool status) {
    if (status) {
        bits |= SPRITE_ZERO_HIT;
    } else {
        bits &= ~SPRITE_ZERO_HIT;
    }
}

void StatusRegister::set_sprite_overflow(bool status) {
    if (status) {
        bits |= SPRITE_OVERFLOW;
    } else {
        bits &= ~SPRITE_OVERFLOW;
    }
}

void StatusRegister::reset_vblank_status() {
    bits &= ~VBLANK_STARTED;
}

bool StatusRegister::is_in_vblank() const {
    return (bits & VBLANK_STARTED) != 0;
}

uint8_t StatusRegister::snapshot() const {
    return bits;
}
