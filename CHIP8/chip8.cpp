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
	drawRow = 0;
	oldRow = 0;
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
		pc += (V[x] == kk) ? 4 : 2;
		break;

		// 0x4XNN	if V[X] != NN, skip next instruction
	case 0x4000:
		printf("Instruction: skip next instruction if V[%hhu] != %hu\n", x, kk);
		pc += (V[x] != kk) ? 4 : 2;
		break;

		// 0x5XY0	if V[X] == V[Y], skip next instruction
	case 0x5000:
		printf("Instruction: skip next instruction if V[%hhu] == V[%hhu]\n", x, y);
		pc += (V[x] == V[y]) ? 4 : 2;
		break;

		// 0x6XNN	V[X] = NN
		case 0x6000:
            printf("Instruction: set V[%hhu] = %hhu\n", x, kk);
            V[x] = kk;
			pc += 2;
			break;

		// 0x7XNN	V[X] += NN			???? CARRY FLAG IS NOT CHANGED ?????
		case 0x7000:
            printf("Instruction: set V[%hhu] += %hhu\n", x, kk);
            V[x] += kk;
			pc += 2;
			break;

		// Handling multiple 0x8 cases
		case 0x8000:
            switch (n)
			{
				// 0x8XY0	V[X] = V[Y]
				case 0x0000:
                    printf("Instruction: set V[%hhu] = V[%hhu]\n", x, y);
                    V[x] = V[y];
					pc += 2;
					break;
				
				// 0x8XY1	V[X] = V[X] | V[Y]
				case 0x0001:
                    printf("Instruction: set V[%hhu] |= V[%hhu]\n", x, y);
                    V[x] = V[x] | V[y];
					pc += 2;
					break;
					
				// 0x8XY2	V[X] = V[X] & V[Y]
				case 0x0002:
                    printf("Instruction: set V[%hhu] &= V[%hhu]\n", x, y);
                    V[x] = V[x] & V[y];
					pc += 2;
					break;

				// 0x8XY3	V[X] = V[X] ^ V[Y]
				case 0x0003:
                    printf("Instruction: set V[%hhu] ^= V[%hhu]\n", x, y);
                    V[x] = V[x] ^ V[y];
					pc += 2;
					break;

				// 0x8XY4	V[X] = V[X] + V[Y]		V[F] = 1 if carry, else 0
				case 0x0004:
                    printf("Instruction: set V[%hhu] += V[%hhu]\n", x, y);
                    V[0xF] = ((int)V[x] + (int)V[y] > 255) ? 1 : 0; //casto V[x] e V[y] a interi e guardo se la somma è maggiore di 255 quindi overflow
                    V[x] = V[x] + V[y];
					pc += 2;
					break;
					
				// 0x8XY5	V[X] = V[X] - V[Y]		V[F] = 0 if borrow, else 1
				case 0x0005:
                    printf("Instruction: set V[%hhu] -= V[%hhu]\n", x, y);
                    V[0xF] = (V[x] > V[y]) ? 0 : 1;
                    V[x] = V[x] - V[y];
					pc += 2;
					break;

				// 0x8XY6	V[F] = bit meno significativo di V[X] and V[X] >> 1
				case 0x0006:
                    printf("Instruction: shift right V[%hhu]\n", x);
                    V[0xF] = V[x] & 0x0001;
                    V[x] = V[x] >> 1;
					pc += 2;
					break;


				// 0x8XY7	V[X] = V[Y] - V[X]		V[F] = 0 if borrow, else 1
				case 0x0007:
                    printf("Instruction: set V[%hhu] = V[%hhu] - V[%hhu]\n", x, y, x);
                    V[0xF] = (V[y] > V[x]) ? 0 : 1;
                    V[x] = V[y] - V[x];
                    pc += 2;
                    break;

				// 0x8XYE	V[F] = bit meno significativo di V[X] and V[X] << 1
				case 0x000E:
                    printf("Instruction: shift left V[%hhu]\n", x);
                    V[0xF] = V[x] & 0x0001;
                    V[x] = V[x] << 1;
					pc += 2;
					break;


				default:
					printf("Unknown opcode 0x%X\n", opcode);
					fflush(stdout);
			}
			break;

		case 0x9000:
			pc += (V[x] != V[y]) ? 4 : 2;
			break;

		// 0xANNN -->  set I to address NNN
		case 0xA000:
            printf("Instruction: set I = %hu\n", nnn);
            I = nnn;
			pc += 2;
			break;
		
		// 0xBNNN -->  PC = V0 + NNN     jump to address V0+NNN
		case 0xB000:
            printf("Instruction: set PC = V[0] + %hu\n", nnn);
            pc = V[0] + nnn;
			break;

		// 0xCXNN -->  V[X] = rand() % NN         0 <= rand <= 255
		case 0xC000:
            printf("Instruction: set V[%hhu] = rand() %% %hhu\n", x, kk);
            V[x] = (unsigned char)((rand() % 256)) % kk;
			pc += 2;
			break;
			
		
		// 0xDXYN	draws sprite at coordinate (VX, VY), width = 8 px, height = N px
		case 0xD000:
			for (int i = 0; i < n; ++i)
			{
				drawRow = memory[I + i];
				oldRow = gfx[V[x] + i][V[y]];
				gfx[V[x] + i][V[y]] ^= drawRow;
				if (drawFlag)
				{
					checkFlip(oldRow, drawRow);
				}
			}
			drawFlag = false;
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


void Chip8::checkFlip(unsigned char oldRow, unsigned char drawRow)
{
	while (oldRow)
	{
		if ((oldRow & 1) & (drawRow & 1))
		{
			V[0xF] = 1;
			drawFlag = true;
			break;
		}
		oldRow = oldRow >> 1;
		drawRow = drawRow >> 1;
	}
}