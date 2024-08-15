#include "main.h"


// global vars

struct Chip8 chip;

SDL_Window* window;
SDL_Renderer* renderer;
SDL_Rect square;

int main(int argc, char* argv[]){
	
	if(argc == 3){
		// set up CHIP8 data
		init_chip();
		srand(time(NULL));	
	} else {
		printf("Correct program usage: ./Chip8 <rom_file> <version (8/48)>\n");
		exit(1);
	}

	// get CHIP version (8 or 48)
	// handles "ambiguous" instructions
	chip.old_flag = !strcmp("8", argv[2]);

	// read ROM data into memory
	load_ROM(argv[1]);


	//set up SDL
	init_SDL();

	// emulator main loop
	int i = 0;
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

		// get user input
		// TO-DO

		// delay to emulate CHIP8 internal clock
		SDL_Delay(1);
		i++;
		if(i > 1200) break;
	}

	// close SDL
	quit_SDL();
	return 0;
}

// set up CHIP8 data
void init_chip(){
	// import fontset to memory
	for(uint i = FONT_START_ADDRESS; i < FONT_SIZE; i++){
		chip.memory[i] = fontset[i];	
	}

	// place program counter at first ROM intruction
	chip.pc = ROM_START_ADDRESS;

	// set draw flag
	chip.draw_flag = 0;

	// point stack pointer to bottom of stack
	chip.stack_pointer = 0;
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
	// register
	uint16_t X = (0x0F00 & chip.op_code) >> 8;
	// register
	uint16_t Y = (0x00F0 & chip.op_code) >> 4;
	// 4 bit num
	uint16_t N = 0x000F & chip.op_code;
	// 8 bit num
	uint16_t NN = 0x00FF & chip.op_code;
	// 12 bit address
	uint16_t NNN = 0x0FFF & chip.op_code;

	uint8_t oldX, oldY;
	switch(0xF000 & chip.op_code){
		case 0x0000:
			switch(0x00FF & chip.op_code){
				case 0x00E0:
					// clear screen
					memset(chip.display, 0, COLUMNS);
					chip.draw_flag = 1;
				break;
				case 0x00EE:
					// return from subroutine
					chip.pc = chip.stack[chip.stack_pointer-1];
					chip.stack_pointer--;				
				break; 
			}
			break;
		case 0x1000:
			// jump
			chip.pc = NNN;			
			break;
		case 0x2000:
			// call subroutine
			chip.stack[chip.stack_pointer] = chip.pc;
			chip.stack_pointer++;
			chip.pc = NNN;
			break;
		case 0x3000:
			// skip instruction if register VX = NNN
			if(chip.registers[X] == NN)
				chip.pc += 2;
			break;
		case 0x4000:
			// skip instruction if register VX != NN
			if(chip.registers[X] != NN)
				chip.pc += 2;
			break;
		case 0x5000:
			// skip instruction if register VX = register VY
			if(chip.registers[X] == chip.registers[Y])
				chip.pc += 2;
			break;
		case 0x6000:
			// set VX register 
			chip.registers[X] = (uint8_t) NN;
			break;
		case 0x7000:
			// add value to register VX
			chip.registers[X] += (uint8_t) NN;
			break;
		case 0x8000:
			switch(0x000F & chip.op_code){
				case 0x0000:
					// set VX to VY
					chip.registers[X] = chip.registers[Y];
					break;
				case 0x0001:
					// set VX to VX OR VY
					chip.registers[X] |= chip.registers[Y];
					break;
				case 0x0002:
					// set VX to VX AND VY
					chip.registers[X] &= chip.registers[Y];
					break;
				case 0x0003:
					// set VX to VX XOR VY
					chip.registers[X] ^= chip.registers[Y];
					break;
				case 0x0004:
					// set VX to VX + VY
					oldX = chip.registers[X];
					chip.registers[X] += chip.registers[Y];
					
					// if the addition overflows, set VF to 1
					if(((uint16_t) oldX + (uint16_t) chip.registers[Y]) > 0xFF)
						chip.registers[0xF] = 1;
					else
						chip.registers[0xF] = 0;

					break;
				case 0x0005:
					// set VX to VX - VY
					oldX = chip.registers[X];
					chip.registers[X] -= chip.registers[Y];

					// if the subtraction underflows, set VF to 0
					if(oldX < chip.registers[Y])
						chip.registers[0xF] = 0;
					else
						chip.registers[0xF] = 1;

					break;
				case 0x0006:
					if(chip.old_flag){
						chip.registers[X] = chip.registers[Y];
					}
					// shift VX one bit to the right
					oldX = chip.registers[X];
					chip.registers[X] >>= 1;

					// stored shifted bit in VF
					chip.registers[0xF] = oldX & 1;
					break;
				case 0x0007:
					// set VX to VY - VX
					oldY = chip.registers[Y];
					chip.registers[X] = chip.registers[Y] - chip.registers[X];
					
					// if the subtraction underflows, set VF to 0
					if(oldY < chip.registers[X])
						chip.registers[0xF] = 0;
					else
						chip.registers[0xF] = 1;

					break;
				case 0x000E:
					if(chip.old_flag){
						chip.registers[X] = chip.registers[Y];
					}
					// shift VX one bit to the left
					oldX = chip.registers[X];
					chip.registers[X] <<= 1;

					// store shifted bit in VF
					chip.registers[0xF] = (oldX >> 7) & 1;
					break;
				default:
					// jump with offset
					if(chip.old_flag) chip.pc = NNN + chip.registers[0];
					else chip.pc = NNN + chip.registers[X];
					break;
			}
			break;
		case 0x9000:
			// skip instruction if register VX != register VY
			if(chip.registers[X] != chip.registers[Y])
				chip.pc += 2;
			break;
		case 0xA000:
			// set index
			chip.index = NNN;
			break;
		case 0xB000:
			break;
		case 0xC000:
			// store random number in VX
			chip.registers[X] = (rand() % NN) & NN;
			break;
		case 0xD000:{
			// display/draw
			uint16_t x = chip.registers[X];
			uint16_t y = chip.registers[Y];
			uint16_t height = N;
			uint8_t pixel;
			
			chip.registers[0xF] = 0;
			for(uint row = 0; row < height; row++){
				pixel = chip.memory[chip.index + row];
				for(uint col = 0; col < 8; col++){
					if((pixel & (0x80 >> col)) != 0){
						if(chip.display[(x + col + ((y + row) * 64))] == 1){
							chip.registers[0xF] = 1;
						}
						chip.display[x +  col + ((y + row) * 64)] ^= 1;	
					}
				}
			}
			chip.draw_flag = 1;
			}	
			break;
		case 0xE000:
			switch(0x000F & chip.op_code){
				case 0x000E:
					// skip if VX key is pressed
					//chip.pc += 2;
					break;
				case 0x0001:
					// skip if VX key is not pressed
					//chip.pc += 2;
					break;
			}
			break;
		case 0xF000:
			switch(0x00FF & chip.op_code){
				case 0x0007:
					// set VX to delay timer
					chip.registers[X] = chip.delay;
					break;
				case 0x0015:
					// set delay timer to VX
					chip.delay = chip.registers[X];
					break;
				case 0x0018:
					// set sound timer to VX
					chip.sound = chip.registers[X];
					break;
				case 0x001E:
					// add VX to index
					chip.index += chip.registers[X];
					break;
				case 0x000A:
					// the instructions blocks until a key is pressed
					// decrease pc 
					// store key value in vX
					break;
				case 0x0029:
					// set index to font address of hexadecimal character in VX
					chip.index = chip.registers[X] * 5; 	
					break;
				case 0x0033:
					// store the 3 digits from VX in memory pointed by index
					chip.memory[chip.index+2] = chip.registers[X] % 10;
					chip.memory[chip.index+1] = (chip.registers[X] / 10) % 10;
					chip.memory[chip.index] = (chip.registers[X] / 100) % 10;
					break;
				case 0x0055:
					// store register V0 to VX (inclusive) in memory
					for(uint i = 0; i <= X; i++){
						chip.memory[chip.index + i] = chip.registers[i];
					}
					if(chip.old_flag) chip.index += X + 1;
					break;
				case 0x0065:
					// load values from memory into registers V0 to VX (inclusive)
					for(uint i = 0; i <= X; i++){
						chip.registers[i] = chip.memory[chip.index + i];
					}
					if(chip.old_flag) chip.index += X + 1;
					break;
			}
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
