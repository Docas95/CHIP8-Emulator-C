# Makefile for CHIP-8 Emulator Project

CC = gcc
CFLAGS = -g
OBJS = main.o
TARGET = Chip8
LIBS = -lSDL2

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

main.o: main.c main.h
	$(CC) $(CFLAGS) -c main.c $(LIBS)
clean:
	rm -f *.o $(TARGET)

run: $(TARGET)
	./$(TARGET)
