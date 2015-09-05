SRC=pbtheme.c
PROG=pbtheme$(EXE)
STRIP=strip

ifeq (${CROSS},pb)
TOOLCHAIN_DIR=SDK_481
CC=$(TOOLCHAIN_DIR)/bin/arm-obreey-linux-gnueabi-gcc -I$(TOOLCHAIN_DIR)/include/c++/4.1.2 -I$(TOOLCHAIN_DIR)/include -I$(TOOLCHAIN_DIR)/arm-obreey-linux-gnueabi/sysroot/usr/include
LDFLAGS += -mcpu=cortex-a8 -march=armv7-a -mfpu=neon -mfloat-abi=softfp 
LDFLAGS += -DNDEBUG -s -fsigned-char -fomit-frame-pointer -fPIC -Wall -O2 -Wno-format-y2k
LDFLAGS += -L$(TOOLCHAIN_DIR)/arm-obreey-linux-gnueabi/sysroot/usr/lib
STRIP=$(TOOLCHAIN_DIR)/bin/arm-obreey-linux-gnueabi-strip
EXE=
endif

ifeq (${CROSS},win)
MINGW=/usr/i586-mingw32msvc
CC=i586-mingw32msvc-gcc -I$(MINGW)/include
STRIP=$(MINGW)/bin/strip
LDFLAGS += -L$(MINGW)/lib
EXE=.exe
endif

all: $(PROG)

pbtheme$(EXE): $(SRC)
	$(CC) -g -o $@ $^ $(LDFLAGS) -lz -lstdc++ 
	$(STRIP) $@
