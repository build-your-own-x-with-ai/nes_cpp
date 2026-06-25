#include "cpu.h"
#include "bus.h"
#include <iostream>
#include <cstring>

CPU::CPU(Bus& bus) : register_a(0), register_x(0), register_y(0),
    status(0b100100), program_counter(0), stack_pointer(STACK_RESET), bus_(bus) {
}

uint8_t CPU::mem_read(uint16_t addr) const {
    return bus_.mem_read(addr);
}

void CPU::mem_write(uint16_t addr, uint8_t data) {
    bus_.mem_write(addr, data);
}

uint16_t CPU::get_operand_address(const AddressingMode& mode) {
    switch (mode) {
        case AddressingMode::Immediate:
            return program_counter;

        case AddressingMode::ZeroPage:
            return mem_read(program_counter);

        case AddressingMode::Absolute:
            return mem_read(program_counter) | (static_cast<uint16_t>(mem_read(program_counter + 1)) << 8);

        case AddressingMode::ZeroPage_X: {
            uint8_t pos = mem_read(program_counter);
            return (pos + register_x) & 0xFF;
        }
        case AddressingMode::ZeroPage_Y: {
            uint8_t pos = mem_read(program_counter);
            return (pos + register_y) & 0xFF;
        }

        case AddressingMode::Absolute_X: {
            uint16_t base = mem_read(program_counter) | (static_cast<uint16_t>(mem_read(program_counter + 1)) << 8);
            return base + register_x;
        }
        case AddressingMode::Absolute_Y: {
            uint16_t base = mem_read(program_counter) | (static_cast<uint16_t>(mem_read(program_counter + 1)) << 8);
            return base + register_y;
        }

        case AddressingMode::Indirect_X: {
            uint8_t base = mem_read(program_counter);
            uint8_t ptr = (base + register_x) & 0xFF;
            uint8_t lo = mem_read(ptr);
            uint8_t hi = mem_read((ptr + 1) & 0xFF);
            return (static_cast<uint16_t>(hi) << 8) | lo;
        }
        case AddressingMode::Indirect_Y: {
            uint8_t base = mem_read(program_counter);
            uint8_t lo = mem_read(base);
            uint8_t hi = mem_read((base + 1) & 0xFF);
            uint16_t deref_base = (static_cast<uint16_t>(hi) << 8) | lo;
            return deref_base + register_y;
        }

        case AddressingMode::NoneAddressing:
            return 0;
    }
    return 0;
}

uint16_t CPU::get_absolute_address(const AddressingMode* mode, uint16_t addr) const {
    (void)mode;
    (void)addr;
    return 0;
}

void CPU::set_register_a(uint8_t value) {
    register_a = value;
    update_zero_and_negative_flags(register_a);
}

void CPU::update_zero_and_negative_flags(uint8_t result) {
    if (result == 0) {
        status |= ZERO_FLAG_MASK;
    } else {
        status &= ~ZERO_FLAG_MASK;
    }

    if (result & 0x80) {
        status |= NEGATIVE_FLAG_MASK;
    } else {
        status &= ~NEGATIVE_FLAG_MASK;
    }
}

void CPU::update_negative_flags(uint8_t result) {
    if (result & 0x80) {
        status |= NEGATIVE_FLAG_MASK;
    } else {
        status &= ~NEGATIVE_FLAG_MASK;
    }
}

void CPU::lda(const AddressingMode& mode) {
    uint16_t addr = get_operand_address(mode);
    uint8_t value = mem_read(addr);
    set_register_a(value);
}

void CPU::ldx(const AddressingMode& mode) {
    uint16_t addr = get_operand_address(mode);
    register_x = mem_read(addr);
    update_zero_and_negative_flags(register_x);
}

void CPU::ldy(const AddressingMode& mode) {
    uint16_t addr = get_operand_address(mode);
    register_y = mem_read(addr);
    update_zero_and_negative_flags(register_y);
}

void CPU::sta(const AddressingMode& mode) {
    uint16_t addr = get_operand_address(mode);
    mem_write(addr, register_a);
}

void CPU::tax() {
    register_x = register_a;
    update_zero_and_negative_flags(register_x);
}

void CPU::tay() {
    register_y = register_a;
    update_zero_and_negative_flags(register_y);
}

void CPU::tsx() {
    register_x = stack_pointer;
    update_zero_and_negative_flags(register_x);
}

void CPU::txa() {
    register_a = register_x;
    update_zero_and_negative_flags(register_a);
}

void CPU::txs() {
    stack_pointer = register_x;
}

void CPU::tya() {
    register_a = register_y;
    update_zero_and_negative_flags(register_a);
}

void CPU::inx() {
    register_x = (register_x == 255) ? 0 : register_x + 1;
    update_zero_and_negative_flags(register_x);
}

void CPU::iny() {
    register_y = (register_y == 255) ? 0 : register_y + 1;
    update_zero_and_negative_flags(register_y);
}

void CPU::dex() {
    register_x = (register_x == 0) ? 255 : register_x - 1;
    update_zero_and_negative_flags(register_x);
}

void CPU::dey() {
    register_y = (register_y == 0) ? 255 : register_y - 1;
    update_zero_and_negative_flags(register_y);
}

void CPU::asl_accumulator() {
    if (register_a & 0x80) {
        set_carry_flag();
    } else {
        clear_carry_flag();
    }
    register_a = register_a << 1;
    update_zero_and_negative_flags(register_a);
}

uint8_t CPU::asl(const AddressingMode& mode) {
    uint16_t addr = get_operand_address(mode);
    uint8_t data = mem_read(addr);
    if (data & 0x80) {
        set_carry_flag();
    } else {
        clear_carry_flag();
    }
    data = data << 1;
    mem_write(addr, data);
    update_zero_and_negative_flags(data);
    return data;
}

void CPU::lsr_accumulator() {
    if (register_a & 1) {
        set_carry_flag();
    } else {
        clear_carry_flag();
    }
    register_a = register_a >> 1;
    update_zero_and_negative_flags(register_a);
}

uint8_t CPU::lsr(const AddressingMode& mode) {
    uint16_t addr = get_operand_address(mode);
    uint8_t data = mem_read(addr);
    if (data & 1) {
        set_carry_flag();
    } else {
        clear_carry_flag();
    }
    data = data >> 1;
    mem_write(addr, data);
    update_zero_and_negative_flags(data);
    return data;
}

void CPU::rol_accumulator() {
    uint8_t old_carry = (status & CARRY_FLAG_MASK) ? 1 : 0;
    if (register_a & 0x80) {
        set_carry_flag();
    } else {
        clear_carry_flag();
    }
    register_a = register_a << 1;
    register_a |= old_carry;
    update_zero_and_negative_flags(register_a);
}

uint8_t CPU::rol(const AddressingMode& mode) {
    uint16_t addr = get_operand_address(mode);
    uint8_t data = mem_read(addr);
    uint8_t old_carry = (status & CARRY_FLAG_MASK) ? 1 : 0;

    if (data & 0x80) {
        set_carry_flag();
    } else {
        clear_carry_flag();
    }
    data = data << 1;
    data |= old_carry;
    mem_write(addr, data);
    update_negative_flags(data);
    return data;
}

void CPU::ror_accumulator() {
    uint8_t old_carry = (status & CARRY_FLAG_MASK) ? 0x80 : 0;
    if (register_a & 1) {
        set_carry_flag();
    } else {
        clear_carry_flag();
    }
    register_a = register_a >> 1;
    register_a |= old_carry;
    update_zero_and_negative_flags(register_a);
}

uint8_t CPU::ror(const AddressingMode& mode) {
    uint16_t addr = get_operand_address(mode);
    uint8_t data = mem_read(addr);
    uint8_t old_carry = (status & CARRY_FLAG_MASK) ? 0x80 : 0;

    if (data & 1) {
        set_carry_flag();
    } else {
        clear_carry_flag();
    }
    data = data >> 1;
    data |= old_carry;
    mem_write(addr, data);
    update_negative_flags(data);
    return data;
}

uint8_t CPU::inc(const AddressingMode& mode) {
    uint16_t addr = get_operand_address(mode);
    uint8_t data = mem_read(addr);
    data = data + 1;
    mem_write(addr, data);
    update_zero_and_negative_flags(data);
    return data;
}

uint8_t CPU::dec(const AddressingMode& mode) {
    uint16_t addr = get_operand_address(mode);
    uint8_t data = mem_read(addr);
    data = data - 1;
    mem_write(addr, data);
    update_zero_and_negative_flags(data);
    return data;
}

void CPU::and_(const AddressingMode& mode) {
    uint16_t addr = get_operand_address(mode);
    uint8_t data = mem_read(addr);
    set_register_a(data & register_a);
}

void CPU::eor(const AddressingMode& mode) {
    uint16_t addr = get_operand_address(mode);
    uint8_t data = mem_read(addr);
    set_register_a(data ^ register_a);
}

void CPU::ora(const AddressingMode& mode) {
    uint16_t addr = get_operand_address(mode);
    uint8_t data = mem_read(addr);
    set_register_a(data | register_a);
}

void CPU::set_carry_flag() {
    status |= CARRY_FLAG_MASK;
}

void CPU::clear_carry_flag() {
    status &= ~CARRY_FLAG_MASK;
}

void CPU::add_to_register_a(uint8_t data) {
    uint16_t sum = register_a + data + ((status & CARRY_FLAG_MASK) ? 1 : 0);
    uint8_t result = sum & 0xFF;

    if (sum > 0xFF) {
        set_carry_flag();
    } else {
        clear_carry_flag();
    }

    if ((data ^ result) & (result ^ register_a) & 0x80) {
        status |= OVERFLOW_FLAG_MASK;
    } else {
        status &= ~OVERFLOW_FLAG_MASK;
    }

    set_register_a(result);
}

void CPU::adc(const AddressingMode& mode) {
    uint16_t addr = get_operand_address(mode);
    uint8_t value = mem_read(addr);
    add_to_register_a(value);
}

void CPU::sbc(const AddressingMode& mode) {
    uint16_t addr = get_operand_address(mode);
    uint8_t data = mem_read(addr);
    add_to_register_a((~data) & 0xFF);
}

void CPU::compare(const AddressingMode& mode, uint8_t compare_with) {
    uint16_t addr = get_operand_address(mode);
    uint8_t data = mem_read(addr);
    if (data <= compare_with) {
        set_carry_flag();
    } else {
        clear_carry_flag();
    }
    update_zero_and_negative_flags(compare_with - data);
}

void CPU::bit(const AddressingMode& mode) {
    uint16_t addr = get_operand_address(mode);
    uint8_t data = mem_read(addr);
    uint8_t and_result = register_a & data;

    if (and_result == 0) {
        status |= ZERO_FLAG_MASK;
    } else {
        status &= ~ZERO_FLAG_MASK;
    }

    status = (status & ~NEGATIVE_FLAG_MASK) | (data & NEGATIVE_FLAG_MASK);
    status = (status & ~OVERFLOW_FLAG_MASK) | (data & OVERFLOW_FLAG_MASK);
}

void CPU::branch(bool condition) {
    if (condition) {
        int8_t jump = static_cast<int8_t>(mem_read(program_counter));
        uint16_t jump_addr = (program_counter + 1) + static_cast<uint16_t>(jump);
        program_counter = jump_addr;
    }
}

void CPU::stack_push(uint8_t data) {
    mem_write(STACK + stack_pointer, data);
    stack_pointer = (stack_pointer - 1) & 0xFF;
}

uint8_t CPU::stack_pop() {
    stack_pointer = (stack_pointer + 1) & 0xFF;
    return mem_read(STACK + stack_pointer);
}

void CPU::stack_push_u16(uint16_t data) {
    uint8_t hi = (data >> 8) & 0xFF;
    uint8_t lo = data & 0xFF;
    stack_push(hi);
    stack_push(lo);
}

uint16_t CPU::stack_pop_u16() {
    uint8_t lo = stack_pop();
    uint8_t hi = stack_pop();
    return (static_cast<uint16_t>(hi) << 8) | lo;
}

void CPU::php() {
    stack_push(status | BREAK_COMMAND_MASK | BREAK_COMMAND_MASK2);
}

void CPU::plp() {
    status = stack_pop();
    status &= ~BREAK_COMMAND_MASK;
    status |= BREAK_COMMAND_MASK2;
}

void CPU::pha() {
    stack_push(register_a);
}

void CPU::pla() {
    uint8_t data = stack_pop();
    set_register_a(data);
}

void CPU::load(const std::vector<uint8_t>& program) {
    for (size_t i = 0; i < program.size() && (0x8000 + i) < 0xFFFF; i++) {
        mem_write(0x8000 + static_cast<uint16_t>(i), program[i]);
    }
    mem_write(0xFFFC, 0x00);
    mem_write(0xFFFD, 0x80);
}

void CPU::reset() {
    register_a = 0;
    register_x = 0;
    register_y = 0;
    stack_pointer = STACK_RESET;
    status = 0b100100;

    program_counter = mem_read(0xFFFC) | (static_cast<uint16_t>(mem_read(0xFFFD)) << 8);
}

void CPU::run() {
    run_with_callback([](CPU*) {});
}

void CPU::run_with_callback(std::function<void(CPU*)> callback) {
    const auto& opcodes = get_opcodes_map();

    while (true) {
        uint8_t code = mem_read(program_counter);
        program_counter++;

        auto it = opcodes.find(code);
        if (it == opcodes.end()) {
            std::cerr << "Unknown opcode: 0x" << std::hex << (int)code << std::endl;
            return;
        }

        const OpCode& opcode = *(it->second);
        uint16_t program_counter_state = program_counter;

        switch (code) {
            case 0xa9: case 0xa5: case 0xb5: case 0xad: case 0xbd: case 0xb9: case 0xa1: case 0xb1:
                lda(opcode.mode);
                break;

            case 0xa2: case 0xa6: case 0xb6: case 0xae: case 0xbe:
                ldx(opcode.mode);
                break;

            case 0xa0: case 0xa4: case 0xb4: case 0xac: case 0xbc:
                ldy(opcode.mode);
                break;

            case 0x85: case 0x95: case 0x8d: case 0x9d: case 0x99: case 0x81: case 0x91:
                sta(opcode.mode);
                break;

            case 0x86: case 0x96: case 0x8e:
                mem_write(get_operand_address(opcode.mode), register_x);
                break;

            case 0x84: case 0x94: case 0x8c:
                mem_write(get_operand_address(opcode.mode), register_y);
                break;

            case 0xaa:
                tax();
                break;
            case 0xe8:
                inx();
                break;
            case 0x00:
                return;

            case 0xd8:
                status &= ~DECIMAL_MODE_MASK;
                break;
            case 0x58:
                status &= ~INTERRUPT_DISABLE_MASK;
                break;
            case 0xb8:
                status &= ~OVERFLOW_FLAG_MASK;
                break;
            case 0x18:
                clear_carry_flag();
                break;
            case 0x38:
                set_carry_flag();
                break;
            case 0x78:
                status |= INTERRUPT_DISABLE_MASK;
                break;
            case 0xf8:
                status |= DECIMAL_MODE_MASK;
                break;

            case 0x48:
                pha();
                break;
            case 0x68:
                pla();
                break;
            case 0x08:
                php();
                break;
            case 0x28:
                plp();
                break;

            case 0x69: case 0x65: case 0x75: case 0x6d: case 0x7d: case 0x79: case 0x61: case 0x71:
                adc(opcode.mode);
                break;

            case 0xe9: case 0xe5: case 0xf5: case 0xed: case 0xfd: case 0xf9: case 0xe1: case 0xf1:
                sbc(opcode.mode);
                break;

            case 0x29: case 0x25: case 0x35: case 0x2d: case 0x3d: case 0x39: case 0x21: case 0x31:
                and_(opcode.mode);
                break;

            case 0x49: case 0x45: case 0x55: case 0x4d: case 0x5d: case 0x59: case 0x41: case 0x51:
                eor(opcode.mode);
                break;

            case 0x09: case 0x05: case 0x15: case 0x0d: case 0x1d: case 0x19: case 0x01: case 0x11:
                ora(opcode.mode);
                break;

            case 0x4a:
                lsr_accumulator();
                break;
            case 0x46: case 0x56: case 0x4e: case 0x5e:
                lsr(opcode.mode);
                break;

            case 0x0a:
                asl_accumulator();
                break;
            case 0x06: case 0x16: case 0x0e: case 0x1e:
                asl(opcode.mode);
                break;

            case 0x2a:
                rol_accumulator();
                break;
            case 0x26: case 0x36: case 0x2e: case 0x3e:
                rol(opcode.mode);
                break;

            case 0x6a:
                ror_accumulator();
                break;
            case 0x66: case 0x76: case 0x6e: case 0x7e:
                ror(opcode.mode);
                break;

            case 0xe6: case 0xf6: case 0xee: case 0xfe:
                inc(opcode.mode);
                break;

            case 0xc8:
                iny();
                break;

            case 0xc6: case 0xd6: case 0xce: case 0xde:
                dec(opcode.mode);
                break;

            case 0xca:
                dex();
                break;
            case 0x88:
                dey();
                break;

            case 0xc9: case 0xc5: case 0xd5: case 0xcd: case 0xdd: case 0xd9: case 0xc1: case 0xd1:
                compare(opcode.mode, register_a);
                break;

            case 0xc0: case 0xc4: case 0xcc:
                compare(opcode.mode, register_y);
                break;

            case 0xe0: case 0xe4: case 0xec:
                compare(opcode.mode, register_x);
                break;

            case 0x4c: {
                uint16_t addr = mem_read(program_counter) | (static_cast<uint16_t>(mem_read(program_counter + 1)) << 8);
                program_counter = addr;
                break;
            }

            case 0x6c: {
                uint16_t mem_addr = mem_read(program_counter) | (static_cast<uint16_t>(mem_read(program_counter + 1)) << 8);
                uint16_t indirect_ref;
                if ((mem_addr & 0x00FF) == 0x00FF) {
                    uint8_t lo = mem_read(mem_addr);
                    uint8_t hi = mem_read(mem_addr & 0xFF00);
                    indirect_ref = (static_cast<uint16_t>(hi) << 8) | lo;
                } else {
                    indirect_ref = mem_read(mem_addr) | (static_cast<uint16_t>(mem_read(mem_addr + 1)) << 8);
                }
                program_counter = indirect_ref;
                break;
            }

            case 0x20: {
                stack_push_u16(program_counter + 2 - 1);
                uint16_t target = mem_read(program_counter) | (static_cast<uint16_t>(mem_read(program_counter + 1)) << 8);
                program_counter = target;
                break;
            }

            case 0x60:
                program_counter = stack_pop_u16() + 1;
                break;

            case 0x40:
                status = stack_pop();
                status &= ~BREAK_COMMAND_MASK;
                status |= BREAK_COMMAND_MASK2;
                program_counter = stack_pop_u16();
                break;

            case 0xd0:
                branch((status & ZERO_FLAG_MASK) == 0);
                break;
            case 0x70:
                branch((status & OVERFLOW_FLAG_MASK) != 0);
                break;
            case 0x50:
                branch((status & OVERFLOW_FLAG_MASK) == 0);
                break;
            case 0x10:
                branch((status & NEGATIVE_FLAG_MASK) == 0);
                break;
            case 0x30:
                branch((status & NEGATIVE_FLAG_MASK) != 0);
                break;
            case 0xf0:
                branch((status & ZERO_FLAG_MASK) != 0);
                break;
            case 0xb0:
                branch((status & CARRY_FLAG_MASK) != 0);
                break;
            case 0x90:
                branch((status & CARRY_FLAG_MASK) == 0);
                break;

            case 0x24: case 0x2c:
                bit(opcode.mode);
                break;

            case 0xea:
                break;

            case 0xa8:
                tay();
                break;
            case 0xba:
                tsx();
                break;
            case 0x8a:
                txa();
                break;
            case 0x9a:
                txs();
                break;
            case 0x98:
                tya();
                break;

            default:
                std::cerr << "Unhandled opcode: 0x" << std::hex << (int)code << std::endl;
                return;
        }

        if (program_counter_state == program_counter) {
            program_counter += (opcode.len - 1);
        }

        bus_.tick(opcode.cycles);

        if (bus_.poll_nmi_status()) {
            nmi();
        }

        callback(this);
    }
}

void CPU::nmi() {
    stack_push_u16(program_counter);
    uint8_t status_copy = status;
    status_copy &= ~BREAK_COMMAND_MASK;
    status_copy |= BREAK_COMMAND_MASK2;
    stack_push(status_copy);
    status |= INTERRUPT_DISABLE_MASK;
    bus_.tick(2);
    program_counter = mem_read(0xFFFA) | (static_cast<uint16_t>(mem_read(0xFFFB)) << 8);
}
