#pragma once

#include <cstdint>

class ScrollRegister {
public:
    uint8_t scroll_x;
    uint8_t scroll_y;
    bool latch;

    ScrollRegister();

    void write(uint8_t data);
    void reset_latch();
};
