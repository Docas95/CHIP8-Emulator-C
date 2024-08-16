# CHIP8-Emulator-C
An emulator for the CHIP-8 virtual machine.

Most information about CHIP-8, including the technical information used to create this emulator, can be found [here](https://en.wikipedia.org/wiki/CHIP-8).

## How to build

### Requisites
- GCC (GNU Compiler Collection)
- Make
- SDL2
- SDL2_Mixer 

### Compile

```
terminal> cd <project_location>
terminal> make
```

### Execute

```
terminal> ./Chip8 <rom_location>
```
## How to play

### Controls:
The original CHIP 8 uses the following controls:

1 | 2 | 3 | C 

4 | 5 | 6 | D

7 | 8 | 9 | E

A | 0 | B | F

These are the equivalente controls for a QWERTY keyboard for this emulator:
1 | 2 | 3 | 4

Q | W | E | R

A | S | D | F

Z | X | C | V

## ROMs
All the ROMs in the "roms" folder have been tested and seem to work properly.

The ROMs numbered from 1 to 7 are "test ROMs" designed to check if the emulator was coded properly. I found those ROMs [here](https://github.com/Timendus/chip8-test-suite).

The ROMs in capslock (TETRIS, PONG, etc) are normal game ROMs which can be played. I found those ROMs [here](https://www.zophar.net/pdroms/chip8.html).

