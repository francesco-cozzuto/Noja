
all: noja path.so io.so

noja: $(wildcard src/runtime/*.h src/runtime/*.c src/runtime/*/*.h src/runtime/*/*.c)
	gcc $(wildcard src/runtime/*.c src/runtime/*/*.c) -o noja -g -Wall -Wextra -lm -ldl -rdynamic


path.so: $(wildcard src/modules/path/*.h src/modules/path/*.c)
	gcc $(wildcard src/modules/path/*.c) -o path.so -shared -fpic -I./include

io.so: $(wildcard src/modules/io/*.h src/modules/io/*.c)
	gcc $(wildcard src/modules/io/*.c) -o io.so -shared -fpic -I./include