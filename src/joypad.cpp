#include "joypad.h"

Joypad::Joypad()
    : strobe_(false),
      button_index_(0),
      button_status_(0) {
}

void Joypad::write(uint8_t data) {
    strobe_ = (data & 1) == 1;
    if (strobe_) {
        button_index_ = 0;
    }
}

uint8_t Joypad::read() {
    if (button_index_ > 7) {
        return 1;
    }
    uint8_t response = (button_status_ & (1 << button_index_)) >> button_index_;
    if (!strobe_ && button_index_ <= 7) {
        button_index_++;
    }
    return response;
}

void Joypad::set_button_pressed(JoypadButton button, bool pressed) {
    uint8_t bit = static_cast<uint8_t>(button);
    if (pressed) {
        button_status_ |= bit;
    } else {
        button_status_ &= ~bit;
    }
}
