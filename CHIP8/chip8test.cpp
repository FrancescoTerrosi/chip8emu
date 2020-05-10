#include "chip8.h"
#include <stdio.h>
#include <vector>

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

void load_instruction(Chip8* hardware, unsigned short mem_address, unsigned short instruction)
{
    unsigned char instr_msb = ((instruction & 0xFF00) >> 8);
    unsigned char instr_lsb = (instruction & 0x0FF);

    hardware->memory[mem_address] = instr_msb;
    hardware->memory[mem_address + 1] = instr_lsb;
}

void load_test_program(Chip8* hardware)
{
    // ------------------------------------ CARICAMENTO PROGRAMMA TEST ------------------------------------
    unsigned short pc_baseaddr = hardware->pc;
    load_instruction(hardware, pc_baseaddr + 0x00, CLEAR_SCREEN);
    load_instruction(hardware, pc_baseaddr + 0x02, SET_REGISTER_VAL(0x07, 0x02));
    load_instruction(hardware, pc_baseaddr + 0x04, SKIP_ON_REG_EQUAL_VAL(0x07, 0x02));
    //pc_baseaddr + 0x06 deve essere skippato
    load_instruction(hardware, pc_baseaddr + 0x08, SET_REGISTER_VAL(0x08, 0x02));
    load_instruction(hardware, pc_baseaddr + 0x0A, SKIP_ON_REG_EQUAL_REG(0x07, 0x08));
    //pc_baseaddr + 0x0C deve essere skippato
    load_instruction(hardware, pc_baseaddr + 0x0E, ADD_REGISTER_VAL(0x07, 0x02));
    load_instruction(hardware, pc_baseaddr + 0x10, SKIP_ON_REG_EQUAL_VAL(0x07, 0x09));
    //pc_baseaddr + 0x12 deve essere eseguito
    load_instruction(hardware, pc_baseaddr + 0x12, CPY_REGISTER(0x03, 0x07));
    load_instruction(hardware, pc_baseaddr + 0x14, REG_OREQ_REG(0x03, 0x03));
    load_instruction(hardware, pc_baseaddr + 0x16, REG_OREQ_REG(0x03, 0x08));
    load_instruction(hardware, pc_baseaddr + 0x18, REG_ANDEQ_REG(0x03, 0x08));
    load_instruction(hardware, pc_baseaddr + 0x1A, REG_XOREQ_REG(0x03, 0x08));
    load_instruction(hardware, pc_baseaddr + 0x1C, SHIFT1_RIGHT_SAVE_LEAST(0x08));
    load_instruction(hardware, pc_baseaddr + 0x1E, SHIFT1_LEFT_SAVE_LEAST(0x08));
    load_instruction(hardware, pc_baseaddr + 0x20, SET_I(0x09));
    load_instruction(hardware, pc_baseaddr + 0x22, SUM_V0_AND_JUMP(pc_baseaddr + 0x24));
    load_instruction(hardware, pc_baseaddr + 0x24, REGSET_RANDOM(0x04, 0x64));
    load_instruction(hardware, pc_baseaddr + 0x26, SUBROUTINE_CALL(pc_baseaddr + 0x64));
        // ------------ CARICAMENTO SUBROUTINE TEST A INDIRIZZO pc_baseaddr + 100 ------------
        load_instruction(hardware, pc_baseaddr + 0x64, CPY_REGISTER(0x05, 0x07));
        load_instruction(hardware, pc_baseaddr + 0x64 + 0x02, SET_REGISTER_VAL(0x07, 0x07));
        load_instruction(hardware, pc_baseaddr + 0x64 + 0x04, SUBROUTINE_RETURN);
        // -----------------------------------------------------------------------------------
    load_instruction(hardware, pc_baseaddr + 0x28, SKIP_ON_REG_NOTEQ_VAL(0x07, 0x07));
    //pc_baseaddr + 0x2A non deve essere skippato
    load_instruction(hardware, pc_baseaddr + 0x2A, GOTO(pc_baseaddr));
    // ----------------------------------------------------------------------------------------------------
}


Chip8 chip8;

void setup()
{
    chip8.initialize();
}

bool testClearScren()
{
    chip8.gfx[10][20] = 0x01;
    unsigned short initpc = chip8.pc;

    load_instruction(&chip8, initpc, CLEAR_SCREEN);

    unsigned short expected_instruction = CLEAR_SCREEN;
    unsigned short expected_opcode = CLEAR_SCREEN & 0xF000;
    unsigned short expected_kk = CLEAR_SCREEN & 0x00FF;

    chip8.emulateCycle();

    //TESTO FETCH
    bool fetchOk = (chip8.instruction == expected_instruction) && (chip8.opcode == expected_opcode) && (chip8.kk == expected_kk);
    if(!fetchOk)
    {
        printf("%s\n", "testClearScreen failed: wrong fetch");
        return false;
    }

    //TESTO LA EXECUTE
    if(!chip8.drawFlag)
    {
        printf("%s\n", "testClearScren failed: drawFlag is not true");
        return false;
    }
    if(chip8.pc != (initpc + 0x02))
    {
        printf("%s\n", "testClearScren failed: pc was not incremented");
    }
    for(int i = 0; i < GMEM_ROWS; i++)
    {
        for(int j = 0; j < GMEM_COLS; j++)
        {
            if(chip8.gfx[i][j] != 0x00)
            {
                printf("%s\n", "testClearScren failed: screen is not cleared");
                return false;
            }
        }
    }

    return true;
}

bool testGoTo()
{
    unsigned short expected_addr = chip8.pc + 0x64;
    load_instruction(&chip8, chip8.pc, GOTO(expected_addr)); //deve essere una GOTO 512 + 100 = 612

    unsigned short expected_instruction = GOTO(expected_addr);
    unsigned short expected_opcode = GOTO(expected_addr) & 0xF000;

    chip8.emulateCycle();

    bool fetchOk = (chip8.instruction == expected_instruction) && (chip8.opcode == expected_opcode) && (chip8.nnn == expected_addr);
    if(!fetchOk)
    {
        printf("%s\n", "testGoTo failed: wrong fetch");
        return false;
    }

    bool executeOk = (chip8.pc == expected_addr);
    if(!executeOk)
    {
        printf("%s\n", "testGoTo failed: wrong execute");
        return false;
    }

    return true;
}

bool testCallSubroutine()
{
    unsigned short subroutine_addr = chip8.pc + 0x64;
    load_instruction(&chip8, chip8.pc, SUBROUTINE_CALL(subroutine_addr));

    unsigned short expected_instruction = SUBROUTINE_CALL(subroutine_addr);
    unsigned short expected_opcode = SUBROUTINE_CALL(subroutine_addr) & 0xF000;
    unsigned short expected_returnaddr = chip8.pc + 2;

    chip8.emulateCycle();

    bool fetchOk = (chip8.instruction == expected_instruction) && (chip8.opcode == expected_opcode) && (chip8.nnn == subroutine_addr);
    if(!fetchOk)
    {
        printf("%s\n", "testCallSubroutine failed: wrong fetch");
        return false;
    }

    bool executeOk = (chip8.pc == subroutine_addr) && (chip8.stack[chip8.sp - 1] == expected_returnaddr);
    if(!executeOk)
    {
        printf("%s\n", "testCallSubroutine failed: wrong execute");
        return false;
    }

    return true;
}

bool testSubroutineReturn()
{
    unsigned short expected_returnaddr = 0x100;
    chip8.stack[0] = expected_returnaddr;
    chip8.sp = 1;
    load_instruction(&chip8, chip8.pc, SUBROUTINE_RETURN);

    unsigned short expected_instruction = SUBROUTINE_RETURN;
    unsigned short expected_opcode = SUBROUTINE_RETURN & 0xF000;
    unsigned short expected_kk = SUBROUTINE_RETURN & 0x00FF;

    chip8.emulateCycle();

    bool fetchOk = (chip8.instruction) == expected_instruction && (chip8.opcode == expected_opcode) && (chip8.kk == expected_kk);
    if(!fetchOk)
    {
        printf("%s\n", "testSubroutineReturn failed: wrong fetch");
        return false;
    }

    bool executeOk = (chip8.pc == expected_returnaddr) && (chip8.sp == 0x00);
    if(!executeOk)
    {
        printf("%s\n", "testSubroutineReturn failed: wrong execute");
        return false;
    }

    return true;
}

bool testSkipIfRegisterEqualValueWhenComparisonIsTrue()
{
    unsigned char value = 0x17;
    unsigned char reg = 0x05;
    chip8.V[reg] = value;
    load_instruction(&chip8, chip8.pc, SKIP_ON_REG_EQUAL_VAL(reg, value));

    unsigned short expected_instruction = SKIP_ON_REG_EQUAL_VAL(reg, value);
    unsigned short expected_opcode = SKIP_ON_REG_EQUAL_VAL(reg, value) & 0xF000;
    unsigned short expected_pc = chip8.pc + 4;
    chip8.emulateCycle();

    bool fetchOk = (chip8.instruction == expected_instruction) && (chip8.opcode == expected_opcode) && (chip8.x == reg) && (chip8.kk == value);
    if(!fetchOk)
    {
        printf("%s\n", "testSkipIfRegisterEqualValueWhenComparisonIsTrue failed: wrong fetch");
        return false;
    }

    bool executeOk = (chip8.pc == expected_pc);
    if(!executeOk)
    {
        printf("%s\n", "testSkipIfRegisterEqualValueWhenComparisonIsTrue failed: wrong execute");
        return false;
    }

    return true;
}

bool testSkipIfRegisterEqualValueWhenComparisonIsFalse()
{
    unsigned char value = 0x17;
    unsigned char reg = 0x05;
    chip8.V[reg] = 0x00;
    load_instruction(&chip8, chip8.pc, SKIP_ON_REG_EQUAL_VAL(reg, value));

    unsigned short expected_instruction = SKIP_ON_REG_EQUAL_VAL(reg, value);
    unsigned short expected_opcode = SKIP_ON_REG_EQUAL_VAL(reg, value) & 0xF000;
    unsigned short expected_pc = chip8.pc + 2;
    chip8.emulateCycle();

    bool fetchOk = (chip8.instruction == expected_instruction) && (chip8.opcode == expected_opcode) && (chip8.x == reg) && (chip8.kk == value);
    if(!fetchOk)
    {
        printf("%s\n", "testSkipIfRegisterEqualValueWhenComparisonIsFalse failed: wrong fetch");
        return false;
    }

    bool executeOk = (chip8.pc == expected_pc);
    if(!executeOk)
    {
        printf("%s\n", "testSkipIfRegisterEqualValueWhenComparisonIsFalse failed: wrong execute");
        return false;
    }

    return true;
}

bool testSkipIfRegisterNotEqualValueWhenComparisonIsTrue()
{
    unsigned char value = 0x17;
    unsigned char reg = 0x05;
    chip8.V[reg] = 0x00;
    load_instruction(&chip8, chip8.pc, SKIP_ON_REG_NOTEQ_VAL(reg, value));

    unsigned short expected_instruction = SKIP_ON_REG_NOTEQ_VAL(reg, value);
    unsigned short expected_opcode = SKIP_ON_REG_NOTEQ_VAL(reg, value) & 0xF000;
    unsigned short expected_pc = chip8.pc + 4;
    chip8.emulateCycle();

    bool fetchOk = (chip8.instruction == expected_instruction) && (chip8.opcode == expected_opcode) && (chip8.x == reg) && (chip8.kk == value);
    if(!fetchOk)
    {
        printf("%s\n", "testSkipIfRegisterNotEqualValueWhenComparisonIsTrue failed: wrong fetch");
        return false;
    }

    bool executeOk = (chip8.pc == expected_pc);
    if(!executeOk)
    {
        printf("%s\n", "testSkipIfRegisterNotEqualValueWhenComparisonIsTrue failed: wrong execute");
        return false;
    }

    return true;
}

bool testSkipIfRegisterNotEqualValueWhenComparisonIsFalse()
{
    unsigned char value = 0x17;
    unsigned char reg = 0x05;
    chip8.V[reg] = value;
    load_instruction(&chip8, chip8.pc, SKIP_ON_REG_NOTEQ_VAL(reg, value));

    unsigned short expected_instruction = SKIP_ON_REG_NOTEQ_VAL(reg, value);
    unsigned short expected_opcode = SKIP_ON_REG_NOTEQ_VAL(reg, value) & 0xF000;
    unsigned short expected_pc = chip8.pc + 2;
    chip8.emulateCycle();

    bool fetchOk = (chip8.instruction == expected_instruction) && (chip8.opcode == expected_opcode) && (chip8.x == reg) && (chip8.kk == value);
    if(!fetchOk)
    {
        printf("%s\n", "testSkipIfRegisterNotEqualValueWhenComparisonIsFalse failed: wrong fetch");
        return false;
    }

    bool executeOk = (chip8.pc == expected_pc);
    if(!executeOk)
    {
        printf("%s\n", "testSkipIfRegisterNotEqualValueWhenComparisonIsFalse failed: wrong execute");
        return false;
    }

    return true;
}

bool testSkipIfRegisterEqualRegisterWhenComparisonIsTrue()
{
    unsigned char value = 0x17;
    unsigned char reg1 = 0x05;
    unsigned char reg2 = 0x0A;
    chip8.V[reg1] = value;
    chip8.V[reg2] = value;
    load_instruction(&chip8, chip8.pc, SKIP_ON_REG_EQUAL_REG(reg1, reg2));

    unsigned short expected_instruction = SKIP_ON_REG_EQUAL_REG(reg1, reg2);
    unsigned short expected_opcode = SKIP_ON_REG_EQUAL_REG(reg1, reg2) & 0xF000;
    unsigned short expected_pc = chip8.pc + 4;
    chip8.emulateCycle();

    bool fetchOk = (chip8.instruction == expected_instruction) && (chip8.opcode == expected_opcode) && (chip8.x == reg1) && (chip8.y == reg2);
    if(!fetchOk)
    {
        printf("%s\n", "testSkipIfRegisterEqualRegisterWhenComparisonIsTrue failed: wrong fetch");
        return false;
    }

    bool executeOk = (chip8.pc == expected_pc);
    if(!executeOk)
    {
        printf("%s\n", "testSkipIfRegisterEqualRegisterWhenComparisonIsTrue failed: wrong execute");
        return false;
    }

    return true;
}

bool testSkipIfRegisterEqualRegisterWhenComparisonIsFalse()
{
    unsigned char value = 0x17;
    unsigned char reg1 = 0x05;
    unsigned char reg2 = 0x0A;
    chip8.V[reg1] = value + 1;
    chip8.V[reg2] = value - 1;
    load_instruction(&chip8, chip8.pc, SKIP_ON_REG_EQUAL_REG(reg1, reg2));

    unsigned short expected_instruction = SKIP_ON_REG_EQUAL_REG(reg1, reg2);
    unsigned short expected_opcode = SKIP_ON_REG_EQUAL_REG(reg1, reg2) & 0xF000;
    unsigned short expected_pc = chip8.pc + 2;
    chip8.emulateCycle();

    bool fetchOk = (chip8.instruction == expected_instruction) && (chip8.opcode == expected_opcode) && (chip8.x == reg1) && (chip8.y == reg2);
    if(!fetchOk)
    {
        printf("%s\n", "testSkipIfRegisterEqualRegisterWhenComparisonIsFalse failed: wrong fetch");
        return false;
    }

    bool executeOk = (chip8.pc == expected_pc);
    if(!executeOk)
    {
        printf("%s\n", "testSkipIfRegisterEqualRegisterWhenComparisonIsFalse failed: wrong execute");
        return false;
    }

    return true;
}

bool testSetRegister()
{
    unsigned char value = 0x02;
    unsigned char reg = 0x03;
    unsigned short initpc = chip8.pc;

    load_instruction(&chip8, initpc, SET_REGISTER_VAL(reg, value));

    unsigned short expected_instruction = SET_REGISTER_VAL(reg, value);
    unsigned short expected_opcode = SET_REGISTER_VAL(reg, value) & 0xF000;

    chip8.emulateCycle();

    bool fetchOk = (chip8.instruction == expected_instruction) && (chip8.opcode == expected_opcode) && (chip8.x == reg) && (chip8.kk == value);
    if(!fetchOk)
    {
        printf("%s\n", "testSetRegister failed: wrong fetch");
        return false;
    }

    bool executeOk = (chip8.V[reg] == value) && (chip8.pc == (initpc + 0x02));
    if(!executeOk)
    {
        printf("%s\n", "testSetRegister failed: wrong execute");
        return false;
    }

    return true;
}

bool testAddRegister()
{
    unsigned char value1 = 0x02;
    unsigned char value2 = 0x03;
    unsigned char sum = value1 + value2;
    unsigned char reg = 0x03;
    unsigned short initpc = chip8.pc;
    chip8.V[reg] = value1;

    load_instruction(&chip8, initpc, ADD_REGISTER_VAL(reg, value2));

    unsigned short expected_instruction = ADD_REGISTER_VAL(reg, value2);
    unsigned short expected_opcode = ADD_REGISTER_VAL(reg, value2) & 0xF000;

    chip8.emulateCycle();

    bool fetchOk = (chip8.instruction == expected_instruction) && (chip8.opcode == expected_opcode) && (chip8.x == reg) && (chip8.kk == value2);
    if(!fetchOk)
    {
        printf("%s\n", "testAddtRegister failed: wrong fetch");
        return false;
    }

    bool executeOk = (chip8.V[reg] == sum) && (chip8.pc == initpc + 0x02);
    if(!executeOk)
    {
        printf("%s\n", "testAddRegister failed: wrong execute");
        return false;
    }

    return true;
}

bool testCpyReg(unsigned char r1, unsigned char r2, unsigned short instr)
{
    chip8.V[r2] = 0x02;
    unsigned short initpc = chip8.pc;
    load_instruction(&chip8, initpc, instr);
    chip8.emulateCycle();

    bool fetchOk = (chip8.instruction == instr) && (chip8.opcode == (instr & 0xF000)) && (chip8.x == r1) && (chip8.y == r2) && (chip8.n == 0x0);
    if(!fetchOk)
    {
        printf("%s\n", "testCpyReg failed: wrong fetch");
        return false;
    }

    bool executeOk = (chip8.V[r1] == chip8.V[r2]) && (chip8.pc == (initpc + 0x02));
    if(!executeOk)
    {
        printf("%s\n", "testCpyReg failed: wrong execute");
        return false;
    }

    return true;
}

bool testAndReg(unsigned char r1, unsigned char r2, unsigned short instr)
{
    unsigned short initpc = chip8.pc;
    chip8.V[r1] = 0x02;
    chip8.V[r2] = 0x03;
    load_instruction(&chip8, initpc, instr);
    chip8.emulateCycle();

    bool fetchOk = (chip8.instruction == instr) && (chip8.opcode == (instr & 0xF000)) && (chip8.x == r1) && (chip8.y == r2) && (chip8.n == 0x2);
    if(!fetchOk)
    {
        printf("%s\n", "testAndReg failed: wrong fetch");
        return false;
    }

    bool executeOk = (chip8.V[r1] == (0x02 & chip8.V[r2])) && (chip8.pc == initpc + 0x02);
    if(!executeOk)
    {
        printf("%s\n", "testAndReg failed: wrong execute");
        return false;
    }

    return true;
}

bool testOrReg(unsigned char r1, unsigned char r2, unsigned short instr)
{
    unsigned short initpc = chip8.pc;
    chip8.V[r1] = 0x04;
    chip8.V[r2] = 0x07;

    load_instruction(&chip8, initpc, instr);
    chip8.emulateCycle();

    bool fetchOk = (chip8.instruction == instr) && (chip8.opcode == (instr & 0xF000)) && (chip8.x == r1) && (chip8.y == r2) && (chip8.n == 0x1);
    if(!fetchOk)
    {
        printf("%s\n", "testOrReg failed: wrong fetch");
        return false;
    }

    bool executeOk = (chip8.V[r1] == (0x04 | chip8.V[r2])) && (chip8.pc == initpc + 0x02);

    if(!executeOk)
    {
        printf("%s\n", "testOrReg failed: wrong execute");
        return false;
    }

    return true;
}

bool testXorReg(unsigned char r1, unsigned char r2, unsigned short instr)
{
    unsigned short initpc = chip8.pc;
    chip8.V[r1] = 0x04;
    chip8.V[r2] = 0x07;

    load_instruction(&chip8, initpc, instr);
    chip8.emulateCycle();

    bool fetchOk = (chip8.instruction == instr) && (chip8.opcode == (instr & 0xF000)) && (chip8.x == r1) && (chip8.y == r2) && (chip8.n == 0x3);
    if(!fetchOk)
    {
        printf("%s\n", "testXorReg failed: wrong fetch");
        return false;
    }

    bool executeOk = (chip8.V[r1] == (0x04 ^ chip8.V[r2])) && (chip8.pc == initpc + 0x02);
    if(!executeOk)
    {
        printf("%s\n", "testXorReg failed: wrong execute");
        return false;
    }

    return true;
}

bool testPlusWhenNoCarry(unsigned char r1, unsigned char r2, unsigned short instr)
{
    unsigned short initpc = chip8.pc;
    chip8.V[r1] = 0x04;
    chip8.V[r2] = 0x17;

    load_instruction(&chip8, initpc, instr);
    chip8.emulateCycle();

    bool fetchOk = (chip8.instruction == instr) && (chip8.opcode == (instr & 0xF000)) && (chip8.x == r1) && (chip8.y == r2) && (chip8.n == 0x4);
    if(!fetchOk)
    {
        printf("%s\n", "testPlusWhenNoCarry failed: wrong fetch");
        return false;
    }

    bool executeOk = (chip8.V[r1] == (chip8.V[r2] + 0x04)) && (chip8.pc == initpc + 0x02);
    if(!executeOk)
    {
        printf("%s\n", "testPlusWhenNoCarry failed: wrong execute");
        return false;
    }
    return true;
}

bool testPlusWhenCarry(unsigned char r1, unsigned char r2, unsigned short instr)
{
    unsigned short initpc = chip8.pc;
    unsigned char val1 = 150;
    unsigned char val2 = 200;
    unsigned char expected_sum = val1 + val2;
    chip8.V[r1] = val1;
    chip8.V[r2] = val2;

    load_instruction(&chip8, initpc, instr);
    chip8.emulateCycle();

    bool fetchOk = (chip8.instruction == instr) && (chip8.opcode == (instr & 0xF000)) && (chip8.x == r1) && (chip8.y == r2) && (chip8.n == 0x4);
    if(!fetchOk)
    {
        printf("%s\n", "testPlusWhenCarry failed: wrong fetch");
        return false;
    }

    bool executeOk = (chip8.V[r1] == expected_sum) && (chip8.V[0xF] == 1) && (chip8.pc == initpc + 0x02);
    if(!executeOk)
    {
        printf("%s\n", "testPlusWhenCarry failed: wrong execute");
        return false;
    }

    return true;
}

bool testMinusWhenNoCarry(unsigned char r1, unsigned char r2, unsigned short instr)
{
    unsigned short initpc = chip8.pc;
    chip8.V[r1] = 0x50;
    chip8.V[r2] = 0x17;

    load_instruction(&chip8, initpc, instr);
    chip8.emulateCycle();

    bool fetchOk = (chip8.instruction == instr) && (chip8.opcode == (instr & 0xF000)) && (chip8.x == r1) && (chip8.y == r2) && (chip8.n == 0x5);
    if(!fetchOk)
    {
        printf("%s\n", "testMinusWhenNoCarry failed: wrong fetch");
        return false;
    }

    bool executeOk = (chip8.V[r1] == 0x50 - chip8.V[r2]) && (chip8.V[0xF] == 0) && (chip8.pc == initpc + 0x02);
    if(!executeOk)
    {
        printf("%s\n", "testMinusWhenNoCarry failed: wrong execute");
        return false;
    }

    return true;
}

bool testMinusWhenCarry(unsigned char r1, unsigned char r2, unsigned short instr)
{
    unsigned short initpc = chip8.pc;
    chip8.V[r2] = 0x70;
    chip8.V[r1] = 0x50;
    unsigned char expected_diff = chip8.V[r1] - chip8.V[r2];

    load_instruction(&chip8, initpc, instr);
    chip8.emulateCycle();

    bool fetchOk = (chip8.instruction == instr) && (chip8.opcode == (instr & 0xF000)) && (chip8.x == r1) && (chip8.y == r2) && (chip8.n == 0x5);
    if(!fetchOk)
    {
        printf("%s\n", "testMinusWhenCarry failed: wrong fetch");
        return false;
    }

    bool executeOk = (chip8.V[r1] == expected_diff) && (chip8.V[0xF] == 1) && (chip8.pc == initpc + 0x02);
    if(!executeOk)
    {
        printf("%s\n", "testMinusWhenCarry failed: wrong execute");
        return false;
    }

    return true;
}

bool testMinusChsignWhenNoCarry(unsigned char r1, unsigned char r2, unsigned short instr)
{
    unsigned short initpc = chip8.pc;
    chip8.V[r2] = 0x70;
    chip8.V[r1] = 0x50;
    unsigned char expected_diff = chip8.V[r2] - chip8.V[r1];

    load_instruction(&chip8, initpc, instr);
    chip8.emulateCycle();

    bool fetchOk = (chip8.instruction == instr) && (chip8.opcode == (instr & 0xF000)) && (chip8.x == r1) && (chip8.y == r2) && (chip8.n == 0x7);
    if(!fetchOk)
    {
        printf("%s\n", "testMinusChsignWhenNoCarry failed: wrong fetch");
        return false;
    }

    bool executeOk = (chip8.V[r1] == expected_diff) && (chip8.V[0xF] == 0) && (chip8.pc == initpc + 0x02);
    if(!executeOk)
    {
        printf("%s\n", "testMinusChsignWhenNoCarry failed: wrong execute");
        return false;
    }

    return true;
}

bool testMinusChsignWhenCarry(unsigned char r1, unsigned char r2, unsigned short instr)
{
    unsigned short initpc = chip8.pc;
    chip8.V[r2] = 0x50;
    chip8.V[r1] = 0x70;
    unsigned char expected_diff = chip8.V[r2] - chip8.V[r1];

    load_instruction(&chip8, initpc, instr);
    chip8.emulateCycle();

    bool fetchOk = (chip8.instruction == instr) && (chip8.opcode == (instr & 0xF000)) && (chip8.x == r1) && (chip8.y == r2) && (chip8.n == 0x7);
    if(!fetchOk)
    {
        printf("%s\n", "testMinusChsignWhenCarry failed: wrong fetch");
        return false;
    }

    bool executeOk = (chip8.V[r1] == expected_diff) && (chip8.V[0xF] == 1) && (chip8.pc == initpc + 0x02);
    if(!executeOk)
    {
        printf("%s\n", "testMinusChsignWhenCarry failed: wrong execute");
        return false;
    }

    return true;
}

bool testShift1Left(unsigned char reg, unsigned short instr)
{
    unsigned short initpc = chip8.pc;
    chip8.V[reg] = 0x50;

    load_instruction(&chip8, initpc, instr);
    chip8.emulateCycle();

    bool fetchOk = (chip8.instruction == instr) && (chip8.opcode == (instr & 0xF000)) && (chip8.x == reg) && (chip8.n == 0xE);
    if(!fetchOk)
    {
        printf("%s\n", "testShift1Left failed: wrong fetch");
        return false;
    }

    bool executeOk = (chip8.V[reg] == 0x50 << 1) && (chip8.V[0xF] == (0x50 & 0x1)) && (chip8.pc == initpc + 0x02);
    if(!executeOk)
    {
        printf("%s\n", "testShift1Left failed: wrong execute");
        return false;
    }

    return true;
}

bool testShift1Right(unsigned char reg, unsigned short instr)
{
    unsigned short initpc = chip8.pc;
    chip8.V[reg] = 0x47;

    load_instruction(&chip8, initpc, instr);
    chip8.emulateCycle();

    bool fetchOk = (chip8.instruction == instr) && (chip8.opcode == (instr & 0xF000)) && (chip8.x == reg) && (chip8.n == 0x6);
    if(!fetchOk)
    {
        printf("%s\n", "testShift1Right failed: wrong fetch");
        return false;
    }

    bool executeOk = (chip8.V[reg] == 0x47 >> 1) && (chip8.V[0xF] == (0x47 & 0x1)) && (chip8.pc == initpc + 0x02);
    if(!executeOk)
    {
        printf("%s\n", "testShift1Right failed: wrong execute");
        return false;
    }

    return true;
}


bool testMathOperations()
{
    unsigned char reg1 = 0x0B;
    unsigned char reg2 = 0x0C;
    unsigned short SET_V11_EQUAL_V12 = CPY_REGISTER(reg1, reg2);
    unsigned short V11_ANDEQ_V12 = REG_ANDEQ_REG(reg1, reg2);
    unsigned short V11_OREQ_V12 = REG_OREQ_REG(reg1, reg2);
    unsigned short V11_XOREQ_V12 = REG_XOREQ_REG(reg1, reg2);
    unsigned short V11_PLUSEQ_V12 = ADD_REGISTER(reg1, reg2);
    unsigned short V11_MINUSEQ_V12 = SUB_REGISTER(reg1, reg2);
    unsigned short V11_MINUSEQ_V12_CHS = SUB_CHSIGN_REG(reg1, reg2);
    unsigned short V11_SHIFT1RIGHT = SHIFT1_RIGHT_SAVE_LEAST(reg1);
    unsigned short V11_SHIFT1LEFT = SHIFT1_LEFT_SAVE_LEAST(reg1);

    setup();
    bool testSet = testCpyReg(reg1, reg2, SET_V11_EQUAL_V12);
    setup();
    bool testAnd = testAndReg(reg1, reg2, V11_ANDEQ_V12);
    setup();
    bool testOr = testOrReg(reg1, reg2, V11_OREQ_V12);
    setup();
    bool testXor = testXorReg(reg1, reg2, V11_XOREQ_V12);
    setup();
    bool testPlus1 = testPlusWhenNoCarry(reg1, reg2, V11_PLUSEQ_V12);
    setup();
    bool testPlus2 = testPlusWhenCarry(reg1, reg2, V11_PLUSEQ_V12);
    setup();
    bool testMinus1 = testMinusWhenNoCarry(reg1, reg2, V11_MINUSEQ_V12);
    setup();
    bool testMinus2 = testMinusWhenCarry(reg1, reg2, V11_MINUSEQ_V12);
    setup();
    bool testMinusChsign1 = testMinusChsignWhenNoCarry(reg1, reg2, V11_MINUSEQ_V12_CHS);
    setup();
    bool testMinusChsign2 = testMinusChsignWhenCarry(reg1, reg2, V11_MINUSEQ_V12_CHS);
    setup();
    bool testShiftLeft = testShift1Left(reg1, V11_SHIFT1LEFT);
    setup();
    bool testShiftRight = testShift1Right(reg1, V11_SHIFT1RIGHT);

    return testSet          &&
           testAnd          &&
           testOr           &&
           testXor          &&
           testPlus1        &&
           testPlus2        &&
           testMinus1       &&
           testMinus2       &&
           testMinusChsign1 &&
           testMinusChsign2 &&
           testShiftLeft    &&
           testShiftRight;
}

bool testRandom()
{
    unsigned short initpc = chip8.pc;
    unsigned char reg = 0x02;
    unsigned char maxrnd = 0x24;
    chip8.V[reg] = maxrnd + 0x1;

    load_instruction(&chip8, initpc, REGSET_RANDOM(reg, maxrnd));
    chip8.emulateCycle();

    bool fetchOk = (chip8.instruction == REGSET_RANDOM(reg, maxrnd)) && (chip8.opcode == (REGSET_RANDOM(reg, maxrnd) & 0xF000)) && (chip8.x == reg) && (chip8.kk == maxrnd);
    if(!fetchOk)
    {
        printf("%s\n", "testRandom failed: wrong fetch");
        return false;
    }

    bool executeOk = (chip8.V[reg] < maxrnd) && (chip8.pc == initpc + 0x02);
    if(!executeOk)
    {
        printf("%s\n", "testRandom failed: wrong execute");
        return false;
    }

    return true;
}

bool testSetI()
{
    unsigned short initpc = chip8.pc;
    unsigned short expected_i = 0x217;

    load_instruction(&chip8, initpc, SET_I(expected_i));
    chip8.emulateCycle();

    bool fetchOk = (chip8.instruction == SET_I(expected_i)) && (chip8.opcode == (SET_I(expected_i) & 0xF000)) && (chip8.nnn = expected_i);
    if(!fetchOk)
    {
        printf("%s\n", "testSetI failed: wrong fetch");
        return false;
    }

    bool executeOk = (chip8.I == expected_i) && (chip8.pc == initpc + 0x02);
    if(!executeOk)
    {
        printf("%s\n", "testSetI failed: wrong execute");
        return false;
    }

    return true;
}

bool testJumpFromV0()
{
    unsigned short offset = 0x317;
    chip8.V[0] = 0x17;

    load_instruction(&chip8, chip8.pc, SUM_V0_AND_JUMP(offset));
    chip8.emulateCycle();

    bool fetchOk = (chip8.instruction == SUM_V0_AND_JUMP(offset)) && (chip8.opcode == (SUM_V0_AND_JUMP(offset) & 0xF000)) && (chip8.nnn = offset);
    if(!fetchOk)
    {
        printf("%s\n", "testJumpFromV0 failed: wrong fetch");
        return false;
    }

    bool executeOk = chip8.pc == (chip8.V[0] + offset);
    if(!executeOk)
    {
        printf("%s\n", "testJumpFromV0 failed: wrong execute");
        return false;
    }

    return true;
}

bool testSetBcd()
{
    unsigned char reg = 0x07;
    unsigned short memdest = 0x500;
    unsigned short initpc = chip8.pc;

    chip8.V[reg] = 123;
    chip8.I = memdest;

    load_instruction(&chip8, chip8.pc, SET_BCD(reg));
    chip8.emulateCycle();

    bool fetchOk = (chip8.instruction == SET_BCD(reg)) && (chip8.opcode == (SET_BCD(reg) & 0xF000) && (chip8.x == reg));
    if(!fetchOk)
    {
        printf("%s\n", "testSetBcd failed: wrong fetch");
        return false;
    }

    bool executeOk = (chip8.memory[memdest] == 1) && (chip8.memory[memdest + 1] == 2) && (chip8.memory[memdest + 2] == 3) && (chip8.pc == initpc + 0x02);
    if(!executeOk)
    {
        printf("%s\n", "testSetBcd failed: wrong execute");
        return false;
    }

    return true;
}

bool testCpyVoVxToMemI()
{
    unsigned char reg = 0x02;
    unsigned short memdest = 0x400;
    unsigned short initpc = chip8.pc;

    chip8.V[0x00] = 0x07;
    chip8.V[0x01] = 0x08;
    chip8.V[reg]  = 0x09;
    chip8.I = memdest;

    load_instruction(&chip8, chip8.pc, CPY_V0VX_TO_MEM_I(reg));
    chip8.emulateCycle();

    bool fetchOk = (chip8.instruction == CPY_V0VX_TO_MEM_I(reg)) && (chip8.opcode == (CPY_V0VX_TO_MEM_I(reg) & 0xF000) && (chip8.x == reg));
    if(!fetchOk)
    {
        printf("%s\n", "testCpyVoVxToMemI failed: wrong fetch");
        return false;
    }

    bool executeOk = (chip8.memory[memdest] == 0x07) && (chip8.memory[memdest + 1] == 0x08) && (chip8.memory[memdest + 2] == 0x09);
    executeOk = executeOk && (chip8.I == memdest + reg + 1) && (chip8.pc == initpc + 0x02);
    if(!executeOk)
    {
        printf("%s\n", "testCpyVoVxToMemI failed: wrong execute");
        return false;
    }

    return true;
}

bool testCpyMemIToV0VX()
{
    unsigned char reg = 0x02;
    unsigned short memdest = 0x400;
    unsigned short initpc = chip8.pc;

    chip8.I = memdest;
    chip8.memory[memdest] = 0x03;
    chip8.memory[memdest + 1] = 0x02;
    chip8.memory[memdest + 2] = 0x01;

    load_instruction(&chip8, chip8.pc, CPY_MEM_I_TO_VOVX(reg));
    chip8.emulateCycle();

    bool fetchOk = (chip8.instruction == CPY_MEM_I_TO_VOVX(reg)) && (chip8.opcode == (CPY_MEM_I_TO_VOVX(reg) & 0xF000) && (chip8.x == reg));
    if(!fetchOk)
    {
        printf("%s\n", "testCpyMemIToV0VX failed: wrong fetch");
        return false;
    }

    bool executeOk = (chip8.V[0x00] == 0x03) && (chip8.V[0x01] == 0x02) && (chip8.V[reg] == 0x01);
    executeOk = executeOk && (chip8.I == memdest + reg + 1) && (chip8.pc == initpc + 0x02);
    if(!executeOk)
    {
        printf("%s\n", "testCpyMemIToV0VX failed: wrong execute");
        return false;
    }

    return true;
}



void run_tests()
{
    std::vector<bool (*)()> testcases;
    testcases.push_back(&testClearScren);
    testcases.push_back(&testGoTo);
    testcases.push_back(&testCallSubroutine);
    testcases.push_back(&testSubroutineReturn);
    testcases.push_back(&testSkipIfRegisterEqualValueWhenComparisonIsTrue);
    testcases.push_back(&testSkipIfRegisterEqualValueWhenComparisonIsFalse);
    testcases.push_back(&testSkipIfRegisterNotEqualValueWhenComparisonIsTrue);
    testcases.push_back(&testSkipIfRegisterNotEqualValueWhenComparisonIsFalse);
    testcases.push_back(&testSkipIfRegisterEqualRegisterWhenComparisonIsTrue);
    testcases.push_back(&testSkipIfRegisterEqualRegisterWhenComparisonIsFalse);
    testcases.push_back(&testSetRegister);
    testcases.push_back(&testAddRegister);
    testcases.push_back(&testMathOperations);
    testcases.push_back(&testRandom);
    testcases.push_back(&testSetI);
    testcases.push_back(&testJumpFromV0);
    testcases.push_back(&testSetBcd);
    testcases.push_back(&testCpyVoVxToMemI);
    testcases.push_back(&testCpyMemIToV0VX);

    int nTests = testcases.size();
    int passed = 0;
    for(int i = 0; i < nTests; i++)
    {
        setup();
        bool pass = testcases[i]();

        if(pass)
        {
            passed += 1;
        }
        else
        {
            exit(0);
        }
    }

    float failureRate = (float)(nTests - passed) / (float) nTests;
    printf("\n\n-------------------------------- TEST REPORT --------------------------------\n"
           "Tests run: %d\nPassed: %d\nFailed: %d\nFailure rate: %f %%\n", nTests, passed, nTests - passed, failureRate*100.0);
    printf("%s\n\n","-----------------------------------------------------------------------------");
}

int main()
{
    run_tests();
}
