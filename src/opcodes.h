#pragma once

#include <cstdint>
#include <unordered_map>

enum class AddressingMode {
    Immediate,
    ZeroPage,
    ZeroPage_X,
    ZeroPage_Y,
    Absolute,
    Absolute_X,
    Absolute_Y,
    Indirect_X,
    Indirect_Y,
    NoneAddressing,
};

struct OpCode {
    uint8_t code;
    const char* mnemonic;
    uint8_t len;
    uint8_t cycles;
    AddressingMode mode;
};

const OpCode& get_opcode(uint8_t code);
const std::unordered_map<uint8_t, const OpCode*>& get_opcodes_map();
