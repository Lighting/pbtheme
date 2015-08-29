SRC=pbtheme.c
PROG=pbtheme$(EXE)

all: $(PROG)

pbtheme$(EXE): $(SRC)
	$(CC) -g -o $@ $^ $(LDFLAGS) -lz -lstdc++ 
