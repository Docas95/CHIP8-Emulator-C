# Makefile for CHIP-8 Emulator Project

CC = gcc
CFLAGS = -g
OBJS = main.o
TARGET = Chip8
LIBS = -lSDL2 -lSDL2_mixer

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LIBS)

main.o: main.c main.h
	$(CC) $(CFLAGS) -c main.c
clean:
	rm -f *.o $(TARGET)

run: $(TARGET)
	./$(TARGET)
