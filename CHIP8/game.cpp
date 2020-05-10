#include "chip8.h"
#include "chip8test.h"
#include <GL/gl.h>
#include <GL/glut.h>

//--- utilissima funzione per fare una sleep di usec microsecondi che funziona sia su win che su linux ---//
#include <chrono>
#include <thread>
void sleepMicroseconds(unsigned long usec) //mi immagino possa servire per la freq. clock
{
    std::this_thread::sleep_for(std::chrono::microseconds(usec));
}
//-------------------------------------------------------------------------------------------------------//

Chip8 myChip8;

/*
 * Definisco la matrice RIGHE x COLONNE x 3 che opengl deve renderizzare.
 * In realtà è una semplice copia (per ora) della memoria grafica di chip8
 * però per ciascun valore 1/0 dei pixel, definisco i parametri R,G,B per quel pixel:
 * Se chip8.gfx[i][j] = 1 -> openGlScreen[i][j][0 = R] = 255, openGlScreen[i][j][1 = G] = 255, openGlScreen[i][j][2 = B] = 255
 * Ossia ho messo rosso 255, verde 255, blu 255 = BIANCO
 * Se li metto tutti a zero avrò il nero, taac
 */
unsigned char openGlScreen[GMEM_ROWS][GMEM_COLS][3];

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

void renderChip8() //funzione che renderizza nella finestra opengl il contenuto della memoria grafica di chip8
{
    for(int i = 0; i < GMEM_ROWS; i++)
    {
        for(int j = 0; j < GMEM_COLS; j++)
        {
            if(myChip8.gfx[i][j] == 1)
            {
                openGlScreen[i][j][0] = openGlScreen[i][j][1] = openGlScreen[i][j][2] = 0xFF; //quel pixel deve essere bianco
            }
        }
    }

    glDrawPixels(GMEM_COLS, GMEM_ROWS, GL_RGB, GL_UNSIGNED_BYTE, (void *)openGlScreen); //disegno openGlScreen
    glutSwapBuffers();
}

void reshapeWindowCallback(GLsizei w, GLsizei h)
{
    //per ora è vuota poi ci si pensa, viene chiamata quando uno allarga o restringe la finestra
}

void emulationLoop()
{
    double clockPeriod_s = 1.0 / myChip8.clockFreq_hz;
    unsigned long clockPeriod_us = static_cast<unsigned long>(clockPeriod_s * 1e6);
    printf("clockPeriod_us: %ld\n", clockPeriod_us);

    printf("Start emulation loop\n");
    while (1)
    {
        printf("%s\n", "Start CPU Cycle");
#if 0
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
        //getchar();
#endif
        printf("%s\n", "End CPU Cycle\n");
        fflush(stdout);
        sleepMicroseconds(clockPeriod_us);
    }

}

void setupOpengl(int argc, char** argv) //funzione di inizializzazione di opengl
{
    for(int i = 0; i < GMEM_ROWS; i++)
    {
        for(int j = 0; j < GMEM_COLS; j++)
        {
            openGlScreen[i][j][0] = openGlScreen[i][j][1] = openGlScreen[i][j][2] = 0x00; //la finestra all'inizio è tutta nera
        }
    }

    glutInit(&argc, argv); //passo a opengl eventuali argomenti da tastiera
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); //modo standard per inizializzare un display mode che va bene quasi sempre

    glutInitWindowSize(GMEM_COLS, GMEM_ROWS); //creo una finestra 32*64 pixel (sono gli stessi della memoria grafica di chip8, poi vediamo come farla più grande)
    glutInitWindowPosition(0, 0); //piazzo la finestra in alto a sx
    glutCreateWindow("chip8 emulator"); //visualizza la finestra con un titolo

    glutDisplayFunc(renderChip8); //assegno renderChip8 alla displayFunc ossia la funzione che viene chiamata per renderizzare la memorya di chip8 (credo che questa debba essere eseguita ogni volta che drawFlag = true)
    glutIdleFunc(emulationLoop); //assegno emulationLoop all'idleFunc ossia la funzione che viene ciclicamente eseguita da opengl tipo nei kilobot
    glutReshapeFunc(reshapeWindowCallback); //assegno reshapeWindowCallback all'evento reshapeWindow di opengl
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

    glutMainLoop(); //lancio l'emulatore attraverso l'esecuzione della mainloop di opengl

    return 0;
}
