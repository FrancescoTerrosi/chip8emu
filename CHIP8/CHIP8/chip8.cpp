#include "chip8.h"

unsigned short opcode;		// 2 byte per le operazioni macchina

unsigned char memory[4096];		// 4k per la memoria

unsigned char V[16];		// 16 byte per i registri della CPU

unsigned short I;			//	Index Register
unsigned short pc;			// Program Counter

unsigned char gfx[64 * 32];  // Graphics

unsigned char delay_timer;
unsigned char sound_timer;

unsigned short stack[16];
unsigned short sp;

void Chip8::initialize()
{
	pc = 0x200;
	opcode = 0;
	I = 0;
	sp = 0;

	memset(memory, 0, 4096);
	memset(V, 0, 16);
	memset(gfx, 0, 64 * 32);
	memset(stack, 0, 16);

	for (int i = 0; i < 80; ++i)
	{
		memory[i] = chip8_fontset[i];
	}



	return;
}

void Chip8::emulateCycle()
{
	// FETCH OPCODE

	// Devo combinare memory[pc] e memory[pc+1] --> ne leggo uno, shifto di 1 byte e faccio l'or con l'altro bit

	opcode = memory[pc] << 8 | memory[pc + 1];

	// DECODE OPCODE

	switch (opcode & 0xF000)
	{
		// DECODE DEGLI OPCODE

		case 0xA000:
			// EXECUTE
			I = opcode & 0x0FFF;
			pc += 2;
			break;

		default:
			printf("Opcode Error!!\n Code: 0x%X\n", opcode);

	}

	if (delay_timer > 0)
	{
		--delay_timer;
	}

	if (sound_timer > 0)
	{
		if (sound_timer == 1)
		{
			printf("BEEP");
		}
		--sound_timer;
	}
}

