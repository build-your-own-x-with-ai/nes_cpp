#include "control.h"

uint16_t ControlRegister::nametable_addr() const {
    switch (bits & 0b11) {
        case 0: return 0x2000;
        case 1: return 0x2400;
        case 2: return 0x2800;
        case 3: return 0x2c00;
        default: return 0x2000;
    }
}

uint8_t ControlRegister::vram_addr_increment() const {
    return (bits & VRAM_ADD_INCREMENT) ? 32 : 1;
}

uint16_t ControlRegister::sprt_pattern_addr() const {
    return (bits & SPRITE_PATTERN_ADDR) ? 0x1000 : 0;
}

uint16_t ControlRegister::bknd_pattern_addr() const {
    return (bits & BACKROUND_PATTERN_ADDR) ? 0x1000 : 0;
}

uint8_t ControlRegister::sprite_size() const {
    return (bits & SPRITE_SIZE) ? 16 : 8;
}

uint8_t ControlRegister::master_slave_select() const {
    return (bits & MASTER_SLAVE_SELECT) ? 1 : 0;
}

bool ControlRegister::generate_vblank_nmi() const {
    return (bits & GENERATE_NMI) != 0;
}

void ControlRegister::update(uint8_t data) {
    bits = data;
}
