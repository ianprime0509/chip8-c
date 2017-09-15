/*
 * This file is part of Chip-8.
 *
 * Chip-8 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Chip-8 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Chip-8.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @file
 * Chip-8 instructions.
 */
#ifndef CHIP8_CORE_H
#define CHIP8_CORE_H

#include <stdint.h>

/**
 * The size of the Chip-8's memory, in bytes.
 */
#define CHIP8_MEM_SIZE 0x1000
/**
 * The address where programs should be loaded.
 */
#define CHIP8_PROG_START 0x200

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
     * Waits until the next clock cycle in low-resolution mode.
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
     * Sets `VF = 1` if there was a carry, or `0` if not.
     * Opcode `8xy4`.
     */
    OP_ADD_REG,
    /**
     * Subtract register from register (`SUB Vx, Vy`).
     * Sets `VF = 1` if there was *no* borrow, or `0` if there was one.
     * Opcode `8xy5`.
     */
    OP_SUB,
    /**
     * Bitwise right shift (`SHR Vx`). Sets `VF` to low bit of `Vx`.
     * Opcode `8x06`.
     */
    OP_SHR,
    /**
     * Store `Vy - Vx` in `Vx` (`SUBN Vx, Vy`).
     * Sets `VF = 1` if there was *no* borrow, or `0` if there was one.
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
     * This will block until a key is pressed, and then store the lowest key
     * number that is pressed. The buzzer will sound while any key is pressed,
     * and this instruction will block until the key is released.
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
     * The hundreds digit will be stored at address `I`, the tens digit at `I +
     * 1`, and the ones digit at `I + 2`.
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
     * Store `R` (flags) into registers (`LD Vx, R`). Super-Chip only.
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
        /* For storing the opcode of an invalid instruction */
        uint16_t opcode;
        uint16_t addr;
        uint8_t byte;
        uint8_t nibble;
    };
};

/**
 * Converts a Chip-8 opcode to an instruction.
 */
struct chip8_instruction chip8_instruction_from_opcode(uint16_t opcode);

#endif
