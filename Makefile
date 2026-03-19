CC = gcc
CFLAGS = -Wall -Wextra -O2 -g -w
LDFLAGS = -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lm

SRC = src/*.c
OUT = elc_compiler

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(OUT)

