#include "chip8.h"
Chip8 myChip8;

int dmain(int argc, char** argv)
{
    //setupGraphics();
    //setupInput();
    myChip8.initialize();
    myChip8.loadRom("pong.ch8");
   // myChip8.loadGame("pong");

    while (1)
    {
        printf("%s\n", "Start CPU Cycle");
        //emulate one cycle
        myChip8.emulateCycle();
        // if draw flag, update screen
        if (myChip8.drawFlag)
        {
            //drawGraphics();
            //myChip8.drawFlag = false;
        }
        // store keys press state (press-release)
        //myChip8.setKeys();
        printf("%s\n", "End CPU Cycle");
        //getchar();
    }

    return 0;
}
