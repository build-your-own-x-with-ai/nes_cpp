#include "bus.h"
#include <iostream>

constexpr uint16_t RAM = 0x0000;
constexpr uint16_t RAM_MIRRORS_END = 0x1FFF;
constexpr uint16_t PPU_REGISTERS_MIRRORS_END = 0x3FFF;

Bus::Bus(const Rom& rom, GameLoopCallback callback)
    : prg_rom_(rom.prg_rom),
      ppu_(rom.chr_rom, rom.screen_mirroring),
      gameloop_callback_(std::move(callback)) {
    cpu_vram_.fill(0);
}

uint8_t Bus::read_prg_rom(uint16_t addr) const {
    uint16_t adjusted_addr = addr - 0x8000;
    if (prg_rom_.size() == 0x4000 && adjusted_addr >= 0x4000) {
        adjusted_addr = adjusted_addr % 0x4000;
    }
    return prg_rom_[adjusted_addr];
}

uint8_t Bus::mem_read(uint16_t addr) const {
    if (addr >= RAM && addr <= RAM_MIRRORS_END) {
        uint16_t mirror_addr = addr & 0b0000011111111111;
        return cpu_vram_[mirror_addr];
    }

    if (addr == 0x2000 || addr == 0x2001 || addr == 0x2003 ||
        addr == 0x2005 || addr == 0x2006 || addr == 0x4014) {
        return 0;
    }

    if (addr == 0x2002) {
        return const_cast<NesPPU&>(ppu_).read_status();
    }

    if (addr == 0x2004) {
        return ppu_.read_oam_data();
    }

    if (addr == 0x2007) {
        return const_cast<NesPPU&>(ppu_).read_data();
    }

    if (addr >= 0x4000 && addr <= 0x4015) {
        return 0;
    }

    if (addr == 0x4016) {
        return const_cast<Joypad&>(joypad1_).read();
    }

    if (addr == 0x4017) {
        return 0;
    }

    if (addr >= 0x2008 && addr <= PPU_REGISTERS_MIRRORS_END) {
        uint16_t mirror_down_addr = addr & 0b0010000000000111;
        return mem_read(mirror_down_addr);
    }

    if (addr >= 0x8000 && addr <= 0xFFFF) {
        return read_prg_rom(addr);
    }

    return 0;
}

void Bus::mem_write(uint16_t addr, uint8_t data) {
    if (addr >= RAM && addr <= RAM_MIRRORS_END) {
        uint16_t mirror_addr = addr & 0b11111111111;
        cpu_vram_[mirror_addr] = data;
        return;
    }

    if (addr == 0x2000) {
        ppu_.write_to_ctrl(data);
        return;
    }

    if (addr == 0x2001) {
        ppu_.write_to_mask(data);
        return;
    }

    if (addr == 0x2003) {
        ppu_.write_to_oam_addr(data);
        return;
    }

    if (addr == 0x2004) {
        ppu_.write_to_oam_data(data);
        return;
    }

    if (addr == 0x2005) {
        ppu_.write_to_scroll(data);
        return;
    }

    if (addr == 0x2006) {
        ppu_.write_to_ppu_addr(data);
        return;
    }

    if (addr == 0x2007) {
        ppu_.write_to_data(data);
        return;
    }

    if ((addr >= 0x4000 && addr <= 0x4013) || addr == 0x4015) {
        return;
    }

    if (addr == 0x4016) {
        joypad1_.write(data);
        return;
    }

    if (addr == 0x4017) {
        return;
    }

    if (addr == 0x4014) {
        std::array<uint8_t, 256> buffer{};
        uint16_t hi = static_cast<uint16_t>(data) << 8;
        for (int i = 0; i < 256; i++) {
            buffer[i] = mem_read(hi + i);
        }
        ppu_.write_oam_dma(buffer.data());
        return;
    }

    if (addr >= 0x2008 && addr <= PPU_REGISTERS_MIRRORS_END) {
        uint16_t mirror_down_addr = addr & 0b0010000000000111;
        mem_write(mirror_down_addr, data);
        return;
    }

    if (addr >= 0x8000 && addr <= 0xFFFF) {
        return;
    }
}

void Bus::insert_cartridge(const Cartridge& cartridge) {
    const Rom& rom = cartridge.get_rom();
    prg_rom_ = rom.prg_rom;
    ppu_ = NesPPU(rom.chr_rom, rom.screen_mirroring);
}

void Bus::tick(uint8_t cycles) {
    cycles_ += cycles;

    bool nmi_before = ppu_.nmi_interrupt;
    ppu_.tick(cycles * 3);
    bool nmi_after = ppu_.nmi_interrupt;

    if (!nmi_before && nmi_after) {
        if (gameloop_callback_) {
            gameloop_callback_(ppu_, joypad1_);
        }
    }
}

bool Bus::poll_nmi_status() {
    return ppu_.poll_nmi_interrupt();
}
