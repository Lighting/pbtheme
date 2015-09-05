SRC=pbtheme.c
PROG=pbtheme$(EXE)
STRIP=strip

ifeq (${CROSS},pb)
TOOLCHAIN_DIR=SDK_481
CC=$(TOOLCHAIN_DIR)/bin/arm-obreey-linux-gnueabi-gcc -I$(TOOLCHAIN_DIR)/include/c++/4.1.2 -I$(TOOLCHAIN_DIR)/include -I$(TOOLCHAIN_DIR)/arm-obreey-linux-gnueabi/sysroot/usr/include
LDFLAGS += -L$(TOOLCHAIN_DIR)/arm-obreey-linux-gnueabi/sysroot/usr/lib
STRIP=$(TOOLCHAIN_DIR)/bin/arm-obreey-linux-gnueabi-strip
EXE=
endif

ifeq (${CROSS},win)
CC=i686-pc-mingw32-gcc -I/usr/local/cross-tools/i386-mingw32/include
LDFLAGS += -L/usr/local/cross-tools/i386-mingw32/lib
STRIP=/usr/local/mingw/i586-mingw32-strip
EXE=.exe
endif

all: $(PROG)

pbtheme$(EXE): $(SRC)
	$(CC) -g -o $@ $^ $(LDFLAGS) -lz -lstdc++ 
	$(STRIP) $@
