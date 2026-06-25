#include "trace.h"
#include "opcodes.h"
#include <sstream>
#include <iomanip>
#include <cctype>

std::string trace(CPU* cpu) {
    const auto& opcodes_map = get_opcodes_map();

    uint8_t code = cpu->mem_read(cpu->program_counter);
    auto it = opcodes_map.find(code);
    if (it == opcodes_map.end()) {
        return "Unknown opcode";
    }

    const OpCode* ops = it->second;
    uint16_t begin = cpu->program_counter;

    std::ostringstream hex_dump;
    hex_dump << std::hex << std::setfill('0') << std::uppercase;
    hex_dump << std::setw(2) << (int)code;

    uint16_t mem_addr = 0;
    uint8_t stored_value = 0;

    if (ops->mode != AddressingMode::Immediate && ops->mode != AddressingMode::NoneAddressing) {
        uint16_t addr = cpu->get_operand_address(ops->mode);
        mem_addr = addr;
        stored_value = cpu->mem_read(addr);
    }

    std::string tmp;

    if (ops->len == 1) {
        if (code == 0x0a || code == 0x4a || code == 0x2a || code == 0x6a) {
            tmp = "A ";
        }
    } else if (ops->len == 2) {
        uint8_t address = cpu->mem_read(begin + 1);
        hex_dump << " " << std::setw(2) << (int)address;

        switch (ops->mode) {
            case AddressingMode::Immediate:
                tmp = "#$" + std::to_string(address);
                break;
            case AddressingMode::ZeroPage:
                tmp = "$" + std::to_string(address) + " = " + std::to_string(stored_value);
                break;
            case AddressingMode::ZeroPage_X: {
                std::ostringstream oss;
                oss << "$" << std::to_string(address) << ",X @ " 
                    << std::to_string((address + cpu->register_x) & 0xFF) << " = " 
                    << std::to_string(stored_value);
                tmp = oss.str();
                break;
            }
            case AddressingMode::ZeroPage_Y: {
                std::ostringstream oss;
                oss << "$" << std::to_string(address) << ",Y @ " 
                    << std::to_string((address + cpu->register_y) & 0xFF) << " = " 
                    << std::to_string(stored_value);
                tmp = oss.str();
                break;
            }
            case AddressingMode::Indirect_X: {
                std::ostringstream oss;
                uint8_t ptr = (address + cpu->register_x) & 0xFF;
                oss << "($" << std::to_string(address) << ",X) @ " 
                    << std::to_string(ptr) << " = " 
                    << std::to_string(mem_addr) << " = " 
                    << std::to_string(stored_value);
                tmp = oss.str();
                break;
            }
            case AddressingMode::Indirect_Y: {
                std::ostringstream oss;
                oss << "($" << std::to_string(address) << "),Y = " 
                    << std::to_string(mem_addr - cpu->register_y) << " @ " 
                    << std::to_string(mem_addr) << " = " 
                    << std::to_string(stored_value);
                tmp = oss.str();
                break;
            }
            case AddressingMode::NoneAddressing: {
                int16_t offset = static_cast<int8_t>(address);
                uint16_t target = (begin + 2 + offset) & 0xFFFF;
                tmp = "$" + std::to_string(target);
                break;
            }
            default:
                break;
        }
    } else if (ops->len == 3) {
        uint8_t addr_lo = cpu->mem_read(begin + 1);
        uint8_t addr_hi = cpu->mem_read(begin + 2);
        hex_dump << " " << std::setw(2) << (int)addr_lo << " " << std::setw(2) << (int)addr_hi;

        uint16_t address = addr_lo | (static_cast<uint16_t>(addr_hi) << 8);

        if (ops->mode == AddressingMode::NoneAddressing) {
            if (code == 0x6c) {
                uint16_t jmp_addr;
                if ((address & 0x00FF) == 0x00FF) {
                    uint8_t lo = cpu->mem_read(address);
                    uint8_t hi = cpu->mem_read(address & 0xFF00);
                    jmp_addr = (static_cast<uint16_t>(hi) << 8) | lo;
                } else {
                    jmp_addr = cpu->mem_read(address) | (static_cast<uint16_t>(cpu->mem_read(address + 1)) << 8);
                }
                tmp = "($" + std::to_string(address) + ") = " + std::to_string(jmp_addr);
            } else {
                tmp = "$" + std::to_string(address);
            }
        } else if (ops->mode == AddressingMode::Absolute) {
            tmp = "$" + std::to_string(address) + " = " + std::to_string(stored_value);
        } else if (ops->mode == AddressingMode::Absolute_X) {
            std::ostringstream oss;
            oss << "$" << std::to_string(address) << ",X @ " 
                << std::to_string(mem_addr) << " = " 
                << std::to_string(stored_value);
            tmp = oss.str();
        } else if (ops->mode == AddressingMode::Absolute_Y) {
            std::ostringstream oss;
            oss << "$" << std::to_string(address) << ",Y @ " 
                << std::to_string(mem_addr) << " = " 
                << std::to_string(stored_value);
            tmp = oss.str();
        }
    }

    std::ostringstream result;
    result << std::hex << std::setfill('0') << std::uppercase;
    result << std::setw(4) << begin << "  ";
    result << std::setw(8) << hex_dump.str() << "  ";
    result << std::setw(4) << ops->mnemonic << " ";
    result << std::setw(25) << tmp << "  ";
    result << "A:" << std::setw(2) << (int)cpu->register_a << " ";
    result << "X:" << std::setw(2) << (int)cpu->register_x << " ";
    result << "Y:" << std::setw(2) << (int)cpu->register_y << " ";
    result << "P:" << std::setw(2) << (int)cpu->status << " ";
    result << "SP:" << std::setw(2) << (int)cpu->stack_pointer;

    std::string result_str = result.str();
    for (char& c : result_str) {
        c = std::toupper(c);
    }

    return result_str;
}
