#include <time.h>
#include <unistd.h>
#include "chip8.h"
#include "keymap.h"
#define DEBUG 0
#if DEBUG
#define print(...) printf(__VA_ARGS__);
#else
#define print(...)
#endif

void beepThread(AudioDevice* device)
{
    device->renderFrequency(700, 500, 32600);
}

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

	memset(memory, 0, MEM_SIZE);
	memset(V, 0, N_REG);
	memset(key, 0, N_KEYS);
	memset(gfx, 0, GMEM_ROWS * GMEM_COLS);
	memset(stack, 0, STACK_SIZE);

	for (int i = 0; i < 80; ++i)
	{
		memory[i] = chip8_fontset[i];
	}

	// inizializzo a caso per ora poi capiremo
    delay_timer = 0;
    sound_timer = 0;
    drawFlag = false;
	drawRow = 0;
    oldRow = 0;
    srand(time(NULL));
}

bool Chip8::loadRom(const char* filename)
{
    FILE* rom = fopen(filename, "rb");

    if(rom != NULL)
    {
        unsigned short init_addr = 0x200;
        unsigned char* ptr = &memory[init_addr];

        int r = fread(ptr, 1, MEM_SIZE - init_addr, rom);
        fclose(rom);
        return r > 0;
    }
    else
    {
        return false;
    }
}

void Chip8::drawSprite(unsigned char x, unsigned char y, unsigned char n)
{
    V[0xF] = 0x00; // setto a 0 il collision flag

    for (int i = 0; i < n; i++)
    {
        unsigned char act_byte = memory[I + i]; //leggo un byte dalla memoria
        for (int j = 0; j < 8; j++) //ciclo su ciascun bit
        {
            unsigned char act_bit = (act_byte >> j) & 0x01; //il bit attuale mi sarà dato dal bit meno significativo del byte attuale shiftato a destra di j posizioni

            unsigned int row_index = (y + i) % GMEM_ROWS;
            unsigned int col_index = (x + (7 - j)) % GMEM_COLS;

            if(act_bit == 1)
            {
                unsigned char act_pixel = gfx[row_index][col_index]; //il pixel attuale che devo confrontare con act_bit
                if(act_pixel == 1) //il collision flag va messo a 1 quando trovo un pixel da disegnare che è già a 1 (http://www.multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/)
                {
                    V[0xF] = 0x01;
                }
                gfx[row_index][col_index] = act_pixel ^ act_bit; //aggiorno il pixel attuale
            }
        }
    }

#if DEBUG
    for(int i = 0; i < GMEM_ROWS; i++)
    {
        for(int j = 0; j < GMEM_COLS; j++)
        {
            printf("%c", gfx[i][j] == 1 ? '*':' ');
        }
        printf("%s", "\n");
    }
#endif

}

void Chip8::emulateCycle()
{
    static int loop = 0;
    loop += 1;
    if(pc > MEM_SIZE)
    {
        print("%s\n", "Error: program counter out of memory");
        exit(0);
        return;
    }

    // FETCH ISTRUZIONE
	// Devo combinare memory[pc] e memory[pc+1] --> ne leggo uno, shifto di 1 byte e faccio l'or con l'altro byte

    instruction = memory[pc] << 8 | memory[pc + 1]; // istruzione = 2 byte = 16 bit

    // Stampo istruzione in esadecimale
    print("\nLoop: %d\nProgram counter before instruction fetch: %hu\n", loop, pc);
    print("Hex instruction: 0x%X\n", instruction);
    // FETCH OPERANDI
    opcode = (instruction & 0xF000);        //opcode = 4 bit più significativi dell'istruzione
    x = ((instruction >> 8) & 0x000F);      //operando x = i 4 bit meno significativi del byte più significativo
    y =  ((instruction >> 4) & 0x000F);     //operando y = i 4 bit più significativi del byte meno significativo
    n =  (instruction & 0x000F);            //operando n = 4 bit meno significativi dell'istruzione
    kk =  (instruction & 0x0FF);            //operando kk = byte meno significativo dell'istruzione
    nnn =  (instruction & 0x0FFF);          //operando nnn = 12 bit meno significativi dell'istruzione (1 byte e mezzo :O)

    //Stampo operandi fetchati
    print("Opcode: 0x%X = %hu\n", opcode, opcode);
    print("x = 0x%X = %hhu\n", x, x);
    print("y = 0x%X = %hhu\n", y, y);
    print("n = 0x%X = %hhu\n", n, n);
    print("kk = 0x%X = %hu\n", kk, kk);
    print("nnn = 0x%X = %hu\n", nnn, nnn);

	switch (opcode)
	{
		// DECODE DEGLI OPCODE

		// gestione 2 op che iniziano con 0x00
        case 0x0000:
            switch (kk)
            {
                // clear screen
            case 0x00E0:
                print("Instruction: %s\n", "Clear Screen");
                memset(gfx, 0, GMEM_ROWS * GMEM_COLS);
                drawFlag = true;
                pc += 2;
                break;
    
                // return from subroutine
            case 0x00EE:
                print("Instruction: %s\n", "Return from subroutine");
                --sp;
                pc = stack[sp];
                print("PC = stack[%hu] = %hu\n", sp, pc);
                break;
    
            default:
                print("Unknown instruction 0x%X\n", instruction);
                fflush(stdout);
                exit(1);
            }
            break;

		// 0x1NNN	goto NNN
        case 0x1000:
            print("Instruction: GOTO %hu\n", nnn);
            pc = nnn;
            break;

		// 0x2NNN   calls subroutine at NNN
        case 0x2000:
            print("Instruction: call subrotine at %hu\n", nnn);
            stack[sp] = pc + 0x02;
            ++sp;
            pc = nnn;
            break;

		// 0x3XNN	if V[X] == NN, skip next instruction
        case 0x3000:
            print("Instruction: skip next instruction if V[%hhu] == %hu\n", x, kk);
            pc += (V[x] == kk) ? 4 : 2;
            break;

		// 0x4XNN	if V[X] != NN, skip next instruction
        case 0x4000:
            print("Instruction: skip next instruction if V[%hhu] != %hu\n", x, kk);
            pc += (V[x] != kk) ? 4 : 2;
            break;

		// 0x5XY0	if V[X] == V[Y], skip next instruction
        case 0x5000:
            print("Instruction: skip next instruction if V[%hhu] == V[%hhu]\n", x, y);
            pc += (V[x] == V[y]) ? 4 : 2;
            break;

		// 0x6XNN	V[X] = NN
		case 0x6000:
            print("Instruction: set V[%hhu] = %hhu\n", x, kk);
            V[x] = kk;
			pc += 2;
			break;

		// 0x7XNN	V[X] += NN
		case 0x7000:
            print("Instruction: set V[%hhu] += %hhu\n", x, kk);
            V[x] += kk;
			pc += 2;
			break;

		// Handling multiple 0x8 cases
		case 0x8000:
            switch (n)
			{
				// 0x8XY0	V[X] = V[Y]
				case 0x0000:
                    print("Instruction: set V[%hhu] = V[%hhu]\n", x, y);
                    V[x] = V[y];
					pc += 2;
					break;
				
				// 0x8XY1	V[X] = V[X] | V[Y]
				case 0x0001:
                    print("Instruction: set V[%hhu] |= V[%hhu]\n", x, y);
                    V[x] = V[x] | V[y];
					pc += 2;
					break;
					
				// 0x8XY2	V[X] = V[X] & V[Y]
				case 0x0002:
                    print("Instruction: set V[%hhu] &= V[%hhu]\n", x, y);
                    V[x] = V[x] & V[y];
					pc += 2;
					break;

				// 0x8XY3	V[X] = V[X] ^ V[Y]
				case 0x0003:
                    print("Instruction: set V[%hhu] ^= V[%hhu]\n", x, y);
                    V[x] = V[x] ^ V[y];
					pc += 2;
					break;

				// 0x8XY4	V[X] = V[X] + V[Y]		V[F] = 1 if carry, else 0
				case 0x0004:
                    print("Instruction: set V[%hhu] += V[%hhu]\n", x, y);
                    V[0xF] = ((int)V[x] + (int)V[y] > 255) ? 1 : 0; //casto V[x] e V[y] a interi e guardo se la somma è maggiore di 255 quindi overflow
                    V[x] = V[x] + V[y];
					pc += 2;
					break;
					
				// 0x8XY5	V[X] = V[X] - V[Y]		V[F] = 0 if borrow, else 1
				case 0x0005:
                    print("Instruction: set V[%hhu] -= V[%hhu]\n", x, y);
                    V[0xF] = (V[x] > V[y]) ? 1 : 0;
                    V[x] = V[x] - V[y];
					pc += 2;
					break;

				// 0x8XY6	V[F] = bit meno significativo di V[X] and V[X] >> 1
				case 0x0006:
                    print("Instruction: shift right V[%hhu]\n", x);
                    V[0xF] = V[x] & 0x0001;
                    V[x] = V[x] >> 1;
					pc += 2;
					break;


				// 0x8XY7	V[X] = V[Y] - V[X]		V[F] = 0 if borrow, else 1
				case 0x0007:
                    print("Instruction: set V[%hhu] = V[%hhu] - V[%hhu]\n", x, y, x);
                    V[0xF] = (V[y] > V[x]) ? 1 : 0;
                    V[x] = V[y] - V[x];
                    pc += 2;
                    break;

                // 0x8XYE	V[F] = bit più significativo di V[X] and V[X] << 1
				case 0x000E:
                    print("Instruction: shift left V[%hhu]\n", x);
                    V[0xF] = (V[x] >> 7) & 0x1;
                    V[x] = (V[x] << 1);
					pc += 2;
					break;


				default:
                    print("Unknown opcode 0x%X\n", opcode);
					fflush(stdout);
                    exit(1);
			}
			break;


		// 0x9XY0	skips instruction if V[x] != V[y]
		case 0x9000:
			pc += (V[x] != V[y]) ? 4 : 2;
			break;

		// 0xANNN -->  set I to address NNN
		case 0xA000:
            print("Instruction: set I = %hu\n", nnn);
            I = nnn;
			pc += 2;
			break;
		
		// 0xBNNN -->  PC = V0 + NNN     jump to address V0+NNN
		case 0xB000:
            print("Instruction: set PC = V[0] + %hu\n", nnn);
            pc = V[0] + nnn;
			break;

		// 0xCXNN -->  V[X] = rand() % NN         0 <= rand <= 255
		case 0xC000:
            print("Instruction: set V[%hhu] = rand() %% %hhu\n", x, kk);
            V[x] = (unsigned char)((rand() % 256)) % kk;
			pc += 2;
			break;
			
		
		// 0xDXYN	draws sprite at coordinate (VX, VY), width = 8 px, height = N px
		case 0xD000:
            print("Instruction: draw sprite at (V[%hhu], V[%hhu]), height = %hhu\n", x, y, n);
            drawSprite(V[x], V[y], n);
			drawFlag = true;
			pc += 2;
			break;

		// Handling 0xE cases, skips instruction if key is / is not pressed
		case 0xE000:
			switch (kk)
			{
				// 0xEX9E	skips instruction if key in VX is pressed
				case 0x009E:
                    print("Instruction: skip next if V[%hhu] is pressed\n", V[x]);
                    pc += (key[V[x]] == 1) ? 4 : 2;
					break;
				
                // 0xEXA1	skips instruction if key in VX is NOT pressed
				case 0x00A1:
                    print("Instruction: skip next if V[%hhu] is NOT pressed\n",V[x]);
                    pc += (key[V[x]] == 0) ? 4 : 2;
					break;

				default:
                    print("Opcode Error!!\n Code: 0x%X\n", opcode);
					fflush(stdout);
                    exit(1);
			}
			break;

		// Handling multiple 0xF cases
		case 0xF000:
			switch (kk)
			{
				// 0xFX07	VX = get_delay
				case 0x0007:
					V[x] = delay_timer;
					pc += 2;
					break;


				// VX = wait_for_key_pressed	blocking waiting for input, store key pressed in V[X]
				case 0x000A:
                    print("Instruction: wait for keypress and store key in V[%hhu]\n", x);
                    for(int i = 0; i < 16; i++)
                    {
                        if(key[i] == 1)
                        {
                            V[x] = i;
                            pc += 2;
                        }
                    }
					break;


				// 0xFX15	delay_timer = VX
				case 0x0015:
					delay_timer = V[x];
					pc += 2;
					break;

				// 0xFX18	sound_timer = VX
				case 0x0018:
					sound_timer = V[x];
					pc += 2;
					break;

				// 0xFX1E	I += VX
				case 0x001E:
					
					// QUESTO CHECK NON SO SE SIA NECESSARIO, WIKIPEDIA DICE SI MA DA ALTRE PARTI NON LO VEDO DHN

					V[0xF] = ((int)I + (int)V[x] > MEM_SIZE) ? 1 : 0;
					I += V[x];
					pc += 2;
					break;


					// DA RIGUARDARE

				// 0xFX29	I = sprite_address_of_char_in_VX
				case 0x0029:
					
					// Spero di aver capito bene cosa chiede :D

                    I = 5 * V[x];
					pc += 2;
					break;


                // 0xFX33	set_bcd(V[x])
				case 0x0033:
                    print("Instruction: set bcd(V[%hhu])\n", x);
                    memory[I+2] = (V[x] % 10);  // le unità
                    memory[I+1] = (V[x] % 100) / 10; // le decine
                    memory[I]   = (V[x] % 1000) / 100; // le centinaia

                    pc += 2;
					break;


				// 0xFX55	stores V0 - VX in memory[I + i]
				case 0x0055:
                    print("Instruction: copy from V[0] to V[%hhu] into memory[%hu]\n", x, I);
                    for (int i = 0; i <= x; ++i)
                    {
                        memory[I + i] = V[i];
                    }
                    I = (I + x + 1);

                    pc += 2;
                    break;


				// 0xFX65	fills V0 - VX with values in memory[I + i]
				case 0x0065:
                    print("Instruction: copy from memory[%hu] into V[0] - V[%hhu]\n", I, x);
                    for (int i = 0; i <= x; ++i)
					{
						V[i] = memory[I + i];
					}
                    I = (I + x + 1);

					pc += 2;
					break;

				default:
                    print("Opcode Error!!\n Code: 0x%X\n", opcode);
                    exit(1);
					fflush(stdout);
			}
			break;

		default:
            print("Opcode Error!!\n Code: 0x%X\n", opcode);
			fflush(stdout);
            exit(1);
    }

    print("Program counter after instruction execute: %hu\n\n", pc);

}

void Chip8::onTickElapsed()
{
    if (delay_timer > 0)
    {
        --delay_timer;
    }

    if (sound_timer > 0)
    {
        --sound_timer;
        if (sound_timer == 0)
        {
            audioThread = new std::thread(beepThread, &audioDevice);
        }
    }
}


void Chip8::onKeyPress(int keycode)
{
    print("Key pressed:% d\n", keycode);
    if(keycode < N_KEYS)
    {
        key[keycode] = 1;
    }
}

void Chip8::onKeyRelease(int keycode)
{
    print("Key released:% d\n", keycode);
    if(keycode < N_KEYS)
    {
        key[keycode] = 0;
    }
}
