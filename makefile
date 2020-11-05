
all: a.out

a.out: $(wildcard src/*.h src/*.c src/*/*.h src/*/*.c)
	gcc $(wildcard src/*.c src/*/*.c) -g -Wall -Wextra -lm