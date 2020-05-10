#ifndef CHIP8TEST_H
#define CHIP8TEST_H

// ---------------  MACRO CHE CONTENGONO LE OPERAZIONI IMPLEMENTATE  ------------------
#define CLEAR_SCREEN                 0x00E0
#define SUBROUTINE_CALL(N)           (0x2000 | N)
#define SUBROUTINE_RETURN            0x00EE
#define GOTO(N)                      (0x1000 | N)
#define SKIP_ON_REG_EQUAL_VAL(R,V)   (0x3000 | R << 8 |  V)
#define SKIP_ON_REG_NOTEQ_VAL(R,V)   (0x4000 | R << 8 |  V)
#define SKIP_ON_REG_EQUAL_REG(R1,R2) (0x5000 | R1 << 8 | R2 << 4)
#define SET_REGISTER_VAL(R,V)        (0x6000 | R << 8 | V)
#define ADD_REGISTER_VAL(R,V)        (0x7000 | R << 8 | V)
#define CPY_REGISTER(R_DST, R_SRC)   (0x8000 | R_DST << 8 | R_SRC << 4)
#define REG_OREQ_REG(R_DST, R_SRC)   (0x8001 | R_DST << 8 | R_SRC << 4)
#define REG_ANDEQ_REG(R_DST, R_SRC)  (0x8002 | R_DST << 8 | R_SRC << 4)
#define REG_XOREQ_REG(R_DST, R_SRC)  (0x8003 | R_DST << 8 | R_SRC << 4)
#define ADD_REGISTER(R_DST, R_SRC)   (0x8004 | R_DST << 8 | R_SRC << 4)
#define SUB_REGISTER(R_DST, R_SRC)   (0x8005 | R_DST << 8 | R_SRC << 4)
#define SHIFT1_RIGHT_SAVE_LEAST(R)   (0x8006 | R << 8)
#define SUB_CHSIGN_REG(R_DST, R_SRC) (0x8007 | R_DST << 8 | R_SRC << 4)
#define SHIFT1_LEFT_SAVE_LEAST(R)    (0x800E | R << 8)
#define SET_I(V)                     (0xA000 | V)
#define SUM_V0_AND_JUMP(V)           (0xB000 | V)
#define REGSET_RANDOM(R, MAXRND)     (0xC000 | R << 8 | MAXRND)
#define SET_BCD(R)                   (0xF033 | R << 8)
#define CPY_V0VX_TO_MEM_I(R)         (0xF055 | R << 8)
#define CPY_MEM_I_TO_VOVX(R)         (0xF065 | R << 8)
// ------------------------------------------------------------------------------------

void run_tests();

#endif //CHIP8TEST_H
