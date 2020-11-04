
all: a.out

a.out: $(wildcard src/*.h src/*.c)
	gcc $(wildcard src/*.c) -g -Wall -Wextra