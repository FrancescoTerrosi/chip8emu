#include "chip8.h"

Chip8::Chip8()
{

}

void Chip8::initialize()
{
	pc = 0x200;
	opcode = 0;
	I = 0;
	sp = 0;
    instruction = 0;
    x = 0;
    y = 0;
    kk = 0;
    nnn = 0;
    n = 0;

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
    drawFlag = false;
}

void Chip8::emulateCycle()
{
    if(pc > MEM_SIZE)
    {
        printf("%s\n", "Error: program counter out of memory");
        return;
    }

    // FETCH ISTRUZIONE
	// Devo combinare memory[pc] e memory[pc+1] --> ne leggo uno, shifto di 1 byte e faccio l'or con l'altro byte
    instruction = memory[pc] << 8 | memory[pc + 1]; // istruzione = 2 byte = 16 bit
    // Stampo istruzione in esadecimale
    printf("\nHex instruction: 0x%X\n", instruction);
    // FETCH OPERANDI
    opcode = (instruction & 0xF000);        //opcode = 4 bit più significativi dell'istruzione
    x = ((instruction >> 8) & 0x000F);      //operando x = i 4 bit meno significativi del byte più significativo
    y =  ((instruction >> 4) & 0x000F);     //operando y = i 4 bit più significativi del byte meno significativo
    n =  (instruction & 0x000F);            //operando n = 4 bit meno significativi dell'istruzione
    kk =  (instruction & 0x0FF);            //operando kk = byte meno significativo dell'istruzione
    nnn =  (instruction & 0x0FFF);          //operando nnn = 12 bit meno significativi dell'istruzione (1 byte e mezzo :O)

    //Stampo operandi fetchati
    printf("Opcode: 0x%X = %hu\n", opcode, opcode);
    printf("x = 0x%X = %hhu\n", x, x);
    printf("y = 0x%X = %hhu\n", y, y);
    printf("n = 0x%X = %hhu\n", n, n);
    printf("kk = 0x%X = %hu\n", kk, kk);
    printf("nnn = 0x%X = %hu\n", nnn, nnn);

    switch (opcode)
	{
		// DECODE DEGLI OPCODE

		// gestione 2 op che iniziano con 0x00
		case 0x0000:
            switch (kk)
			{
				// clear screen
                case 0xE0:
                    printf("Instruction: %s\n", "Clear Screen");
                    memset(gfx, 0, 64 * 32);
                    drawFlag = true;
					pc += 2;
					break;

				// return from subroutine
                case 0xEE:
                    printf("Instruction: %s\n", "Return from subroutine");
					--sp;
					pc = stack[sp];
                    printf("PC = stack[%hu] = %hu\n", sp, pc);
					break;

				default:
                    printf("Unknown instruction 0x%X\n", instruction);
					fflush(stdout);
			}
			break;

		// 0x1NNN	goto NNN
		case 0x1000:
            printf("Instruction: GOTO %hu\n", nnn);
            pc = nnn;
			break;

		// 0x2NNN   calls subroutine at NNN
		case 0x2000:
            printf("Instruction: call subrotine at %hu\n", nnn);
            stack[sp] = pc + 0x02;
			++sp;
            pc = nnn;
			break;

		// 0x3XNN	if V[X] == NN, skip next instruction
		case 0x3000:
            printf("Instruction: skip next instruction if V[%hhu] == %hu\n", x, kk);
            if (V[x] == kk)
			{
				pc += 4;
			}
            else
            {
                pc += 2;
            }
			break;
			// 0x4XNN	if V[X] != NN, skip next instruction
		case 0x4000:
            printf("Instruction: skip next instruction if V[%hhu] != %hu\n", x, kk);
            if (V[x] != kk)
            {
                pc += 4;
            }
            else
            {
                pc += 2;
            }
            break;

		// 0x5XY0	if V[X] == V[Y], skip next instruction
		case 0x5000:
            if (V[x] == V[y])
			{
				pc += 4;
			}
            else
            {
                pc += 2;
            }
			break;

		// 0x6XNN	V[X] = NN
		case 0x6000:
            V[x] = kk;
			pc += 2;
			break;

		// 0x7XNN	V[X] += NN			???? CARRY FLAG IS NOT CHANGED ?????
		case 0x7000:
            V[x] += kk;
			pc += 2;
			break;

		// Handling multiple 0x8 cases
		case 0x8000:
			switch (opcode & 0x000F)
			{
				// 0x8XY0	V[X] = V[Y]
				case 0x0000:
					V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
					pc += 2;
					break;
				
				// 0x8XY1	V[X] = V[X] | V[Y]
				case 0x0001:
					V[(opcode & 0x0F00) >> 8] = (V[(opcode & 0x0F00) >> 8] | V[(opcode & 0x00F0) >> 4]);
					pc += 2;
					break;
					
				// 0x8XY2	V[X] = V[X] & V[Y]
				case 0x0002:
					V[(opcode & 0x0F00) >> 8] = (V[(opcode & 0x0F00) >> 8] & V[(opcode & 0x00F0) >> 4]);
					pc += 2;
					break;

				// 0x8XY3	V[X] = V[X] ^ V[Y]
				case 0x0003:
					V[(opcode & 0x0F00) >> 8] = (V[(opcode & 0x0F00) >> 8] ^ V[(opcode & 0x00F0) >> 4]);
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
					V[0x000F] = V[(opcode & 0x0F00) >> 8] & 0x0001;
					V[(opcode & 0x0F00) >> 8] >> 1;
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
					V[0x000F] = V[(opcode & 0x0F00) >> 8] & 0x0001;
					V[(opcode & 0x0F00) >> 8] << 1;
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
			V[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF) & (rand() % 256);
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
