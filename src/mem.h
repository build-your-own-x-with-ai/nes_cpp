#pragma once

#include <cstdint>

class Mem {
public:
    virtual ~Mem() = default;
    virtual uint8_t mem_read(uint16_t addr) const = 0;
    virtual void mem_write(uint16_t addr, uint8_t data) = 0;
};
