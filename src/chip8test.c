/*
 * Copyright 2017-2018 Ian Johnson
 *
 * This is free software, distributed under the MIT license.  A copy of the
 * license can be found in the LICENSE file in the project root, or at
 * https://opensource.org/licenses/MIT.
 */
/**
 * @file
 * Simple tests for the project.
 *
 * Tests are just functions with the signature `int test_func(void)`, which
 * return 0 on success or 1 on failure, and they should be registered in the
 * `main` function of this file.
 */
#include <stdint.h>
#include <stdio.h>

#include "assembler.h"
#include "interpreter.h"
#include "log.h"

#define TEST_RUN(test) testing_run(#test, test)
#define ASSERT(cond)                                                           \
    do {                                                                       \
        if (!(cond)) {                                                         \
            log_error("Assertion failed on line %d: " #cond, __LINE__);        \
            return 1;                                                          \
        }                                                                      \
    } while (0)
#define ASSERT_EQ(lhs, rhs, fmtstr)                                            \
    do {                                                                       \
        if ((lhs) != (rhs)) {                                                  \
            log_error(                                                         \
                "Assertion failed on line %d: " #lhs " == " #rhs, __LINE__);   \
            log_info("LHS = " fmtstr "; RHS = " fmtstr, (lhs), (rhs));         \
            return 1;                                                          \
        }                                                                      \
    } while (0)
#define ASSERT_EQ_UINT(lhs, rhs) ASSERT_EQ((lhs), (rhs), "%u")

static int n_failures;

static void testing_run(const char *name, int (*test)(void));
static void testing_setup(void);
static int testing_teardown(void);

static struct chip8_options chip8_options_testing(void);

/**
 * Tests Chip-8 arithmetic instruction evaluation.
 */
int test_arithmetic(void);
/**
 * Tests assembly of instructions.
 */
int test_asm(void);
/**
 * Tests assembly instruction alignment.
 */
int test_asm_align(void);
/**
 * Tests evaluation of expressions in assembler.
 */
int test_asm_eval(void);
/**
 * Tests whether the assembler fails properly on bad input.
 */
int test_asm_fail(void);
/**
 * Tests conditional assembly (IFDEF, etc.).
 */
int test_asm_if(void);
/**
 * Tests Chip-8 comparison instruction evaluation.
 */
int test_comparison(void);
/**
 * Tests Chip-8 display instruction evaluation.
 */
int test_display(void);
/**
 * Tests the evaluation of various jump instructions.
 */
int test_jp(void);
/**
 * Tests Chip-8 memory instruction evaluation.
 */
int test_ld(void);
/**
 * Tests shift and load quirks mode behavior.
 */
int test_quirks(void);

int main(int argc, char **argv)
{
    log_init(argv[0], stdout, LOG_DEBUG);
    testing_setup();
    TEST_RUN(test_arithmetic);
    TEST_RUN(test_asm);
    TEST_RUN(test_asm_align);
    TEST_RUN(test_asm_eval);
    TEST_RUN(test_asm_fail);
    TEST_RUN(test_asm_if);
    TEST_RUN(test_comparison);
    TEST_RUN(test_display);
    TEST_RUN(test_jp);
    TEST_RUN(test_ld);
    TEST_RUN(test_quirks);
    return testing_teardown();
}

int test_arithmetic(void)
{
    struct chip8_options opts = chip8_options_testing();
    struct chip8 *chip = chip8_new(opts);

    ASSERT(chip != NULL);

    /* LD V0, #66 */
    chip8_execute_opcode(chip, 0x6066);
    /* ADD V0, #0A */
    chip8_execute_opcode(chip, 0x700A);
    ASSERT_EQ_UINT(chip->regs[REG_V0], 0x70);
    ASSERT_EQ_UINT(chip->regs[REG_VF], 0);
    /* ADD V0, 0xFF */
    chip8_execute_opcode(chip, 0x70FF);
    ASSERT_EQ_UINT(chip->regs[REG_V0], 0x6F);
    ASSERT_EQ_UINT(chip->regs[REG_VF], 1);
    /* LD V1, 0x10 */
    chip8_execute_opcode(chip, 0x6110);
    /* OR V0, V1 */
    chip8_execute_opcode(chip, 0x8011);
    ASSERT_EQ_UINT(chip->regs[REG_V0], 0x7F);
    /* LD V2, 0xF0 */
    chip8_execute_opcode(chip, 0x62F0);
    /* AND V0, V2 */
    chip8_execute_opcode(chip, 0x8022);
    ASSERT_EQ_UINT(chip->regs[REG_V0], 0x70);
    /* XOR V0, V2 */
    chip8_execute_opcode(chip, 0x8023);
    ASSERT_EQ_UINT(chip->regs[REG_V0], 0x80);
    /* ADD V0, V2 */
    chip8_execute_opcode(chip, 0x8024);
    ASSERT_EQ_UINT(chip->regs[REG_V0], 0x70);
    ASSERT_EQ_UINT(chip->regs[REG_VF], 1);
    /* LD V1, 0x70 */
    chip8_execute_opcode(chip, 0x6170);
    /* ADD V0, V1 */
    chip8_execute_opcode(chip, 0x8014);
    ASSERT_EQ_UINT(chip->regs[REG_V0], 0xE0);
    ASSERT_EQ_UINT(chip->regs[REG_VF], 0);
    /* SUB V0, V1 */
    chip8_execute_opcode(chip, 0x8015);
    ASSERT_EQ_UINT(chip->regs[REG_V0], 0x70);
    ASSERT_EQ_UINT(chip->regs[REG_VF], 1);
    /* SUB V0, V2 */
    chip8_execute_opcode(chip, 0x8025);
    ASSERT_EQ_UINT(chip->regs[REG_V0], 0x80);
    ASSERT_EQ_UINT(chip->regs[REG_VF], 0);
    /* LD V3, #07 */
    chip8_execute_opcode(chip, 0x6307);
    /* SHR V3 */
    chip8_execute_opcode(chip, 0x8306);
    ASSERT_EQ_UINT(chip->regs[REG_V3], 0x03);
    ASSERT_EQ_UINT(chip->regs[REG_VF], 1);
    /* SHL V3 */
    chip8_execute_opcode(chip, 0x830E);
    ASSERT_EQ_UINT(chip->regs[REG_V3], 0x06);
    ASSERT_EQ_UINT(chip->regs[REG_VF], 0);
    /* SUBN V3, V0 */
    chip8_execute_opcode(chip, 0x8307);
    ASSERT_EQ_UINT(chip->regs[REG_V3], 0x7A);
    ASSERT_EQ_UINT(chip->regs[REG_VF], 1);
    /* LD VF, #80 */
    chip8_execute_opcode(chip, 0x6F80);
    /* ADD VF, #DD */
    chip8_execute_opcode(chip, 0x7FDD);
    ASSERT_EQ_UINT(chip->regs[REG_VF], 1);

    /* Unusual shift instruction representations */
    /* LD V3, #F0 */
    chip8_execute_opcode(chip, 0x63F0);
    /* LD V5, #00 */
    chip8_execute_opcode(chip, 0x6500);
    /* LD V7, #00 */
    chip8_execute_opcode(chip, 0x6700);
    /* SHL V3, V7 */
    chip8_execute_opcode(chip, 0x837E);
    ASSERT_EQ_UINT(chip->regs[REG_V3], 0xE0);
    ASSERT_EQ_UINT(chip->regs[REG_VF], 1);
    ASSERT_EQ_UINT(chip->regs[REG_V7], 0x00);
    /* SHR V3, V5 */
    chip8_execute_opcode(chip, 0x8356);
    ASSERT_EQ_UINT(chip->regs[REG_V3], 0x70);
    ASSERT_EQ_UINT(chip->regs[REG_VF], 0);
    ASSERT_EQ_UINT(chip->regs[REG_V5], 0x00);

    chip8_destroy(chip);
    return 0;
}

int test_asm(void)
{
#define TEST_INSTR(instr, opcode) \
    ASSERT(chip8asm_process_line(chipasm, (instr)) == 0); \
    ASSERT(chip8asm_emit(chipasm, prog) == 0); \
    ASSERT_EQ_UINT(chip8asm_program_opcode(prog, prog->len - 2), (opcode))

    struct chip8asm *chipasm = chip8asm_new(chip8asm_options_default());
    struct chip8asm_program *prog = chip8asm_program_new();

    ASSERT(chipasm != NULL && prog != NULL);

    ASSERT(!chip8asm_process_line(chipasm, "program_start:"));
    TEST_INSTR("   SCD 7", 0x00C7);
    TEST_INSTR("\tCLS", 0x00E0);
    TEST_INSTR("RET", 0x00EE);
    TEST_INSTR("SCR", 0x00FB);
    TEST_INSTR("SCL", 0x00FC);
    TEST_INSTR("EXIT", 0x00FD);
    TEST_INSTR("LOW", 0x00FE);
    TEST_INSTR("HIGH", 0x00FF);
    TEST_INSTR("JP program_start", 0x1200);
    TEST_INSTR("CALL program_start", 0x2200);
    TEST_INSTR("SE V8, #45", 0x3845);
    TEST_INSTR("SNE VA, #90", 0x4A90);
    TEST_INSTR("SE VE, V0", 0x5E00);
    TEST_INSTR("LD V1, $1101", 0x610D);
    TEST_INSTR("ADD V4, 10", 0x740A);
    TEST_INSTR("LD V7, VB", 0x87B0);
    TEST_INSTR("OR VD,\tVC", 0x8DC1);
    TEST_INSTR("AND VB,V6", 0x8B62);
    TEST_INSTR("XOR V5, V0", 0x8503);
    TEST_INSTR("ADD V5, V9", 0x8594);
    TEST_INSTR("SUB VD,VA", 0x8DA5);
    TEST_INSTR("SHR V3", 0x8306);
    TEST_INSTR("SUBN V9, VC", 0x89C7);
    TEST_INSTR("SHL VF", 0x8F0E);
    TEST_INSTR("SNE V8, V2", 0x9820);
    TEST_INSTR("LD I, program_start   ", 0xA200);
    TEST_INSTR("JP V0, program_start", 0xB200);
    TEST_INSTR("RND   V0,     #F5", 0xC0F5);
    TEST_INSTR("DRW V0, V1, 10", 0xD01A);
    TEST_INSTR("SKP V4\t;comment   , hi", 0xE49E);
    TEST_INSTR("SKNP VD", 0xEDA1);
    TEST_INSTR("\tLD\tV8,\tDT\t", 0xF807);
    TEST_INSTR("LD VD, K\t\t", 0xFD0A);
    TEST_INSTR("LD DT, VA", 0xFA15);
    TEST_INSTR("LD ST, V6", 0xF618);
    TEST_INSTR("ADD I, V3", 0xF31E);
    TEST_INSTR("LD F, V8;, V8", 0xF829);
    TEST_INSTR("LD HF, VE", 0xFE30);
    TEST_INSTR("LD B, V6", 0xF633);
    TEST_INSTR("LD [I], V2", 0xF255);
    TEST_INSTR("LD V7, [I]", 0xF765);
    TEST_INSTR("LD R, V1", 0xF175);
    TEST_INSTR("LD V6, R", 0xF685);

    /* Do a couple checks for case-insensitivity */
    TEST_INSTR("lD v5, [i]", 0xF565);
    TEST_INSTR("Ld sT, Va", 0xFA18);
    TEST_INSTR("hIgH", 0x00FF);

    /* But labels are case-sensitive */
    ASSERT(chip8asm_process_line(chipasm, "JP PROGRAM_START") == 0);
    log_set_output(NULL);
    ASSERT(chip8asm_emit(chipasm, prog));
    log_set_output(stdout);

    chip8asm_program_destroy(prog);
    chip8asm_destroy(chipasm);
    return 0;

#undef TEST_INSTR
}

int test_asm_align(void)
{
    struct chip8asm *chipasm = chip8asm_new(chip8asm_options_default());
    struct chip8asm_program *prog = chip8asm_program_new();

    ASSERT(chipasm != NULL && prog != NULL);

    ASSERT(!chip8asm_process_line(chipasm, "DW #1234"));
    ASSERT(!chip8asm_process_line(chipasm, "DB #56"));
    ASSERT(!chip8asm_process_line(chipasm, "DW #789A"));
    ASSERT(!chip8asm_process_line(chipasm, "JP #200"));
    ASSERT(!chip8asm_process_line(chipasm, "DB #BC"));
    /* Labels must also be aligned */
    ASSERT(!chip8asm_process_line(chipasm, "lbl:"));
    ASSERT(!chip8asm_process_line(chipasm, "JP lbl"));
    ASSERT(!chip8asm_emit(chipasm, prog));

    ASSERT_EQ_UINT(prog->mem[0], 0x12);
    ASSERT_EQ_UINT(prog->mem[1], 0x34);
    ASSERT_EQ_UINT(prog->mem[2], 0x56);
    ASSERT_EQ_UINT(prog->mem[3], 0x78);
    ASSERT_EQ_UINT(prog->mem[4], 0x9A);
    /* The JP instruction must be aligned */
    ASSERT_EQ_UINT(prog->mem[5], 0x00);
    ASSERT_EQ_UINT(prog->mem[6], 0x12);
    ASSERT_EQ_UINT(prog->mem[7], 0x00);
    ASSERT_EQ_UINT(prog->mem[8], 0xBC);
    ASSERT_EQ_UINT(prog->mem[9], 0x00);
    ASSERT_EQ_UINT(prog->mem[10], 0x12);
    ASSERT_EQ_UINT(prog->mem[11], 0x0A);

    chip8asm_program_destroy(prog);
    chip8asm_destroy(chipasm);
    return 0;
}

int test_asm_eval(void)
{
    struct chip8asm *chipasm = chip8asm_new(chip8asm_options_default());
    uint16_t value;

    ASSERT(chipasm != NULL);

    chip8asm_eval(chipasm, "2 + #F - $10", 1, &value);
    ASSERT_EQ_UINT(value, 15);
    chip8asm_eval(chipasm, "-1", 2, &value);
    ASSERT_EQ_UINT(value, UINT16_MAX);
    chip8asm_eval(chipasm, "2 + 3 * 1", 3, &value);
    ASSERT_EQ_UINT(value, 5);
    chip8asm_eval(chipasm, "((4 + 4) * (#0a - $00000010))", 4, &value);
    ASSERT_EQ_UINT(value, 64);
    chip8asm_eval(chipasm, "~$01010101 | $.1.1.1.1 ^ $.0.01111 & $10101010", 5, &value);
    ASSERT_EQ_UINT(value, 0xFFFF);
    chip8asm_eval(chipasm, "7 > 2 < 2", 6, &value);
    ASSERT_EQ_UINT(value, 4);
    chip8asm_eval(chipasm, "13 % 8 / 2", 7, &value);
    ASSERT_EQ_UINT(value, 2);
    chip8asm_eval(chipasm, "~--~45", 8, &value);
    ASSERT_EQ_UINT(value, 45);
    chip8asm_process_line(chipasm, "THE_ANSWER = 42");
    chip8asm_eval(chipasm, "THE_ANSWER - 2", 9, &value);
    ASSERT_EQ_UINT(value, 40);
    chip8asm_process_line(chipasm, "_crazyIDENT1234 = #FFFF");
    chip8asm_eval(chipasm, "~_crazyIDENT1234", 10, &value);
    ASSERT_EQ_UINT(value, 0);
    /* Test for failure on invalid expressions */
    log_set_output(NULL);
    ASSERT(chip8asm_eval(chipasm, "~1234~", 11, &value));
    ASSERT(chip8asm_eval(chipasm, "123+", 12, &value));
    ASSERT(chip8asm_eval(chipasm, "undefined", 13, &value));
    log_set_output(stderr);

    chip8asm_destroy(chipasm);
    return 0;
}

int test_asm_fail(void)
{
    struct chip8asm *chipasm = chip8asm_new(chip8asm_options_default());

    ASSERT(chipasm != NULL);
    log_set_output(NULL);
    ASSERT(chip8asm_process_line(chipasm, "LD VF,") != 0);
    ASSERT(chip8asm_process_line(chipasm, "HIGH,") != 0);
    ASSERT(chip8asm_process_line(chipasm, "HIGH ,") != 0);
    ASSERT(chip8asm_process_line(chipasm, "JP V0, #200,") != 0);
    ASSERT(chip8asm_process_line(chipasm, "LD VA,,2") != 0);
    ASSERT(chip8asm_process_line(chipasm, "  ADD V0  ,         ") != 0);
    ASSERT(chip8asm_process_line(chipasm, "ADD way, too, many, operands, here, hi, i") != 0);
    log_set_output(stderr);

    chip8asm_destroy(chipasm);
    return 0;
}

int test_asm_if(void)
{
    struct chip8asm *chipasm = chip8asm_new(chip8asm_options_default());
    struct chip8asm_program *prog = chip8asm_program_new();

    ASSERT(chipasm != NULL && prog != NULL);

    chip8asm_process_line(chipasm, "DEFINE TEST1");
    chip8asm_process_line(chipasm, "DEFINE TEST2");
    chip8asm_process_line(chipasm, "IFDEF TEST1");
    chip8asm_process_line(chipasm, "IFDEF UNDEFINED");
    chip8asm_process_line(chipasm, "DB 1");
    chip8asm_process_line(chipasm, "ELSE");
    chip8asm_process_line(chipasm, "DB 2");
    chip8asm_process_line(chipasm, "ENDIF");
    chip8asm_process_line(chipasm, "ELSE");
    chip8asm_process_line(chipasm, "IFDEF TEST2");
    chip8asm_process_line(chipasm, "DB 3");
    chip8asm_process_line(chipasm, "ELSE");
    chip8asm_process_line(chipasm, "DB 4");
    chip8asm_process_line(chipasm, "ENDIF");
    chip8asm_process_line(chipasm, "ENDIF");

    chip8asm_process_line(chipasm, "IFNDEF TEST2");
    chip8asm_process_line(chipasm, "DB 5");
    chip8asm_process_line(chipasm, "ELSE");
    chip8asm_process_line(chipasm, "DB 6");
    chip8asm_process_line(chipasm, "ENDIF");

    ASSERT(chip8asm_emit(chipasm, prog) == 0);
    ASSERT_EQ_UINT(prog->mem[0], 0x02);
    ASSERT_EQ_UINT(prog->mem[1], 0x06);

    chip8asm_program_destroy(prog);
    chip8asm_destroy(chipasm);
    return 0;
}

int test_comparison(void)
{
    struct chip8_options opts = chip8_options_testing();
    struct chip8 *chip = chip8_new(opts);
    uint16_t pc;

    ASSERT(chip != NULL);
    /* LD V0, #45 */
    chip8_execute_opcode(chip, 0x6045);
    pc = chip->pc;
    /* SE V0, #45 */
    chip8_execute_opcode(chip, 0x3045);
    ASSERT_EQ_UINT(chip->pc, pc + 4);
    pc = chip->pc;
    /* SNE V0, #45 */
    chip8_execute_opcode(chip, 0x4045);
    ASSERT_EQ_UINT(chip->pc, pc + 2);
    /* LD V1, #39 */
    chip8_execute_opcode(chip, 0x6139);
    pc = chip->pc;
    /* SE V0, V1 */
    chip8_execute_opcode(chip, 0x5010);
    ASSERT_EQ_UINT(chip->pc, pc + 2);
    pc = chip->pc;
    /* SNE V0, V1 */
    chip8_execute_opcode(chip, 0x9010);
    ASSERT_EQ_UINT(chip->pc, pc + 4);

    chip8_destroy(chip);
    return 0;
}

int test_display(void)
{
/* Compares the 8-byte-long row starting at (x, y) from the display */
#define ASSERT_EQ_ROW(x, y, row) \
    for (int i = 0; i < 8; i++) \
        ASSERT_EQ_UINT(chip->display[(x) + i][(y)], (row)[i]);

    struct chip8 *chip = chip8_new(chip8_options_testing());
    /* The three rows of the sprite */
    uint8_t row1[] = {1, 0, 1, 0, 0, 1, 0, 1};
    uint8_t row2[] = {0, 1, 0, 1, 1, 0, 1, 0};
    uint8_t row3[] = {1, 1, 1, 1, 0, 0, 0, 0};
    /* A zero row */
    uint8_t row_zero[] = {0, 0, 0, 0, 0, 0, 0, 0};

    ASSERT(chip != NULL);

    /* Put a 3-byte sprite in memory at #A00 */
    chip->mem[0xA00] = 0xA5; /* 10100101 */
    chip->mem[0xA01] = 0x5A; /* 01011010 */
    chip->mem[0xA02] = 0xF0; /* 11110000 */
    /* The rest of the memory is empty */

    /* LD V0, 1 */
    chip8_execute_opcode(chip, 0x6001);
    /* LD V1, 2 */
    chip8_execute_opcode(chip, 0x6102);
    /* LD I, #A00 */
    chip8_execute_opcode(chip, 0xAA00);

    /* DRW V0, V1, 3 */
    chip8_execute_opcode(chip, 0xD013);
    /* Make sure all three rows were drawn correctly */
    ASSERT_EQ_ROW(1, 2, row1);
    ASSERT_EQ_ROW(1, 3, row2);
    ASSERT_EQ_ROW(1, 4, row3);
    /* SCR */
    chip8_execute_opcode(chip, 0x00FB);
    /* There should be nothing left in the area we shifted from. */
    for (int x = 1; x < 5; x++)
        for (int y = 2; y <= 4; y++)
            ASSERT_EQ_UINT(chip->display[x][y], 0);
    ASSERT_EQ_ROW(5, 2, row1);
    ASSERT_EQ_ROW(5, 3, row2);
    ASSERT_EQ_ROW(5, 4, row3);
    /* SCL */
    chip8_execute_opcode(chip, 0x00FC);
    /* There should be nothing left in the area we shifted from. */
    ASSERT_EQ_ROW(9, 2, row_zero);
    ASSERT_EQ_ROW(9, 3, row_zero);
    ASSERT_EQ_ROW(9, 4, row_zero);
    ASSERT_EQ_ROW(1, 2, row1);
    ASSERT_EQ_ROW(1, 3, row2);
    ASSERT_EQ_ROW(1, 4, row3);
    /* SCD 8 */
    chip8_execute_opcode(chip, 0x00C8);
    /* There should be nothing left in the area we shifted from. */
    ASSERT_EQ_ROW(1, 2, row_zero);
    ASSERT_EQ_ROW(1, 3, row_zero);
    ASSERT_EQ_ROW(1, 4, row_zero);
    ASSERT_EQ_ROW(1, 10, row1);
    ASSERT_EQ_ROW(1, 11, row2);
    ASSERT_EQ_ROW(1, 12, row3);
    /* CLS */
    chip8_execute_opcode(chip, 0x00E0);
    for (int i = 0; i < CHIP8_DISPLAY_WIDTH; i++)
        for (int j = 0; j < CHIP8_DISPLAY_HEIGHT; j++)
            ASSERT_EQ_UINT(chip->display[i][j], 0);
    /* DRW V0, V1, 0 */
    chip8_execute_opcode(chip, 0xD010);
    /* Now we should have a 16x16 sprite */
    ASSERT_EQ_ROW(1, 2, row1);
    ASSERT_EQ_ROW(9, 2, row2);
    ASSERT_EQ_ROW(1, 3, row3);
    /* Drawing it again, we should get nothing */
    /* DRW V0, V1, 0 */
    chip8_execute_opcode(chip, 0xD010);
    ASSERT_EQ_ROW(1, 2, row_zero);
    ASSERT_EQ_ROW(9, 2, row_zero);
    ASSERT_EQ_ROW(1, 3, row_zero);

    chip8_destroy(chip);
    return 0;
#undef ASSERT_EQ_ROW
}

int test_jp(void)
{
    struct chip8 *chip = chip8_new(chip8_options_testing());

    ASSERT(chip != NULL);

    /* JP #400 */
    chip8_execute_opcode(chip, 0x1400);
    ASSERT_EQ_UINT(chip->pc, 0x400);
    /* CALL #200 */
    chip8_execute_opcode(chip, 0x2200);
    ASSERT_EQ_UINT(chip->pc, 0x200);
    /* CALL #300 */
    chip8_execute_opcode(chip, 0x2300);
    ASSERT_EQ_UINT(chip->pc, 0x300);
    /* RET */
    chip8_execute_opcode(chip, 0x00EE);
    ASSERT_EQ_UINT(chip->pc, 0x202);
    /* RET */
    chip8_execute_opcode(chip, 0x00EE);
    ASSERT_EQ_UINT(chip->pc, 0x402);

    chip8_destroy(chip);

    return 0;
}

int test_ld(void)
{
    struct chip8_options opts = chip8_options_testing();
    struct chip8 *chip = chip8_new(opts);
    uint16_t reg_i;

    ASSERT(chip != NULL);

    /* LD V5, #67 */
    chip8_execute_opcode(chip, 0x6567);
    ASSERT_EQ_UINT(chip->regs[REG_V5], 0x67);
    /* LD VA, V5 */
    chip8_execute_opcode(chip, 0x8A50);
    ASSERT_EQ_UINT(chip->regs[REG_VA], chip->regs[REG_V5]);
    /* LD I, #600 */
    chip8_execute_opcode(chip, 0xA600);
    ASSERT_EQ_UINT(chip->reg_i, 0x600);
    /* LD DT, V5 */
    chip8_execute_opcode(chip, 0xF515);
    ASSERT_EQ_UINT(chip->reg_dt, chip->regs[REG_V5]);
    /* LD V0, DT */
    chip8_execute_opcode(chip, 0xF007);
    ASSERT_EQ_UINT(chip->regs[REG_V0], chip->reg_dt);
    /* LD ST, V0 */
    chip8_execute_opcode(chip, 0xF018);
    ASSERT_EQ_UINT(chip->reg_st, chip->regs[REG_V0]);
    /* LD B, V5 */
    chip8_execute_opcode(chip, 0xF533);
    ASSERT_EQ_UINT(chip->mem[0x600], 1);
    ASSERT_EQ_UINT(chip->mem[0x601], 0);
    ASSERT_EQ_UINT(chip->mem[0x602], 3);

    /* Put some values in memory */
    reg_i = chip->reg_i;
    chip->mem[reg_i] = 0x12;
    chip->mem[reg_i + 1] = 0x34;
    /* LD V1, [I] */
    chip8_execute_opcode(chip, 0xF165);
    ASSERT_EQ_UINT(chip->reg_i, reg_i);
    ASSERT_EQ_UINT(chip->regs[REG_V0], 0x12);
    ASSERT_EQ_UINT(chip->regs[REG_V1], 0x34);
    chip->reg_i += 2;
    reg_i = chip->reg_i;
    /* LD [I], V1 */
    chip8_execute_opcode(chip, 0xF155);
    ASSERT_EQ_UINT(chip->reg_i, reg_i);
    ASSERT_EQ_UINT(chip->mem[reg_i], 0x12);
    ASSERT_EQ_UINT(chip->mem[reg_i + 1], 0x34);

    /* Put some values in the RPL flags. */
    chip->rpl[0] = 0x6A;
    chip->rpl[1] = 0x4B;
    chip->rpl[2] = 0xF8;
    /* LD V2, R */
    chip8_execute_opcode(chip, 0xF285);
    ASSERT_EQ_UINT(chip->regs[REG_V0], 0x6A);
    ASSERT_EQ_UINT(chip->regs[REG_V1], 0x4B);
    ASSERT_EQ_UINT(chip->regs[REG_V2], 0xF8);
    chip->rpl[0] = 0x50;
    chip->rpl[1] = 0x10;
    chip->rpl[2] = 0x02;
    /* LD R, V2 */
    chip8_execute_opcode(chip, 0xF275);
    ASSERT_EQ_UINT(chip->rpl[0], 0x6A);
    ASSERT_EQ_UINT(chip->rpl[1], 0x4B);
    ASSERT_EQ_UINT(chip->rpl[2], 0xF8);

    chip8_destroy(chip);
    return 0;
}

int test_quirks(void)
{
    struct chip8 *chip;
    struct chip8_options opts = chip8_options_testing();
    uint16_t reg_i;

    opts.load_quirks = true;
    opts.shift_quirks = true;
    chip = chip8_new(opts);
    ASSERT(chip != NULL);

    /* LD V0, #07 */
    chip8_execute_opcode(chip, 0x6007);
    /* LD V1, 0 */
    chip8_execute_opcode(chip, 0x6100);
    /* SHR V1, V0 */
    chip8_execute_opcode(chip, 0x8106);
    ASSERT_EQ_UINT(chip->regs[REG_V0], 0x07);
    ASSERT_EQ_UINT(chip->regs[REG_V1], 0x03);
    /* SHL V0, V1 */
    chip8_execute_opcode(chip, 0x801E);
    ASSERT_EQ_UINT(chip->regs[REG_V1], 0x03);
    ASSERT_EQ_UINT(chip->regs[REG_V0], 0x06);

    /* LD [I], V1 */
    reg_i = chip->reg_i;
    chip8_execute_opcode(chip, 0xF155);
    ASSERT_EQ_UINT(chip->reg_i, reg_i + 2);
    /* LD V1, [I] */
    reg_i = chip->reg_i;
    chip8_execute_opcode(chip, 0xF165);
    ASSERT_EQ_UINT(chip->reg_i, reg_i + 2);

    chip8_destroy(chip);
    return 0;
}

static void testing_run(const char *name, int (*test)(void))
{
    int res;

    log_info("Running test %s", name);
    if ((res = test())) {
        log_error("Test %s FAILED with status %d", name, res);
        n_failures++;
    } else {
        log_info("Test %s PASSED", name);
    }
}

static void testing_setup(void)
{
    log_debug("Starting tests");
}

static int testing_teardown(void)
{
    log_info("All tests finished with %d failure%s", n_failures, n_failures == 1 ? "" : "s");
    if (n_failures != 0)
        return 1;
    return 0;
}

static struct chip8_options chip8_options_testing(void)
{
    struct chip8_options opts = chip8_options_default();

    opts.enable_timer = false;
    opts.delay_draws = false;
    return opts;
}
