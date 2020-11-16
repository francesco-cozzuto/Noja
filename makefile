
all: noja path.so io.so docs

CXREF_FLAGS = 	-DHAVE_CONFIG_H\
				-DLOCALEDIR="/usr/local/share/locale"\
				-DAC_DOCDIR=/usr/local/share/doc/gnubg\
				-DAC_DATADIR=/usr/local/share\
				-DAC_PKGDATADIR=/usr/local/share/gnubg\
				-DG_DISABLE_ASSERT\
				-D_Float16=float\
				-D_Float16x=float\
				-D_Float32=float\
				-D_Float32x=float\
				-D_Float64=float\
				-D_Float64x=float\
				-D_Float128=float\
				-D_Float128x=float

docs:
	mkdir docs
	cxref $(wildcard src/*/*.c src/*/*/*.c src/*/*.h src/*/*/*.h) -Odocs $(CXREF_FLAGS) -html -I./include

noja: $(wildcard src/runtime/*.h src/runtime/*.c src/runtime/*/*.h src/runtime/*/*.c)
	gcc $(wildcard src/runtime/*.c src/runtime/*/*.c) -o noja -g -Wall -Wextra -lm -ldl -rdynamic


path.so: $(wildcard src/modules/path/*.h src/modules/path/*.c)
	gcc $(wildcard src/modules/path/*.c) -o path.so -shared -fpic -I./include

io.so: $(wildcard src/modules/io/*.h src/modules/io/*.c)
	gcc $(wildcard src/modules/io/*.c) -o io.so -shared -fpic -I./include

clean:
	rm -r docs