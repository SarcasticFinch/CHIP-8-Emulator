#ifndef __CHIP8_H_
#define __CHIP8_H_
//-----------------------------------------------------------------------------
//CHIP-8 Emulator
//-----------------------------------------------------------------------------
//Emulates a CHIP-8 OS: https://en.wikipedia.org/wiki/CHIP-8
//This project attempts to emulate the original CHIP-8, as such there are few
//deviations from the original implementation of the CHIP-8
//Designed by Brendan Souksmlane 2019
//-----------------------------------------------------------------------------
#include <iostream>

class chip8
{
public:
    //Hardware

    //Keypad
    //-----------------------------------------------------------------------------
    //A HEX keypad is implemented in the CHIP-8:
    // 1 2 3 C
    // 4 5 6 D
    // 7 8 9 E
    // A 0 B F
    //-----------------------------------------------------------------------------
    unsigned char key[16];

    //Graphics
    //-----------------------------------------------------------------------------
    //The CHIP-8 only renders in black and white and has 64x32 resolution
    //Pixels are changed using XOR; when pixels are erased the VF flag is set
    //-----------------------------------------------------------------------------
    unsigned char gfx[64 * 32];

    //Timers
    //-----------------------------------------------------------------------------
    //There are two 60hz timers that count down to 0
    //The system's buzzer sounds when the sound timer reaches 0
    //-----------------------------------------------------------------------------
    unsigned char delayTimer;
    unsigned char soundTimer;

private:
    //CPU

    //opcodes
    //-----------------------------------------------------------------------------
    //The chip 8 has 35 opcodes, all 2bytes.
    //-----------------------------------------------------------------------------
    unsigned short opcode;

    //Memory
    //-----------------------------------------------------------------------------
    //There are 4096 (0x1000) 8bit memory locations on most CHIP-8 systems
    //Most programs begin at memory location 512(0x200) due to the interpereter
    //occupying the first 512 bytes.
    //0x000-0x1FF - CHIP-8 interpereter
    //0x050-0x0A0 - Used for the built in 4x5 pixel font set (0-F)
    //0x200-0xFFF - Program/GP RAM
    //-----------------------------------------------------------------------------
    unsigned char memory[4096];

    //Registers
    //-----------------------------------------------------------------------------
    //There are 16 8bit registers from V0-V1F. VF is used as a carry flag often
    //There is an address register I, which is 16bit and is used for memory opcodes
    //There is also a 16 bit program counter
    //-----------------------------------------------------------------------------
    unsigned char V[16];
    unsigned short I;
    unsigned short PC;

    //Stack
    //-----------------------------------------------------------------------------
    //Most implementations of the stack only have 16 levels.
    //Most often used for jumps
    //-----------------------------------------------------------------------------
    unsigned short stack[16];
    unsigned short sp;

    //Fontset
    //-----------------------------------------------------------------------------
    //CHIP-8 Fontset used to display text from 0-F using opcode FX29
    //-----------------------------------------------------------------------------
    unsigned char chip8Fontset[80] =
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

    bool drawFlag;  //Lets gfx know to redraw the screen
    std::string romPath;

public:

    //Sets the romPath, loads the rom, and initializes the registers
    void initialize();
    //Loads the rom in romPath to memory
    void loadRom();
    void emulateCycle();
    void getOpcode();
    void runOpcode();
};

#endif
