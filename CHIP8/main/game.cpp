#include "chip8.h"
#include "chip8test.h"
#include "keymap.h"
#include <GL/gl.h>
#include <GL/glut.h>

/*
 * Quì definisco le costanti per la finestra opengl
 */
const int PIXEL_SIZE = 10; //ciascun pixel di gfx sarà ripetuto 10 volte in basso e 10 volte a destra
const int SCREEN_ROWS = GMEM_ROWS * PIXEL_SIZE;
const int SCREEN_COLS = GMEM_COLS * PIXEL_SIZE;

//--- utilissima funzione per fare una sleep di usec microsecondi che funziona sia su win che su linux ---//
#include <chrono>
#include <thread>
void sleepMicroseconds(unsigned long usec) //serve per la freq. clock
{
    std::this_thread::sleep_for(std::chrono::microseconds(usec));
}
//-------------------------------------------------------------------------------------------------------//

Chip8 myChip8;

/*
 * Definisco la matrice RIGHE x COLONNE x 3 che opengl deve renderizzare.
 * In realtà è una semplice copia della memoria grafica di chip8
 * però per ciascun valore 1/0 dei pixel, definisco i parametri R,G,B per quel pixel:
 * Se chip8.gfx[i][j] = 1 -> openGlScreen[i][j][0 = R] = 255, openGlScreen[i][j][1 = G] = 255, openGlScreen[i][j][2 = B] = 255
 * Ossia ho messo rosso 255, verde 255, blu 255 = BIANCO
 * Se li metto tutti a zero avrò il nero, taac
 */
unsigned char openGlScreen[GMEM_ROWS][GMEM_COLS][3];
unsigned char screenToRenderize[SCREEN_ROWS][SCREEN_COLS][3]; //espansione di openGlScreen

std::chrono::microseconds t0;

void keyPressCallback(unsigned char k, int x, int y) //signatura (:O) che vuole opengl
{
    static_cast<void>(x); // giusto per togliere gli warning unused variable
    static_cast<void>(y);
    int keycode = keymap(k);
    if(keycode != -1)
    {
        myChip8.onKeyPress(keycode);
    }
}

void keyReleaseCallback(unsigned char k, int x, int y)
{
    static_cast<void>(x);
    static_cast<void>(y);

    int keycode = keymap(k);
    if(keycode != -1)
    {
        myChip8.onKeyRelease(keycode);
    }
}

void renderChip8() //funzione che renderizza nella finestra opengl il contenuto della memoria grafica di chip8
{
    //traduco da gfx a openGlScreen
    for(int i = 0; i < GMEM_ROWS; i++)
    {
        for(int j = 0; j < GMEM_COLS; j++)
        {
            unsigned char col = myChip8.gfx[i][j] == 1 ? 0xFF : 0x00; //questo pixel deve essere bianco o nero
            openGlScreen[GMEM_ROWS - 1 - i][j][0] = openGlScreen[GMEM_ROWS - 1 - i][j][1] = openGlScreen[GMEM_ROWS - 1 - i][j][2] = col;
        }
    }

    //espando openGlScreen di PIXEL_SIZE e renderizzo questo
    unsigned int act_row = 0;
    unsigned int act_col = 0;
    for(int i = 0; i < GMEM_ROWS; i++)
    {
        for(int j = 0; j < GMEM_COLS; j++)
        {
            unsigned char colour = openGlScreen[i][j][0];
            for(int k = 0; k < PIXEL_SIZE; k++)
            {
                for(int l = 0; l < PIXEL_SIZE; l++)
                {
                    screenToRenderize[act_row + k][act_col + l][0] = screenToRenderize[act_row + k][act_col + l][1] = screenToRenderize[act_row + k][act_col + l][2] = colour;
                    screenToRenderize[act_row + l][act_col + k][0] = screenToRenderize[act_row + l][act_col + k][1] = screenToRenderize[act_row + l][act_col + k][2] = colour;
                }
            }
            act_col += PIXEL_SIZE;
        }
        act_col = 0;
        act_row += PIXEL_SIZE;
    }

    glDrawPixels(SCREEN_COLS, SCREEN_ROWS, GL_RGB, GL_UNSIGNED_BYTE, (void *)screenToRenderize); //disegno screenToRenderize
    glutSwapBuffers();
}

void reshapeWindowCallback(GLsizei w, GLsizei h)
{
    //per ora è vuota poi ci si pensa, viene chiamata quando uno allarga o restringe la finestra
    memset((void*)screenToRenderize, 0x00, SCREEN_COLS * SCREEN_ROWS * 3);
    glDrawPixels(SCREEN_COLS, SCREEN_ROWS, GL_RGB, GL_UNSIGNED_BYTE, (void *)screenToRenderize);
    glutSwapBuffers();
}

void emulationLoop()
{
    double clockPeriod_s = 1.0 / myChip8.clockFreq_hz;
    unsigned long clockPeriod_us = static_cast<unsigned long>(clockPeriod_s * 1e6);
    std::chrono::microseconds t = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch());

    myChip8.emulateCycle();
    if(myChip8.drawFlag)
    {
        renderChip8();
        myChip8.drawFlag = false;
    }

    if((t.count() - t0.count()) >= clockPeriod_us)
    {
        myChip8.onTickElapsed();
        t0 = t;
    }

}

void setupOpengl(int argc, char** argv) //funzione di inizializzazione di opengl
{
    memset(openGlScreen, 0x00, GMEM_COLS * GMEM_ROWS);
    memset(screenToRenderize, 0x00, SCREEN_COLS * SCREEN_ROWS);
    glClear(GL_COLOR_BUFFER_BIT);

    glutInit(&argc, argv); //passo a opengl eventuali argomenti da tastiera
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); //modo standard per inizializzare un display mode che va bene quasi sempre

    glutInitWindowSize(SCREEN_COLS, SCREEN_ROWS); //creo una finestra SCREEN_COLS * SCREEN_ROWS
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
    printf("diocane");
    setupOpengl(argc, argv);
    //setupInput();
    myChip8.initialize();
    myChip8.loadRom(argc > 1 ? argv[argc - 1] : "./test/test.ch8");

    t0 = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch());
    glutMainLoop(); //lancio l'emulatore attraverso l'esecuzione della mainloop di opengl

    return 0;
}
