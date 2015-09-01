ifeq (${CROSS},pb)
TOOLCHAIN_DIR=../SDK_481
PATH:=$(CURDIR)/$(TOOLCHAIN_DIR)/libexec/gcc/arm-obreey-linux-gnueabi/4.8.1:$(PATH)
CC=$(TOOLCHAIN_DIR)/arm-obreey-linux-gnueabi/bin/gcc -I$(TOOLCHAIN_DIR)/lib/gcc/arm-obreey-linux-gnueabi/4.8.1/include
LDFLAGS += -L$(TOOLCHAIN_DIR)/lib/gcc/arm-obreey-linux-gnueabi/4.8.1
EXE=
endif

ifeq (${CROSS},win)
CC=i686-pc-mingw32-gcc -I/usr/local/cross-tools/i386-mingw32/include
LDFLAGS += -L/usr/local/cross-tools/i386-mingw32/lib
EXE=.exe
endif

SRC=pbtheme.c
PROG=pbtheme$(EXE)

all: $(PROG)

pbtheme$(EXE): $(SRC)
	rm -f $(PROG)
	$(CC) -g -o $@ $^ $(LDFLAGS) -lz -lstdc++ 
