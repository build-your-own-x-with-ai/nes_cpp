#include "addr.h"

AddrRegister::AddrRegister() : hi(0), lo(0), hi_ptr(true) {}

void AddrRegister::set(uint16_t data) {
    hi = (data >> 8) & 0xFF;
    lo = data & 0xFF;
}

void AddrRegister::update(uint8_t data) {
    if (hi_ptr) {
        hi = data;
    } else {
        lo = data;
    }

    if (get() > 0x3fff) {
        set(get() & 0b11111111111111);
    }

    hi_ptr = !hi_ptr;
}

void AddrRegister::increment(uint8_t inc) {
    uint8_t old_lo = lo;
    lo = lo + inc;
    if (old_lo > lo) {
        hi = hi + 1;
    }
    if (get() > 0x3fff) {
        set(get() & 0b11111111111111);
    }
}

void AddrRegister::reset_latch() {
    hi_ptr = true;
}

uint16_t AddrRegister::get() const {
    return (static_cast<uint16_t>(hi) << 8) | static_cast<uint16_t>(lo);
}
