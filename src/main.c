#include <stdio.h>

struct Chip8{
	int8_t registers[16];		// 16 8-bit registers 
	int8_t  memory[4096]; 		// 4096 bytes of memory
	int16_t index_register; 	// 16 bit index register
	int16_t program_counter; 	// 16 bit program counter
	int16_t stack[16]; 		// 16 level stack
	int8_t stack_pointer; 		// 8 bit stack pointer
	int8_t delay_timer; 		// 8 bit delay timer
	int8_t sound_timer; 		// 8 bit sound timer
	int8_t input_keys[16]; 		// 16 input keys
	int32_t display_memory[32][64]; // graphics memory, 32 bits high, 64 bits wide
	
};

int main(void){

	return 0;
}
