#ifndef MAIN_H
#define MAIN_H
// includes

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

// macros
#define FONT_SIZE 80
#define FONT_START_ADDRESS 0x50
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 320
#define ROWS 32
#define COLUMNS 64
#define ROM_START_ADDRESS 0x200
#define NO_KEYPRESS 255
#define QUIT 254
#define AUDIO_LOCATION "../sound/pluck.mp3" 

// data structures
struct Chip8{
        uint8_t memory[4096];
	uint8_t display[ROWS * COLUMNS];
	uint16_t pc;
	uint16_t index; 
	uint16_t stack[16];
	uint8_t stack_pointer;
	uint8_t delay;
	uint8_t sound;
	uint8_t registers[16];
	uint16_t op_code;
	uint8_t draw_flag;
	uint8_t draw_wait;
	uint8_t input[16];
};

// constants
const uint8_t fontset[FONT_SIZE] = {
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

// prototypes
void init_chip();
void load_ROM(char* filename);
void init_SDL();
void quit_SDL();
void fetch_instruction();
void decode_instruction();
void draw();
void get_user_input();
#endif
