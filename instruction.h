/**
 * @file
 * Chip-8 instructions.
 */
#ifndef CHIP8_CORE_H
#define CHIP8_CORE_H

#include <stdint.h>

/**
 * A Chip-8 register.
 */
enum chip8_register {
    REG_V0 = 0,
    REG_V1,
    REG_V2,
    REG_V3,
    REG_V4,
    REG_V5,
    REG_V6,
    REG_V7,
    REG_V8,
    REG_V9,
    REG_VA,
    REG_VB,
    REG_VC,
    REG_VD,
    REG_VE,
    REG_VF,
};

/**
 * An operation for a Chip-8 instruction.
 */
enum chip8_operation {
    /**
     * Invalid instruction.
     */
    OP_INVALID = -1,
    /**
     * Scroll screen down (`SCD nibble`). Super-Chip only.
     * Will scroll the screen by `nibble` pixels.
     * Opcode `00Cn`.
     */
    OP_SCD,
    /**
     * Clear the display (`CLS`).
     * Opcode `00E0`.
     */
    OP_CLS,
    /**
     * Return from subroutine (`RET`).
     * Opcode `00EE`.
     */
    OP_RET,
    /**
     * Scroll screen right by 4 pixels (`SCR`). Super-Chip only.
     * Opcode `00FB`.
     */
    OP_SCR,
    /**
     * Scroll screen left by 4 pixels (`SCL`). Super-Chip only.
     * Opcode `00FC`.
     */
    OP_SCL,
    /**
     * Exit the interpreter (`EXIT`). Super-Chip only.
     * Opcode `0xFD`.
     */
    OP_EXIT,
    /**
     * Enter low-resolution (64x32) mode (`LOW`). Super-Chip only.
     * Opcode `00FE`.
     */
    OP_LOW,
    /**
     * Enter high-resolution (128x64) mode (`HIGH`). Super-Chip only.
     * Opcode `00FF`.
     */
    OP_HIGH,
    /**
     * Jump to address (`JP addr`).
     * Opcode `1nnn`.
     */
    OP_JP,
    /**
     * Call subroutine (`CALL addr`).
     * Opcode `2nnn`.
     */
    OP_CALL,
    /**
     * Skip next instruction if byte equality test (`SE Vx, byte`).
     * Opcode `3xkk`.
     */
    OP_SE_BYTE,
    /**
     * Skip next instruction if byte inequality test (`SNE Vx, byte`).
     * Opcode `4xkk`.
     */
    OP_SNE_BYTE,
    /**
     * Skip next instruction if register equality test (`SE Vx, Vy`).
     * Opcode `5xy0`.
     */
    OP_SE_REG,
    /**
     * Load byte into register (`LD Vx, byte`).
     * Opcode `6xkk`.
     */
    OP_LD_BYTE,
    /**
     * Add byte to register (`ADD Vx, byte`). Sets `VF` on carry.
     * Opcode `7xkk`.
     */
    OP_ADD_BYTE,
    /**
     * Load register into register (`LD, Vx, Vy`).
     * Opcode `8xy0`.
     */
    OP_LD_REG,
    /**
     * Bitwise OR (`OR Vx, Vy`).
     * Opcode `8xy1`.
     */
    OP_OR,
    /**
     * Bitwise AND (`AND Vx, Vy`).
     * Opcode `8xy2`.
     */
    OP_AND,
    /**
     * Bitwise XOR (`XOR Vx, Vy`).
     * Opcode `8xy3`.
     */
    OP_XOR,
    /**
     * Add register to register (`ADD Vx, Vy`).
     * Opcode `8xy4`.
     */
    OP_ADD_REG,
    /**
     * Subtract register from register (`SUB Vx, Vy`). Sets `VF` on borrow.
     * Opcode `8xy5`.
     */
    OP_SUB,
    /**
     * Bitwise right shift (`SHR Vx`). Sets `VF` to low bit of `Vx`.
     * Opcode `8x06`.
     */
    OP_SHR,
    /**
     * `SUBN Vx, Vy` does the same thing as `SUB Vy, Vx`.
     * Opcode `8xy7`.
     */
    OP_SUBN,
    /**
     * Bitwise left shift (`SHL Vx`). Sets `VF` to high bit of `Vx`.
     * Opcode `8x0E`.
     */
    OP_SHL,
    /**
     * Skip next instruction if register inequality test (`SNE Vx, Vy`).
     * Opcode `9xy0`.
     */
    OP_SNE_REG,
    /**
     * Load address into I (`LD I, addr`).
     * Opcode `Annn`.
     */
    OP_LD_I,
    /**
     * Jump to address offset by `V0` (`JP V0, addr`).
     * Opcode `Bnnn`.
     */
    OP_JP_V0,
    /**
     * Load random byte to register and mask (`RND Vx, byte`).
     * A random byte will be AND-ed with the given byte and stored in `Vx`.
     * Opcode `Cxkk`.
     */
    OP_RND,
    /**
     * Draw sprite (`DRW Vx, Vy, nibble`). Sets `VF` on collision.
     * The sprite will be `nibble` bytes long, read starting at memory location
     * `I`, and drawn at position `(Vx, Vy)`. On the Super-Chip only, `nibble`
     * may be 0, in which case the sprite will be 16x16 pixels.
     * Opcode `Dxyn`.
     */
    OP_DRW,
    /**
     * Skip next instruction if key is down (`SKP Vx`).
     * The lower 4 bits of `Vx` specify the key.
     * Opcode `Ex9E`.
     */
    OP_SKP,
    /**
     * Skip next instruction if key is not down (`SKNP Vx`).
     * The lower 4 bits of `Vx` specify the key.
     * Opcode `ExA1`.
     */
    OP_SKNP,
    /**
     * Load delay timer value into register (`LD Vx, DT`).
     * Opcode `Fx07`.
     */
    OP_LD_REG_DT,
    /**
     * Wait for key press and store key value (`LD Vx, K`).
     * Opcode `Fx0A`.
     */
    OP_LD_KEY,
    /**
     * Load register value into delay timer (`LD DT, Vx`).
     * Opcode `Fx15`.
     */
    OP_LD_DT_REG,
    /**
     * Load register value into sound timer (`LD ST, Vx`).
     * Opcode `Fx18`.
     */
    OP_LD_ST,
    /**
     * Add register to I (`ADD I, Vx`).
     * Opcode `Fx1E`.
     */
    OP_ADD_I,
    /**
     * Load 4x5 hex digit sprite location into I (`LD F, Vx`).
     * The lower 4 bits of `Vx` will be the sprite to load.
     * Opcode `Fx29`.
     */
    OP_LD_F,
    /**
     * Load 8x10 (?) hex digit sprite location into I (`LD HF, Vx`). Super-Chip
     * only.
     * The lower 4 bits of `Vx` will be the sprite to load.
     * Opcode `Fx30`.
     */
    OP_LD_HF,
    /**
     * Store 3-digit BCD representation of register (`LD B, Vx`).
     * Opcode `Fx33`.
     */
    OP_LD_B,
    /**
     * Store registers up to `Vx` in memory (`LD [I], Vx`).
     * Opcode `Fx55`.
     */
    OP_LD_DEREF_I_REG,
    /**
     * Read registers up to `Vx` from memory (`LD Vx, [I]`).
     * Opcode `Fx65`.
     */
    OP_LD_REG_DEREF_I,
    /**
     * Store register values into `R` (flags) (`LD R, Vx`). Super-Chip only.
     * Registers `V0` to `Vx` will be stored (where `x < 8`).
     * Opcode `Fx75`.
     * TODO: what is this?
     */
    OP_LD_R_REG,
    /**
     * Store `R` (flags) into registers (`LD R, Vx`). Super-Chip only.
     * Will store into registers `V0` to `Vx` (where `x < 8`).
     * Opcode `Fx85`.
     * TODO: what is this?
     */
    OP_LD_REG_R,
};

/**
 * A complete Chip-8 instruction (operation and arguments).
 */
struct chip8_instruction {
    enum chip8_operation op;
    /*
     * The arguments to the operation (same format as above). These are
     * undefined if the argument does not appear with the given operation.
     */
    enum chip8_register vx;
    enum chip8_register vy;
    union {
        uint16_t addr;
        uint8_t byte;
        uint8_t nibble;
    };
};

/**
 * Converts a Chip-8 opcode to an instruction.
 */
struct chip8_instruction opcode_to_instruction(uint16_t opcode);

#endif