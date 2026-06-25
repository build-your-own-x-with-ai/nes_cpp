#pragma once

#include <cstdint>
#include <string>
#include <vector>

enum class Mirroring {
    Vertical,
    Horizontal,
    FourScreen,
};

struct Rom {
    std::vector<uint8_t> prg_rom;
    std::vector<uint8_t> chr_rom;
    uint8_t mapper;
    Mirroring screen_mirroring;
};

class Cartridge {
public:
    static const uint8_t NES_TAG[4];

    Cartridge() = default;
    ~Cartridge() = default;

    bool load_from_file(const std::string& path);
    bool load_from_data(const std::vector<uint8_t>& data);

    const Rom& get_rom() const { return rom_; }

    uint8_t read_prg_rom(uint16_t addr) const;
    uint8_t read_chr_rom(uint16_t addr) const;

private:
    Rom rom_;
};
