#include "main.h"


// global vars

struct Chip8 chip;

SDL_Window* window;
SDL_Renderer* renderer;
SDL_Rect square;

int main(int argc, char* argv[]){
	
	if(argc == 2){
		// set up CHIP8 data
		init_chip();
		srand(time(NULL));	
	} else {
		printf("Correct program usage: ./Chip8 <rom_file> <version (8/48)>\n");
		exit(1);
	}

	// read ROM data into memory
	load_ROM(argv[1]);


	//set up SDL
	init_SDL();

	int i;
	// emulator main loop
	while(!chip.input[QUIT]){

		// get user input
		get_user_input();

		// decrement timers
		if(chip.delay > 0) chip.delay--;
		if(chip.sound > 0) chip.sound--;

		// decode 11 instructions
		i = 0;
		while(i < 11){
			// fetch intruction
			fetch_instruction();

			// limit drawing instructions to one per frame
			if((chip.op_code & 0xF000) == 0xD000 && chip.draw_wait){
				chip.pc -= 2;
				chip.draw_wait = 0;
				break;
			}

			// decode and execute instruction
			decode_instruction();
			i++;
		}

		// draw on screen		
		if(chip.draw_flag){
			draw();
			chip.draw_flag = 0;
		}
		// delay to emulate CHIP8 internal clock
		SDL_Delay(16);
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

	// set flags
	chip.draw_flag = 0;

	// point stack pointer to bottom of stack
	chip.stack_pointer = 0;

	// no key is pressed by default
	for(int i = 0; i < 16; i++){
		chip.input[i] = 0;
	}

	chip.delay = 0;
	chip.sound = 0;
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
					memset(chip.display, 0, COLUMNS * ROWS - 1);
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
					chip.registers[0xF] = 0;
					break;
				case 0x0002:
					// set VX to VX AND VY
					chip.registers[X] &= chip.registers[Y];
					chip.registers[0xF] = 0;
					break;
				case 0x0003:
					// set VX to VX XOR VY
					chip.registers[X] ^= chip.registers[Y];
					chip.registers[0xF] = 0;
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
					// shift VX one bit to the right
					chip.registers[X] = chip.registers[Y];
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
					// shift VX one bit to the left
					chip.registers[X] = chip.registers[Y];
					oldX = chip.registers[X];
					chip.registers[X] <<= 1;

					// store shifted bit in VF
					chip.registers[0xF] = (oldX >> 7) & 1;
					break;
				default:
					// jump with offset
					chip.pc = NNN + chip.registers[X];
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
			chip.pc = NNN + chip.registers[0x0];
			break;
		case 0xC000:
			// store random number in VX
			chip.registers[X] = (rand() % NN) & NN;
			break;
		case 0xD000:{
			// display/draw
			chip.draw_wait = 1;
			uint16_t x = chip.registers[X] % 64;
			uint16_t y = chip.registers[Y] % 32;
			uint16_t height = N;
			uint8_t pixel;
			
			chip.registers[0xF] = 0;
			for(uint row = 0; row < height; row++){
				pixel = chip.memory[chip.index + row];
				if(y + row > ROWS) break;
				for(uint col = 0; col < 8; col++){
					if(x + col > COLUMNS) break;
					
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
					if(chip.input[chip.registers[X]])
						chip.pc += 2;
					break;
				case 0x0001:
					// skip if VX key is not pressed
					if(!chip.input[chip.registers[X]])
						chip.pc += 2;
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
					// wait for keypress
				    {
				        int key_found = 0;
				        for(int i = 0; i < 16; i++) {
				            if(chip.input[i] != 0) {
				                chip.registers[X] = i;
				                chip.input[i] = 0;
				                key_found = 1;
				                break;
				            }
				        }
				        if(!key_found) {
				            chip.pc -= 2;
				        }
				    }
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
						chip.memory[chip.index] = chip.registers[i];
						chip.index++;
					}
					break;
				case 0x0065:
					// load values from memory into registers V0 to VX (inclusive)
					for(uint i = 0; i <= X; i++){
						chip.registers[i] = chip.memory[chip.index];
						chip.index++;
					}
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
				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
			} 
			SDL_RenderFillRect(renderer, &square);
		}
	}
	SDL_RenderPresent(renderer);
}

void get_user_input(){
    SDL_Event e;
    SDL_Keycode keycode;
    while(SDL_PollEvent(&e) != 0){
        keycode = e.key.keysym.sym;

        switch(e.type){
            case(SDL_KEYDOWN):
                switch(keycode){
                    case SDLK_1:
                        chip.input[0x1] = 1;
                        break;
                    case SDLK_2:
                        chip.input[0x2] = 1;
                        break;
                    case SDLK_3:
                        chip.input[0x3] = 1;
                        break;
                    case SDLK_4:
                        chip.input[0xC] = 1;
                        break;
                    case SDLK_q:
                        chip.input[0x4] = 1;
                        break;
                    case SDLK_w:
                        chip.input[0x5] = 1;
                        break;
                    case SDLK_e:
                        chip.input[0x6] = 1;
                        break;
                    case SDLK_r:
                        chip.input[0xD] = 1;
                        break;
                    case SDLK_a:
                        chip.input[0x7] = 1;
                        break;
                    case SDLK_s:
                        chip.input[0x8] = 1;
                        break;
                    case SDLK_d:
                        chip.input[0x9] = 1;
                        break;
                    case SDLK_f:
                        chip.input[0xE] = 1;
                        break;
                    case SDLK_z:
                        chip.input[0xA] = 1;
                        break;
                    case SDLK_x:
                        chip.input[0x0] = 1;
                        break;
                    case SDLK_c:
                        chip.input[0xB] = 1;
                        break;
                    case SDLK_v:
                        chip.input[0xF] = 1;
                        break;
                    case SDLK_ESCAPE:
                        chip.input[QUIT] = 1; // Handle quit key
                        break;
                }
                return;
            case(SDL_KEYUP):
                switch(keycode){
                    case SDLK_1:
                        chip.input[0x1] = 0;
                        break;
                    case SDLK_2:
                        chip.input[0x2] = 0;
                        break;
                    case SDLK_3:
                        chip.input[0x3] = 0;
                        break;
                    case SDLK_4:
                        chip.input[0xC] = 0;
                        break;
                    case SDLK_q:
                        chip.input[0x4] = 0;
                        break;
                    case SDLK_w:
                        chip.input[0x5] = 0;
                        break;
                    case SDLK_e:
                        chip.input[0x6] = 0;
                        break;
                    case SDLK_r:
                        chip.input[0xD] = 0;
                        break;
                    case SDLK_a:
                        chip.input[0x7] = 0;
                        break;
                    case SDLK_s:
                        chip.input[0x8] = 0;
                        break;
                    case SDLK_d:
                        chip.input[0x9] = 0;
                        break;
                    case SDLK_f:
                        chip.input[0xE] = 0;
                        break;
                    case SDLK_z:
                        chip.input[0xA] = 0;
                        break;
                    case SDLK_x:
                        chip.input[0x0] = 0;
                        break;
                    case SDLK_c:
                        chip.input[0xB] = 0;
                        break;
                    case SDLK_v:
                        chip.input[0xF] = 0;
                        break;
                    case SDLK_ESCAPE:
                        chip.input[QUIT] = 0; // Handle quit key
                        break;
                }
                return;
        }
    }
}
