#include "chip8.h"
Chip8 myChip8;

int driver_main(int argc, char** argv)
{
    //setupGraphics();
    //setupInput();
    myChip8.initialize();
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
        }
        // store keys press state (press-release)
        //myChip8.setKeys();
        printf("%s\n", "End CPU Cycle");
        //getchar();
    }

    return 0;
}
