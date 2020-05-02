#include "chip8.h"

Chip8 myChip8;

int main(int argc, char** argv)
{
	//setupGraphics();
	//setupInput();

	//myChip8.initialize();
	//myChip8.loadGame("pong");

	while (1)
	{
		//emulate one cycle
		
		//myChip8.emulateCycle();

		// if draw flag, update screen
		if (myChip8.drawFlag)
		{
			//drawGraphics();
		}

		// store keys press state (press-release)

		//myChip8.setKeys();

	}

	return 0;
}
