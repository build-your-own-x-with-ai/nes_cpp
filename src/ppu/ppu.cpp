#include "ppu.h"
#include <iostream>
#include "render/palette.h"

NesPPU::NesPPU() 
    : chr_rom(2048, 0), mirroring(Mirroring::Horizontal),
      oam_addr(0), scanline(0), nmi_interrupt(false),
      internal_data_buf(0), cycles_(0) {
    vram.fill(0);
    oam_data.fill(0);
    palette_table.fill(0);
}

NesPPU::NesPPU(const std::vector<uint8_t>& chr_rom, Mirroring mirroring)
    : chr_rom(chr_rom), mirroring(mirroring),
      oam_addr(0), scanline(0), nmi_interrupt(false),
      internal_data_buf(0), cycles_(0) {
    vram.fill(0);
    oam_data.fill(0);
    palette_table.fill(0);
}

uint16_t NesPPU::mirror_vram_addr(uint16_t addr) const {
    uint16_t mirrored_vram = addr & 0b10111111111111;
    uint16_t vram_index = mirrored_vram - 0x2000;
    uint16_t name_table = vram_index / 0x400;
    
    switch (mirroring) {
        case Mirroring::Vertical:
            if (name_table == 2 || name_table == 3) {
                return vram_index - 0x800;
            }
            break;
        case Mirroring::Horizontal:
            if (name_table == 2 || name_table == 1) {
                return vram_index - 0x400;
            }
            if (name_table == 3) {
                return vram_index - 0x800;
            }
            break;
        case Mirroring::FourScreen:
            break;
    }
    return vram_index;
}

bool NesPPU::tick(uint8_t cycles) {
    cycles_ += cycles;

    if (cycles_ >= 341) {
        if (is_sprite_0_hit(cycles_)) {
            status.set_sprite_zero_hit(true);
        }

        cycles_ -= 341;
        scanline += 1;

        if (scanline == 241) {
            status.set_vblank_status(true);
            status.set_sprite_zero_hit(false);

            if (ctrl.generate_vblank_nmi()) {
                nmi_interrupt = true;
            }
        }

        if (scanline >= 262) {
            scanline = 0;
            nmi_interrupt = false;
            status.set_sprite_zero_hit(false);
            status.reset_vblank_status();
            return true;
        }
    }

    return false;
}

bool NesPPU::is_sprite_0_hit(size_t cycle) const {
    uint8_t y = oam_data[0];
    uint8_t x = oam_data[3];
    return (y == scanline) && (x <= cycle) && mask.show_sprites();
}

bool NesPPU::poll_nmi_interrupt() {
    if (nmi_interrupt) {
        nmi_interrupt = false;
        return true;
    }
    return false;
}

void NesPPU::increment_vram_addr() {
    addr.increment(ctrl.vram_addr_increment());
}

void NesPPU::write_to_ctrl(uint8_t value) {
    bool before_nmi_status = ctrl.generate_vblank_nmi();
    ctrl.update(value);
    if (!before_nmi_status && ctrl.generate_vblank_nmi() && status.is_in_vblank()) {
        nmi_interrupt = true;
    }
}

void NesPPU::write_to_mask(uint8_t value) {
    mask.update(value);
}

uint8_t NesPPU::read_status() {
    uint8_t data = status.snapshot();
    status.reset_vblank_status();
    addr.reset_latch();
    scroll.reset_latch();
    return data;
}

void NesPPU::write_to_oam_addr(uint8_t value) {
    oam_addr = value;
}

void NesPPU::write_to_oam_data(uint8_t value) {
    oam_data[oam_addr] = value;
    oam_addr = oam_addr + 1;
}

uint8_t NesPPU::read_oam_data() const {
    return oam_data[oam_addr];
}

void NesPPU::write_to_scroll(uint8_t value) {
    scroll.write(value);
}

void NesPPU::write_to_ppu_addr(uint8_t value) {
    addr.update(value);
}

void NesPPU::write_to_data(uint8_t value) {
    uint16_t current_addr = addr.get();
    
    if (current_addr <= 0x1fff) {
        std::cout << "attempt to write to chr rom space " << current_addr << std::endl;
    } else if (current_addr >= 0x2000 && current_addr <= 0x2fff) {
        vram[mirror_vram_addr(current_addr)] = value;
    } else if (current_addr >= 0x3000 && current_addr <= 0x3eff) {
        std::cerr << "addr " << current_addr << " shouldn't be used in reality" << std::endl;
    } else if (current_addr == 0x3f10 || current_addr == 0x3f14 || 
               current_addr == 0x3f18 || current_addr == 0x3f1c) {
        uint16_t add_mirror = current_addr - 0x10;
        palette_table[add_mirror - 0x3f00] = value;
    } else if (current_addr >= 0x3f00 && current_addr <= 0x3fff) {
        palette_table[current_addr - 0x3f00] = value;
    } else {
        std::cerr << "unexpected access to mirrored space " << current_addr << std::endl;
    }
    
    increment_vram_addr();
}

uint8_t NesPPU::read_data() {
    uint16_t current_addr = addr.get();
    increment_vram_addr();
    
    if (current_addr <= 0x1fff) {
        uint8_t result = internal_data_buf;
        internal_data_buf = chr_rom[current_addr];
        return result;
    } else if (current_addr >= 0x2000 && current_addr <= 0x2fff) {
        uint8_t result = internal_data_buf;
        internal_data_buf = vram[mirror_vram_addr(current_addr)];
        return result;
    } else if (current_addr >= 0x3000 && current_addr <= 0x3eff) {
        std::cerr << "addr " << current_addr << " shouldn't be used in reality" << std::endl;
        return 0;
    } else if (current_addr == 0x3f10 || current_addr == 0x3f14 || 
               current_addr == 0x3f18 || current_addr == 0x3f1c) {
        uint16_t add_mirror = current_addr - 0x10;
        return palette_table[add_mirror - 0x3f00];
    } else if (current_addr >= 0x3f00 && current_addr <= 0x3fff) {
        return palette_table[current_addr - 0x3f00];
    } else {
        std::cerr << "unexpected access to mirrored space " << current_addr << std::endl;
        return 0;
    }
}

void NesPPU::write_oam_dma(const uint8_t* data) {
    for (int i = 0; i < 256; i++) {
        oam_data[oam_addr] = data[i];
        oam_addr = oam_addr + 1;
    }
}
