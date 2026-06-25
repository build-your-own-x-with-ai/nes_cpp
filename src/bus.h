#pragma once

#include "mem.h"
#include "cartridge.h"
#include "ppu/ppu.h"
#include "joypad.h"
#include <array>
#include <vector>
#include <functional>

class Bus : public Mem {
public:
    using GameLoopCallback = std::function<void(NesPPU&, Joypad&)>;

    Bus() = default;
    explicit Bus(const Rom& rom, GameLoopCallback callback = nullptr);

    uint8_t mem_read(uint16_t addr) const override;
    void mem_write(uint16_t addr, uint8_t data) override;

    void insert_cartridge(const Cartridge& cartridge);

    void tick(uint8_t cycles);
    bool poll_nmi_status();

    Joypad& joypad1() { return joypad1_; }
    NesPPU& ppu() { return ppu_; }

private:
    std::array<uint8_t, 2048> cpu_vram_{};
    std::vector<uint8_t> prg_rom_;
    NesPPU ppu_;
    Joypad joypad1_;

    size_t cycles_ = 0;
    GameLoopCallback gameloop_callback_;

    uint8_t read_prg_rom(uint16_t addr) const;
};
