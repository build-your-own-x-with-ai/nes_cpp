#pragma once

#include <cstdint>

class AddrRegister {
public:
    AddrRegister();

    void update(uint8_t data);
    void increment(uint8_t inc);
    void reset_latch();
    uint16_t get() const;

private:
    uint8_t hi;
    uint8_t lo;
    bool hi_ptr;

    void set(uint16_t data);
};
