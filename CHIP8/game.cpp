#include "chip8.h"

// ---------------  MACRO CHE CONTENGONO LE OPERAZIONI IMPLEMENTATE  ------------------
#define CLEAR_SCREEN                 0x00E0
#define SUBROUTINE_CALL(N)           (0x2000 | (N & 0x0FFF))
#define SUBROUTINE_RETURN            0x00EE
#define GOTO(N)                      (0x1000 | (N & 0x0FFF))
#define SKIP_ON_REG_EQUAL_VAL(R,V)   (0x3000 | ((R & 0x0F00) | (V & 0x00FF)))
#define SKIP_ON_REG_NOTEQ_VAL(R,V)   (0x4000 | ((R & 0x0F00) | (V & 0x00FF)))
#define SKIP_ON_REG_EQUAL_REG(R1,R2) (0x5000 | ((R1 & 0x0F00) | (R2 & 0x00F0)))
#define SET_REGISTER_VAL(R,V)        (0x6000 | ((R & 0x0F00) | (V & 0x0FF)))
#define ADD_REGISTER_VAL(R,V)        (0x7000 | ((R & 0x0F00) | (V & 0x0FF)))
#define CPY_REGISTER(R_DST, R_SRC)   (0x8000 | ((R_DST & 0x0F00) | (R_SRC & 0x00F0)))
#define REG_OREQ_REG(R_DST, R_SRC)   (0x8001 | ((R_DST & 0x0F00) | (R_SRC & 0x00F0)))
#define REG_ANDEQ_REG(R_DST, R_SRC)  (0x8002 | ((R_DST & 0x0F00) | (R_SRC & 0x00F0)))
#define REG_XOREQ_REG(R_DST, R_SRC)  (0x8003 | ((R_DST & 0x0F00) | (R_SRC & 0x00F0)))
//#define ADD_REGISTER(R_DST, R_SRC)   (0x8004 | ((R_DST & 0x0F00) | (R_SRC & 0x00F0)))
//#define SUB_REGISTER(R_DST, R_SRC)   (0x8005 | ((R_DST & 0x0F00) | (R_SRC & 0x00F0)))
#define SHIFT1_RIGHT_SAVE_LEAST(R)   (0x8006 | (R & 0x0F00))
//#define SUB_CHSIGN_REG(R_DST, R_SRC) (0x8007 |((R_DST & 0x0F00) | (R_SRC & 0x00F0)))
#define SHIFT1_LEFT_SAVE_LEAST(R)    (0x800E | (R & 0x0F00))
#define SET_I(V)                     (0xA000 | (V & 0x0FFF))
#define SUM_V0_AND_JUMP(V)           (0xB000 | (V & 0x0FFF))
#define REGSET_RANDOM(R, MAXRND)     (0xC000 | ((R & 0x0F00) | (MAXRND & 0x00FF)))
// ------------------------------------------------------------------------------------

void load_instruction(Chip8* hardware, unsigned short mem_address, unsigned short instruction)
{
    unsigned char instr_msb = ((instruction >> 8) & 0x0FF);
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

    // ------------ CARICAMENTO PROGRAMMA TEST ------------
    unsigned short pc_baseaddr = myChip8.pc;
    load_instruction(&myChip8, pc_baseaddr + 0x00, CLEAR_SCREEN);
    load_instruction(&myChip8, pc_baseaddr + 0x02, CLEAR_SCREEN);
    load_instruction(&myChip8, pc_baseaddr + 0x04, CLEAR_SCREEN);
    load_instruction(&myChip8, pc_baseaddr + 0x06, CLEAR_SCREEN);
    load_instruction(&myChip8, pc_baseaddr + 0x08, CLEAR_SCREEN);
    load_instruction(&myChip8, pc_baseaddr + 0x0A, CLEAR_SCREEN);
    load_instruction(&myChip8, pc_baseaddr + 0x0C, CLEAR_SCREEN);
    load_instruction(&myChip8, pc_baseaddr + 0x0E, CLEAR_SCREEN);
    load_instruction(&myChip8, pc_baseaddr + 0x10, CLEAR_SCREEN);
    load_instruction(&myChip8, pc_baseaddr + 0x12, CLEAR_SCREEN);
    load_instruction(&myChip8, pc_baseaddr + 0x14, CLEAR_SCREEN);
    load_instruction(&myChip8, pc_baseaddr + 0x16, CLEAR_SCREEN);
    load_instruction(&myChip8, pc_baseaddr + 0x18, CLEAR_SCREEN);
    load_instruction(&myChip8, pc_baseaddr + 0x1A, CLEAR_SCREEN);
    load_instruction(&myChip8, pc_baseaddr + 0x1C, CLEAR_SCREEN);
    load_instruction(&myChip8, pc_baseaddr + 0x1E, GOTO(pc_baseaddr));
    // ----------------------------------------------------
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
