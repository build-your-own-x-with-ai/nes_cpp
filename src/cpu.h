#pragma once

#include "mem.h"
#include "opcodes.h"
#include <cstdint>
#include <functional>

constexpr uint8_t CARRY_FLAG_MASK = 0x01;
constexpr uint8_t ZERO_FLAG_MASK = 0x02;
constexpr uint8_t INTERRUPT_DISABLE_MASK = 0x04;
constexpr uint8_t DECIMAL_MODE_MASK = 0x08;
constexpr uint8_t BREAK_COMMAND_MASK = 0x10;
constexpr uint8_t BREAK_COMMAND_MASK2 = 0x20;
constexpr uint8_t OVERFLOW_FLAG_MASK = 0x40;
constexpr uint8_t NEGATIVE_FLAG_MASK = 0x80;

constexpr uint16_t STACK = 0x0100;
constexpr uint8_t STACK_RESET = 0xFD;

class Bus;

class CPU {
public:
    uint8_t register_a;
    uint8_t register_x;
    uint8_t register_y;
    uint8_t status;
    uint16_t program_counter;
    uint8_t stack_pointer;

    explicit CPU(Bus& bus);

    uint8_t mem_read(uint16_t addr) const;
    void mem_write(uint16_t addr, uint8_t data);

    uint16_t get_operand_address(const AddressingMode& mode);

    void load(const std::vector<uint8_t>& program);
    void reset();
    void run();
    void run_with_callback(std::function<void(CPU*)> callback);

    uint16_t get_absolute_address(const AddressingMode* mode, uint16_t addr) const;

    void nmi();

private:
    Bus& bus_;

    void set_register_a(uint8_t value);
    void update_zero_and_negative_flags(uint8_t result);
    void update_negative_flags(uint8_t result);

    void lda(const AddressingMode& mode);
    void ldx(const AddressingMode& mode);
    void ldy(const AddressingMode& mode);
    void sta(const AddressingMode& mode);
    void tax();
    void tay();
    void tsx();
    void txa();
    void txs();
    void tya();

    void inx();
    void iny();
    void dex();
    void dey();

    void asl_accumulator();
    void lsr_accumulator();
    void rol_accumulator();
    void ror_accumulator();
    uint8_t asl(const AddressingMode& mode);
    uint8_t lsr(const AddressingMode& mode);
    uint8_t rol(const AddressingMode& mode);
    uint8_t ror(const AddressingMode& mode);
    uint8_t inc(const AddressingMode& mode);
    uint8_t dec(const AddressingMode& mode);

    void and_(const AddressingMode& mode);
    void eor(const AddressingMode& mode);
    void ora(const AddressingMode& mode);

    void add_to_register_a(uint8_t data);
    void adc(const AddressingMode& mode);
    void sbc(const AddressingMode& mode);

    void compare(const AddressingMode& mode, uint8_t compare_with);

    void bit(const AddressingMode& mode);

    void branch(bool condition);

    void stack_push(uint8_t data);
    uint8_t stack_pop();
    void stack_push_u16(uint16_t data);
    uint16_t stack_pop_u16();

    void php();
    void plp();
    void pha();
    void pla();

    void set_carry_flag();
    void clear_carry_flag();

    // Unofficial/Illegal opcodes
    void lax(const AddressingMode& mode);
    void sax(const AddressingMode& mode);
    void dcp(const AddressingMode& mode);
    void isb(const AddressingMode& mode);
    void slo(const AddressingMode& mode);
    void rla(const AddressingMode& mode);
    void sre(const AddressingMode& mode);
    void rra(const AddressingMode& mode);
};
