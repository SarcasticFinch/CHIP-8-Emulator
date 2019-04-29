#include "CHIP8.hpp"
#include <iostream>
#include <fstream>

void chip8::initialize()
//Sets the CHIP-8 to the default values
{
    PC      = 0x200; //Program counter should start at 0x200
    opcode  = 0;    //Reset opcode
    I       = 0;    //Reset I
    sp      = 0;    //Reset stack pointer

    //Clear display

    //Clear stack
    for(int i = 0; i < 16; i++){
        stack[i] = 0;
    }

    //Clear registers V0-VF
    for(int i = 0; i < 16; i++){
        v[i] = 0;
    }

    //Clear memory
    for(int i = 0; i < 4096; i++){
        memory[i] = 0;
    }

    //Load fontset into first 0x50 bytes of memory
    for(int i = 0; i < 80; i++){
        memory[i] = chip8Fontset[i]
    }

    //Reset Timers
    delayTimer = 0;
    soundTimer = 0;
}

void chip8::loadRom()
//Loads the file based on filename and stores it in memory above 0x200(512)
{
    int BUFFER_SIZE = 1024;
    FILE *source;
    unsigned char buffer[BUFFER_SIZE];
    source = fopen(romPath,"rb");

    if(source){
        int i = 0;
        while(!feof(source)){
            fread (buffer,1,BUFFER_SIZE,source);
            while (i < BUFFER_SIZE){
                memory[i + 512] = buffer[i];
                i++;
            }
        }
        fclose(source);
    }
}

void chip8::emulateCycle()
{
    // Fetch Opcode
    getOpcode();
    // Decode and Excecute Opcode
    runOpcode();

    // Update timers
    if(delayTimer > 0){
        delayTimer--;
    }
    if(soundTimer > 0){
        if(soundTimer == 1){
            printf("BEEP\n");
        }
        soundTimer--;
    }
}

void chip8::getOpcode()
//Gets the 16 bit opcode at the address pointed to by the program counter
//Stores it in opcode
{
    //Assume highorder byte is stored in lower memory
    //Get the first byte, leftshift 8 bits then bitwise OR to add the second byte
    opcode = memory[PC] << 8 | memory[PC+1];
}

void chip8::runOpcode()
//Interperets opcode and the runs the specified command
//Filters by nibble starting at highest
{
    //Preemptively get VX and VY if they are needed
    unsigned short x = (opcode & 0x0F00) >> 8;
    unsigned short y = (opcode & 0x00F0) >> 4;

    switch(opcode & 0xF000){ //Mask the highest nibble first
        case 0x0000: //0xxx opcodes
        switch(opcode & 0x000F){ //Mask lowest nibble
            case 0x0000: //00E0: Clear the screen
            for(int i = 0; i < 64; i++){
                for(int j = 0; j < 32; j++){
                    gfx[i*64+j] = 0x0000;
                }
            }
            drawFlag = 1;
            break;

            case 0x000E: //00EE: Returns from subroutine
            PC = stack[sp]; //Set program counter to stack value
            sp--;
            break;

            //0NNN opcode not supported due to need for RCA 1802
            default:
            printf ("Unknown opcode: 0x%X\n", opcode);
        }
        break;

        case 0x1000: //1NNN: Jump to address NNN
        PC = opcode & 0xFFF;
        PC -= 2; //Account for the increment after excecution (see bottom of func)
        break;

        case 0x2000: //2NNN: Runs subroutine at NNN
        sp++;
        stack[sp] = PC;       //Store program counter in stack
        PC = opcode & 0x0FFF;  //Jump to address NNN
        PC -= 2; //Account for the increment after excecution (see bottom of func)
        break;

        case 0x3000: //3XNN: Skips the next instruction if VX == NN
        if(V[x] == opcode & 0x00FF){
            PC += 2;
        }
        break;

        case 0x4000: //4XNN: Skips the next instruction if VX != NN
        if(V[x] != opcode & 0x00FF){
            PC += 2;
        }
        break;

        case 0x5000: //5XY0: Skips the next instruction if VX == VY
        if(V[x] == V[y]){
            PC += 2;
        }
        break;

        case 0x6000: //6XNN: Sets VX = NN
        V[x] = opcode & 0x00FF;
        break;

        case 0x7000: //7XNN: Adds VX += NN does not affect carry flag
        V[x] += opcode & 0x00FF;
        break;

        case 0x8000: //8xxx opcodes
        switch(opcode & 0x000F){ //Mask lowest nibble
            case 0x0000: //8XY0: Sets VX = VY
            V[x] = V[y];
            break;

            case 0x0001: //8XY1: Sets VX = VX OR VY
            V[x] = V[x] | V[y];
            break;

            case 0x0002: //8XY2: Sets VX = VX AND VY
            V[x] = V[x] & V[y];
            break;

            case 0x0003: //8XY3: Sets VX = VX XOR VY
            V[x] = V[x] ^ V[y];
            break;

            case 0x0004: //8XY4: Sets VX += VY, VF set to 1 when theres a carry
            if (V[y] > (0xFF - V[x] )){
                //if VY > FF-VX, overflow
                V[16] = 1;
            }
            else{
                V[16] = 0;
            }
            V[x] += V[y];
            break;

            case 0x0005: //8XY5: Sets VX -= VY, VF set to 0 when theres a borrow
            if (V[x] > V[y] ){
                //if VX > VY, no overflow
                V[16] = 1;
            }
            else{
                V[16] = 0;
            }
            V[x] -= V[y];
            break;

            case 0x0006: //8XY6: Rotates VX right through VF
            V[16] = V[x] & 0x0001;  //VF = lowest bit
            V[x] = V[x] >> 1;
            break;

            case 0x0007: //8XY7: Sets VX = VY-VX
            if (V[y] > V[x] ){
                //if VY > VX, no overflow
                V[16] = 1;
            }
            else{
                V[16] = 0;
            }
            V[x] = V[y] - V[x];
            break;

            case 0x000E: //8XYE: Rotates VX left through VF
            V[16] = (V[x] & 0x0080) >> 7;  //VF = high bit shfted 7 right
            V[x] = V[x] << 1;
            break;

            default:
            printf ("Unknown opcode: 0x%X\n", opcode);
        }
        break;

        case 0x9000: //9XY0: Skips the next instruction if VX != VY
        if(V[x] != V[y]){
            PC += 2;
        }
        break;

        case 0xA000: //ANNN: Sets I = NNN
        I = opcode & 0x0FFF;
        break;

        case 0xB000: //BNNN: Jumps to address NNN plus V0
        PC = (opcode & 0xFFF) + V[0];
        PC -= 2; //Account for the increment after excecution (see bottom of func)
        break;

        case 0xC000: //CXNN: Sets VX = rand(0-255) & NN
        V[x] = (rand() % 255) & (opcode & 0x00FF)
        break;

        case 0xD000: //DXYN: Draws sprite in I at (VX, VY) with width 8px height Npx
        //Get coordinates
        x = V[x];
        y = V[y];
        unsigned short height = opcode & 0x000F;
        unsigned short pixel;

        V[16] = 0;      //Clear carry

        for (int yline = 0; yline < height; yline++){
            pixel = memory[I + yline];  //Get the current line of pixels
            for (int xline = 0; xline < 8; xline++) {
                if ((pixel & 0x80 >> xline) != 0) {
                    //If the current pixel along x needs to be drawn
                    if (gfx[(x + xline + ((y + yline) * 64))] == 1) {
                        //If the current pixel in gfx is on, set VF before turning off
                        V[16] = 1;
                    }
                    gfx[(x + xline + ((y + yline) * 64))] ^= 1; //XOR inverts gfx
                    //0 XOR 1 = 1
                    //1 XOR 1 = 0
                }
            }
        }
        drawFlag = 1;
        break;

        case 0xE000: //Exxx opcodes
        switch(opcode & 0x000F){ //Mask lowest nibble
            case 0x000E: //EX9E: Skips next instruction if the key in VX is pressed
            if(key[V[x]] == 1){
                PC += 2;
            }
            break;

            case 0x0001: //EXA1: Skips next instruction if the key in VX isnt pressed
            if(key[V[x]] == 0){
                PC += 2;
            }
            break;

            default:
            printf ("Unknown opcode: 0x%X\n", opcode);
        }
        break;

        case 0xF000: //Fxxx opcodes
        switch(opcode & 0x00FF){ //Mask lower 2 nibbles
            case 0x0007: //FX07: Sets VX = delayTimer
            V[x] = delayTimer;
            break;

            case 0x000A: //FX0A: Sets VX = keypress, waits for keypress
            for (int i = 0; i < 16; i++){
                if (key[i] == 1){
                    V[x] = i;
                }
                else{
                    PC -= 2; //If no keypress, stay at instruction and check again
                }
            }
            break;

            case 0x0015: //FX15: Sets delayTimer = VX
            delayTimer = V[x];
            break;

            case 0x0018: //FX18: Sets soundTimer = VX
            soundTimer = V[x];
            break;

            case 0x001E: //FX1E: Sets I += VX
            I += V[x];
            break;

            case 0x0029: //FX29: Sets I = location of sprite for character in VX
            //[RMV] Do this
            break;

            case 0x0033: //FX33: Stores the BCD of VX in I+0 I+1 I+2 (100s, 10s, 1s)
            memory[I + 0] = V[x] / 100;
            memory[I + 1] = (V[x] /10) % 10;
            memory[I + 2] = (V[x] % 100) % 10;
            break;

            case 0x0055: //FX55: Dumps V0-VX inclusive into memory starting at
            //      address I, does not modify I
            int n = (opcode & 0x0F00) >> 8;
            for(int i = 0; i < n; i++){
                memory[(I + i)] = V[i]
            }
            break;

            case 0x0065: //FX65: Fills V0-VX inclusive with memory starting at
            //      address I, does not modify I
            int n = (opcode & 0x0F00) >> 8;
            for(int i = 0; i < n; i++){
                V[i] = memory[(I + i)]
            }
            break;

            default:
            printf ("Unknown opcode: 0x%X\n", opcode);
        }
        break;

        default:
        printf ("Unknown opcode: 0x%X\n", opcode);
    }
    //After excecuting opcode, increment program counter by 2 bytes
    PC += 2;
}
