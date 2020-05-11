#include "keymap.h"

int keymap(int key) // assegno a ciascun keycode dato da opengl un valore da flippare nel vettore keys di chip8
{
    /*
     Ovviamente sta roba è arbitraria
    */
    switch(key)
    {
        case '1': return 1;
        case '2': return 2;
        case '3': return 3;
        case '4': return 12;
        case 'q': return 4;
        case 'w': return 5;
        case 'e': return 6;
        case 'r': return 13;
        case 'a': return 7;
        case 's': return 8;
        case 'd': return 9;
        case 'f': return 14;
        case 'z': return 10;
        case 'x': return 0;
        case 'c': return 11;
        case 'v': return 15;
        default: return -1;
    }
}
