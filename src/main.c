#include "main.h"


// global vars

struct Chip8 chip;

SDL_Window* window;
SDL_Renderer* renderer;
SDL_Rect square;

int main(int argc, char* argv[]){
	
	if(argc >= 2){
		// set up CHIP8 data
		init_chip();	
	} else {
		printf("Correct program usage: ./Chip8 <rom_file>\n");
		exit(1);
	}

	// read ROM data into memory
	load_ROM(argv[1]);


	//set up SDL
	init_SDL();

	// emulator main loop
	while(1){
		// fetch intruction
		fetch_instruction();

		// decode and execute instruction
		decode_instruction();
			
		// draw on screen		
		if(chip.draw_flag){
			draw();
			chip.draw_flag = 0;
		}
	}

	quit_SDL();
	return 0;
}

// set up CHIP8 data
void init_chip(){
	// import fontset to memory
	for(int i = FONT_START_ADDRESS; i < FONT_SIZE; i++){
		chip.memory[i] = fontset[i];	
	}

	// place program counter at first ROM intruction
	chip.pc = ROM_START_ADDRESS;

	// set draw flag
	chip.draw_flag = 0;
}

// load content from ROM into memory
void load_ROM(char* filename){
	FILE* f = fopen(filename, "rb");
	if(!f){
		printf("Error opening file!\n");
		exit(1);
	}

	fseek(f, 0L, SEEK_END);
	size_t size = ftell(f);
	fseek(f, 0L, SEEK_SET);

	fread(&chip.memory[ROM_START_ADDRESS], 1, size, f);

	fclose(f);
}

void init_SDL(){
	// init SDL
	if(SDL_Init(SDL_INIT_VIDEO) < 0){
                printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
                exit(1);
        }

	// Create window
	window = SDL_CreateWindow("CHIP8 Emu", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);	 
	if(!window){
		printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		SDL_Quit();
		exit(1);
	}

	// Create renderer
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if(renderer  == NULL){
		printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
		SDL_DestroyWindow(window);
		SDL_Quit();
		exit(1);
	}
	
	// define square	
	square.w = 10;
	square.h = 10;
	square.x = 0;
	square.y = 0;
	
	// clear screen
	SDL_RenderClear(renderer);
}

void quit_SDL(){
	SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
}

// fetch current instruction
void fetch_instruction(){
	chip.op_code = 0;
	chip.op_code = chip.memory[chip.pc] << 8 | chip.memory[chip.pc+1];
	chip.pc+=2;	
}

// decode and execute instruction from given opcode
void decode_instruction(){
	// opcode -> NNNN (each N is 4-bits)
	// first N -> type of instruction
	// second N -> register num
	// third N -> register num
	// fourth N -> 4-bit num
	// third + fourth N -> 8-bit num
	// second + third + fourth N -> 12-bit memory address
	int16_t secondN = (0x0F00 & chip.op_code) >> 8;
	int16_t thirdN = (0x00F0 & chip.op_code) >> 4;
	int16_t fourthN = 0x000F & chip.op_code;
	int16_t doubleN = 0x00FF & chip.op_code;
	int16_t tripleN = 0x0FFF & chip.op_code;

	switch(0xF000 & chip.op_code){
		case 0x0000:
			switch(0x00FF & chip.op_code){
				case 0x00E0:
					// clear screen
					memset(chip.display, 0, COLUMNS);
					chip.draw_flag = 1;
				break;
			}
			break;
		case 0x1000:
			// jump
			chip.pc = tripleN;			
			break;
		case 0x2000:
			break;
		case 0x3000:
			break;
		case 0x4000:
			break;
		case 0x5000:
			break;
		case 0x6000:
			// set register 
			chip.registers[secondN] = (int8_t) doubleN;
			break;
		case 0x7000:
			// add value to register VX
			chip.registers[secondN] += (int8_t) doubleN;
			break;
		case 0x8000:
			break;
		case 0x9000:
			break;
		case 0xA000:
			// set index
			chip.index = tripleN;
			break;
		case 0xB000:
			break;
		case 0xC000:
			break;
		case 0xD000:
			// display/draw
			int16_t x = chip.registers[secondN];
			int16_t y = chip.registers[thirdN];
			int16_t height = fourthN;
			int8_t pixel;
			
			chip.registers[0xF] = 0;
			for(int row = 0; row < height; row++){
				pixel = chip.memory[chip.index + row];
				for(int col = 0; col < 8; col++){
					if((pixel & (0x80 >> col)) != 0){
						if(chip.display[(x + col + ((y + row) * 64))] == 1){
							chip.registers[0xF] = 1;
						}
						chip.display[x +  col + ((y + row) * 64)] ^= 1;	
					}
				}
			}
			chip.draw_flag = 1;	
			break;
		case 0xE000:
			break;
		case 0xF000:
			break;
		default:
			break;
	}
}

void draw(){
	for(int y = 0; y < ROWS; y++){
		for(int x = 0; x < COLUMNS; x++){
			square.x = x * 10;
			square.y = y * 10;
			if(chip.display[y * 64 + x]){
				SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
			} else {
				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
			} 
			SDL_RenderFillRect(renderer, &square);
		}
	}
	SDL_RenderPresent(renderer);
}
