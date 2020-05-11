#include "keymap.h"

int keymap(int key) // assegno a ciascun keycode dato da opengl un valore da flippare nel vettore keys di chip8
{
    /*
     Ovviamente sta roba è arbitraria
    */
    switch(key)
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
