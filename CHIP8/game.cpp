#include "chip8.h"

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
//#define ADD_REGISTER(R_DST, R_SRC)   (0x8004 | R_DST << 8 | R_SRC << 4)
//#define SUB_REGISTER(R_DST, R_SRC)   (0x8005 | R_DST << 8 | R_SRC << 4)
#define SHIFT1_RIGHT_SAVE_LEAST(R)   (0x8006 | R << 8)
//#define SUB_CHSIGN_REG(R_DST, R_SRC) (0x8007 R_DST << 8 | R_SRC << 4)
#define SHIFT1_LEFT_SAVE_LEAST(R)    (0x800E | R << 8)
#define SET_I(V)                     (0xA000 | V)
#define SUM_V0_AND_JUMP(V)           (0xB000 | V)
#define REGSET_RANDOM(R, MAXRND)     (0xC000 | R << 8 | MAXRND)
// ------------------------------------------------------------------------------------

void load_instruction(Chip8* hardware, unsigned short mem_address, unsigned short instruction)
{
    unsigned char instr_msb = ((instruction & 0xFF00) >> 8);
    unsigned char instr_lsb = (instruction & 0x0FF);

    hardware->memory[mem_address] = instr_msb;
    hardware->memory[mem_address + 1] = instr_lsb;
}



Chip8 myChip8;

int main(int argc, char** argv)
{
	//setupGraphics();
	//setupInput();
    myChip8.initialize();

    // ------------------------------------ CARICAMENTO PROGRAMMA TEST ------------------------------------
    unsigned short pc_baseaddr = myChip8.pc;
    load_instruction(&myChip8, pc_baseaddr + 0x00, CLEAR_SCREEN);
    load_instruction(&myChip8, pc_baseaddr + 0x02, SET_REGISTER_VAL(0x07, 0x02));
    load_instruction(&myChip8, pc_baseaddr + 0x04, SKIP_ON_REG_EQUAL_VAL(0x07, 0x02));
    //pc_baseaddr + 0x06 deve essere skippato
    load_instruction(&myChip8, pc_baseaddr + 0x08, SET_REGISTER_VAL(0x08, 0x02));
    load_instruction(&myChip8, pc_baseaddr + 0x0A, SKIP_ON_REG_EQUAL_REG(0x07, 0x08));
    //pc_baseaddr + 0x0C deve essere skippato
    load_instruction(&myChip8, pc_baseaddr + 0x0E, ADD_REGISTER_VAL(0x07, 0x02));
    load_instruction(&myChip8, pc_baseaddr + 0x10, SKIP_ON_REG_EQUAL_VAL(0x07, 0x09));
    //pc_baseaddr + 0x12 deve essere eseguito
    load_instruction(&myChip8, pc_baseaddr + 0x12, CPY_REGISTER(0x03, 0x07));
    load_instruction(&myChip8, pc_baseaddr + 0x14, REG_OREQ_REG(0x03, 0x03));
    load_instruction(&myChip8, pc_baseaddr + 0x16, REG_OREQ_REG(0x03, 0x08));
    load_instruction(&myChip8, pc_baseaddr + 0x18, REG_ANDEQ_REG(0x03, 0x08));
    load_instruction(&myChip8, pc_baseaddr + 0x1A, REG_XOREQ_REG(0x03, 0x08));
    load_instruction(&myChip8, pc_baseaddr + 0x1C, SHIFT1_RIGHT_SAVE_LEAST(0x08));
    load_instruction(&myChip8, pc_baseaddr + 0x1E, SHIFT1_LEFT_SAVE_LEAST(0x08));
    load_instruction(&myChip8, pc_baseaddr + 0x20, SET_I(0x09));
    load_instruction(&myChip8, pc_baseaddr + 0x22, SUM_V0_AND_JUMP(pc_baseaddr + 0x24));
    load_instruction(&myChip8, pc_baseaddr + 0x24, REGSET_RANDOM(0x04, 0x64));
    load_instruction(&myChip8, pc_baseaddr + 0x26, SUBROUTINE_CALL(pc_baseaddr + 0x64));

        // ------------ CARICAMENTO SUBROUTINE TEST A INDIRIZZO pc_baseaddr + 100 ------------
        load_instruction(&myChip8, pc_baseaddr + 0x64, CPY_REGISTER(0x05, 0x07));
        load_instruction(&myChip8, pc_baseaddr + 0x64 + 0x02, SET_REGISTER_VAL(0x07, 0x07));
        load_instruction(&myChip8, pc_baseaddr + 0x64 + 0x04, SUBROUTINE_RETURN);
        // -----------------------------------------------------------------------------------

    load_instruction(&myChip8, pc_baseaddr + 0x28, SKIP_ON_REG_NOTEQ_VAL(0x07, 0x07));
    //pc_baseaddr + 0x2A non deve essere skippato
    load_instruction(&myChip8, pc_baseaddr + 0x2A, GOTO(pc_baseaddr));
    // ----------------------------------------------------------------------------------------------------
	//myChip8.loadGame("pong");

	while (1)
	{
        printf("%s\n", "Start CPU Cycle");

		//emulate one cycle
        myChip8.emulateCycle();

		// if draw flag, update screen
		if (myChip8.drawFlag)
		{
			//drawGraphics();
		}

		// store keys press state (press-release)
		//myChip8.setKeys();

        printf("%s\n", "End CPU Cycle");
        getchar();
	}

	return 0;
}
