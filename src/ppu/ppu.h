#pragma once

#include <cstdint>
#include <array>
#include <vector>
#include "cartridge.h"
#include "registers/control.h"
#include "registers/mask.h"
#include "registers/status.h"
#include "registers/scroll.h"
#include "registers/addr.h"

class Frame;

class NesPPU {
public:
    std::vector<uint8_t> chr_rom;
    Mirroring mirroring;
    ControlRegister ctrl;
    MaskRegister mask;
    StatusRegister status;
    ScrollRegister scroll;
    AddrRegister addr;
    std::array<uint8_t, 2048> vram;

    uint8_t oam_addr;
    std::array<uint8_t, 256> oam_data;
    std::array<uint8_t, 32> palette_table;

    uint16_t scanline;
    bool nmi_interrupt;

    NesPPU();
    explicit NesPPU(const std::vector<uint8_t>& chr_rom, Mirroring mirroring);

    uint16_t mirror_vram_addr(uint16_t addr) const;

    bool tick(uint8_t cycles);
    bool poll_nmi_interrupt();

    void write_to_ctrl(uint8_t value);
    void write_to_mask(uint8_t value);
    uint8_t read_status();
    void write_to_oam_addr(uint8_t value);
    void write_to_oam_data(uint8_t value);
    uint8_t read_oam_data() const;
    void write_to_scroll(uint8_t value);
    void write_to_ppu_addr(uint8_t value);
    void write_to_data(uint8_t value);
    uint8_t read_data();
    void write_oam_dma(const uint8_t* data);

private:
    uint8_t internal_data_buf;
    size_t cycles_;

    void increment_vram_addr();
    bool is_sprite_0_hit(size_t cycle) const;
};
