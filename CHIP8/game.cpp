#include "chip8.h"
#include "chip8test.h"
#include <GL/gl.h>
#include <GL/glut.h>

Chip8 myChip8;

int keymap(unsigned char glKey) // assegno a ciascun keycode dato da opengl un valore da flippare nel vettore keys di chip8
{
    /*
     Ovviamente sta roba è arbitraria
    */
    switch(glKey)
    {
        case 'q': return 0;
        case 'w': return 1;
        case 'e': return 2;
        case 'r': return 3;
        case 't': return 4;
        case 'y': return 5;
        case 'a': return 6;
        case 's': return 7;
        case 'd': return 8;
        case 'f': return 9;
        case 'g': return 10;
        case 'z': return 11;
        case 'x': return 12;
        case 'c': return 13;
        case 'v': return 14;
        case 'b': return 15;
        default: return -1;
    }
}

void keyPressCallback(unsigned char key, int x, int y) //signatura (:O) che vuole opengl
{
    static_cast<void>(x); // giusto per togliere gli warning unused variable
    static_cast<void>(y);

    int keycode = keymap(key);
    if(keycode != -1)
    {
        myChip8.onKeyPress(keycode);
    }
}

void keyReleaseCallback(unsigned char key, int x, int y)
{
    static_cast<void>(x);
    static_cast<void>(y);

    int keycode = keymap(key);
    if(keycode != -1)
    {
        myChip8.onKeyRelease(keycode);
    }
}

void setupOpengl(int argc, char** argv) //funzione di inizializzazione di opengl
{
    glutInit(&argc, argv); //passo a opengl eventuali argomenti da tastiera
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); //modo standard per inizializzare un display mode che va bene quasi sempre

    glutInitWindowSize(32, 64); //creo una finestra 32*64 pixel (sono gli stessi della memoria grafica di chip8, poi vediamo come farla più grande)
    glutInitWindowPosition(0, 0); //piazzo la finestra in alto a sx
    glutCreateWindow("chip8 emulator"); //visualizza la finestra con un titolo

    glutKeyboardFunc(keyPressCallback); //assegno la funzione keyPressCallback all'evento keyboard di opengl
    glutKeyboardUpFunc(keyReleaseCallback); //stessa cosa con keyReleaseCallback
}

int main(int argc, char** argv)
{
    run_tests();
    
    setupOpengl(argc, argv);
    //setupInput();
    myChip8.initialize();
    myChip8.loadRom("pong.ch8");
   // myChip8.loadGame("pong");

    while (0)
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
