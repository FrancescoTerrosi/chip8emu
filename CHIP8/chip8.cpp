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

	// inizializzo a caso per ora poi capiremo

	delay_timer = 60;
	sound_timer = 60;

	return;
}

void Chip8::emulateCycle()
{
	// FETCH OPCODE

	// Devo combinare memory[pc] e memory[pc+1] --> ne leggo uno, shifto di 1 byte e faccio l'or con l'altro byte

	opcode = memory[pc] << 8 | memory[pc + 1];

	// DECODE OPCODE

	switch (opcode & 0xF000)
	{
		// DECODE DEGLI OPCODE

		// gestione 2 op che iniziano con 0x00
		case 0x0000:
			switch (opcode & 0xFF)
			{
				// clear screen
				case 0x00E0:
					pc += 2;
					break;

				// return from subroutine
				case 0x00EE:

					/*
					VADO A BRACCIO:
					*/

					--sp;
					pc = stack[sp];
					break;
				default:
					printf("Unknown opcode 0x%X\n", opcode);
					fflush(stdout);
			}
			break;


		// 0x1NNN	goto NNN
		case 0x1000:
			pc = opcode & 0x0FFF;
			break;

		// 0x2NNN   calls subroutine at NNN
		case 0x2000:
			stack[sp] = pc;
			++sp;
			pc = opcode & 0x0FFF;
			break;

		// 0x3XNN	if V[X] == NN, skip next instruction
		case 0x3000:
			if (V[opcode & 0x0F00] == (opcode & 0x00FF))
			{
				pc += 4;
			}
			break;

			// 0x4XNN	if V[X] != NN, skip next instruction
		case 0x4000:
			if (V[opcode & 0x0F00] != (opcode & 0x00FF))
			{
				pc += 4;
			}
			break;

		// 0x5XY0	if V[X] == V[Y], skip next instruction
		case 0x5000:
			if (V[opcode & 0x0F00] == V[opcode & 0x00F0])
			{
				pc += 4;
			}
			break;

		// 0x6XNN	V[X] = NN
		case 0x6000:
			V[opcode & 0x0F00] = (opcode & 0x00FF);
			pc += 2;
			break;

		// 0x7XNN	V[X] += NN			???? CARRY FLAG IS NOT CHANGED ?????
		case 0x7000:
			V[opcode & 0x0F00] += (opcode & 0x0FF);
			pc += 2;
			break;

		// Handling multiple 0x8 cases
		case 0x8000:
			switch (opcode & 0x000F)
			{
				// 0x8XY0	V[X] = V[Y]
				case 0x0000:
					V[opcode & 0x0F00] = V[opcode & 0x00F0];
					pc += 2;
					break;
				
				// 0x8XY1	V[X] = V[X] | V[Y]
				case 0x0001:
					V[opcode & 0x0F00] = (V[opcode & 0x0F00] | V[opcode & 0x00F0]);
					pc += 2;
					break;
					
				// 0x8XY2	V[X] = V[X] & V[Y]
				case 0x0002:
					V[opcode & 0x0F00] = (V[opcode & 0x0F00] & V[opcode & 0x00F0]);
					pc += 2;
					break;

				// 0x8XY3	V[X] = V[X] ^ V[Y]
				case 0x0003:
					V[opcode & 0x0F00] = (V[opcode & 0x0F00] ^ V[opcode & 0x00F0]);
					pc += 2;
					break;

				/*
				
				// 0x8XY4	V[X] = V[X] + V[Y]		V[F] = 1 if carry, else 0
				case 0x0004:
					V[opcode & 0x0F00] = (V[opcode & 0x0F00] + V[opcode & 0x00F0]);
					pc += 2;
					break;
					
				// 0x8XY5	V[X] = V[X] - V[Y]		V[F] = 0 if borrow, else 1
				case 0x0005:
					V[opcode & 0x0F00] = (V[opcode & 0x0F00] - V[opcode & 0x00F0]);
					pc += 2;
					break;

					*/

				// 0x8XY6	V[F] = bit meno significativo di V[X] and V[X] >> 1
				case 0x0006:
					V[0x000F] = V[opcode & 0x0F00] & 0x0001;
					V[opcode & 0x0F00] >> 1;
					pc += 2;
					break;

				/*

				// 0x8XY7	V[X] = V[Y] - V[X]		V[F] = 0 if borrow, else 1
				case 0x0007:
					V[opcode & 0x0F00] = (V[opcode & 0x00F0] - V[opcode & 0x0F00]);
					pc += 2;
					break;

				*/

				// 0x8XYE	V[F] = bit meno significativo di V[X] and V[X] << 1
				case 0x000E:
					V[0x000F] = V[opcode & 0x0F00] & 0x0001;
					V[opcode & 0x0F00] << 1;
					pc += 2;
					break;


				default:
					printf("Unknown opcode 0x%X\n", opcode);
					fflush(stdout);
			}
			break;

		// 0xANNN -->  set I to address NNN
		case 0xA000:
			I = opcode & 0x0FFF;
			pc += 2;
			break;
		
		// 0xBNNN -->  PC = V0 + NNN     jump to address V0+NNN
		case 0xB000:
			pc = V[0] + (opcode & 0x0FFF);
			break;

		// 0xCXNN -->  V[X] = rand() % NN         0 <= rand <= 255
		case 0xC000:
			V[opcode & 0x0F00] = (opcode & 0x00FF) & (rand() % 256);
			pc += 2;
			break;

		default:
			printf("Opcode Error!!\n Code: 0x%X\n", opcode);
			fflush(stdout);

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