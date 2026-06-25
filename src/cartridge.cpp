#include "cartridge.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cstring>

const uint8_t Cartridge::NES_TAG[4] = {0x4E, 0x45, 0x53, 0x1A};

constexpr size_t PRG_ROM_PAGE_SIZE = 16384;
constexpr size_t CHR_ROM_PAGE_SIZE = 8192;

bool Cartridge::load_from_file(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open file: " << path << std::endl;
        return false;
    }

    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> data(size);
    file.read(reinterpret_cast<char*>(data.data()), size);

    return load_from_data(data);
}

bool Cartridge::load_from_data(const std::vector<uint8_t>& data) {
    if (data.size() < 16) {
        std::cerr << "File too small to be iNES format" << std::endl;
        return false;
    }

    if (std::memcmp(data.data(), NES_TAG, 4) != 0) {
        std::cerr << "File is not in iNES file format" << std::endl;
        return false;
    }

    uint8_t mapper_lo = (data[6] >> 4) & 0x0F;
    uint8_t mapper_hi = data[7] & 0xF0;
    rom_.mapper = mapper_lo | mapper_hi;

    uint8_t ines_ver = (data[7] >> 2) & 0x03;
    if (ines_ver != 0) {
        std::cerr << "NES2.0 format is not supported" << std::endl;
        return false;
    }

    bool four_screen = (data[6] & 0b1000) != 0;
    bool vertical_mirroring = (data[6] & 0b1) != 0;

    if (four_screen) {
        rom_.screen_mirroring = Mirroring::FourScreen;
    } else if (vertical_mirroring) {
        rom_.screen_mirroring = Mirroring::Vertical;
    } else {
        rom_.screen_mirroring = Mirroring::Horizontal;
    }

    size_t prg_rom_size = static_cast<size_t>(data[4]) * PRG_ROM_PAGE_SIZE;
    size_t chr_rom_size = static_cast<size_t>(data[5]) * CHR_ROM_PAGE_SIZE;

    bool skip_trainer = (data[6] & 0b100) != 0;

    size_t prg_rom_start = 16 + (skip_trainer ? 512 : 0);
    size_t chr_rom_start = prg_rom_start + prg_rom_size;

    if (data.size() < chr_rom_start + chr_rom_size) {
        std::cerr << "File is truncated" << std::endl;
        return false;
    }

    rom_.prg_rom.resize(prg_rom_size);
    rom_.chr_rom.resize(chr_rom_size);

    std::copy_n(data.data() + prg_rom_start, prg_rom_size, rom_.prg_rom.begin());
    std::copy_n(data.data() + chr_rom_start, chr_rom_size, rom_.chr_rom.begin());

    return true;
}

uint8_t Cartridge::read_prg_rom(uint16_t addr) const {
    if (addr >= 0x8000) {
        size_t index = addr - 0x8000;
        if (rom_.prg_rom.size() == PRG_ROM_PAGE_SIZE && index >= PRG_ROM_PAGE_SIZE) {
            index = index % PRG_ROM_PAGE_SIZE;
        }
        if (index < rom_.prg_rom.size()) {
            return rom_.prg_rom[index];
        }
    }
    return 0;
}

uint8_t Cartridge::read_chr_rom(uint16_t addr) const {
    if (addr < rom_.chr_rom.size()) {
        return rom_.chr_rom[addr];
    }
    return 0;
}
