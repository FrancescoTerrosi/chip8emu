#pragma once
#ifndef CHIP8_H
#define CHIP8_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

const auto MEM_SIZE = 4096;
const auto N_REG =  16;
const auto STACK_SIZE = 16;
const auto N_KEYS = 16;
const auto GMEM_ROWS = 64;
const auto GMEM_COLS = 32;

class Chip8
{
public:
    Chip8();

	void initialize();
    bool loadRom(const char* filename);
    void emulateCycle();
    void onKeyPress(int keycode);
    void onKeyRelease(int keycode);

    const unsigned int clockFreq_hz = 60;

    unsigned char chip8_fontset[80] =
    {
      0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
      0x20, 0x60, 0x20, 0x20, 0x70, // 1
      0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
      0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
      0x90, 0x90, 0xF0, 0x10, 0x10, // 4
      0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
      0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
      0xF0, 0x10, 0x20, 0x40, 0x40, // 7
      0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
      0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
      0xF0, 0x90, 0xF0, 0x90, 0x90, // A
      0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
      0xF0, 0x80, 0x80, 0x80, 0xF0, // C
      0xE0, 0x90, 0x90, 0x90, 0xE0, // D
      0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
      0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

    unsigned short instruction;                      // 2 byte per le operazioni macchina
    unsigned short opcode;                           // Quì salvo opcode (per cui vale opcode = instruction & 0xF000) <-- il msb
    unsigned char x;                                 // Quì salvo 1 byte per operando x
    unsigned char y;                                 // Quì salvo 1 byte per operando y
    unsigned char n;                                 // Quì salvo 1 byte per operando n
    unsigned char kk;                                // Quì salvo 1 byte per operando kk
    unsigned short nnn;                              // Quì salvo 2 byte per operando nnn
    unsigned char memory[MEM_SIZE];                  // 4k per la memoria
    unsigned char V[N_REG];                          // 16 registri CPU da 1 byte ciascuno - V[15] NON USARE!
    unsigned char key[N_KEYS];
    unsigned short stack[STACK_SIZE];                // 16 livelli da 2 byte per lo stack
    unsigned char gfx[GMEM_ROWS][GMEM_COLS];         // 64*32 byte per la memoria grafica

    unsigned short I;                   // Index register
    unsigned short pc;                  // Program Counter
    unsigned short sp;                  // Stack pointer

    unsigned char delay_timer;
    unsigned char sound_timer;
    unsigned short drawFlag;
    unsigned char drawRow;
    unsigned char oldRow;

private:
    bool flipFlag;
    void checkFlip(unsigned char oldRow, unsigned char drawRow);
};

#endif //CHIP8_H
