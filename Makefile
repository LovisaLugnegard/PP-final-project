CC = mpicc
CCFLAGS = -03
CCGFLAGS = -g
LIBS = -lmpi -lm

BINS = quickSort

all: $(BINS)

quickSort: quickSort.c
	$(CC) $(CCGFLAGS) -o $@ $^ $(LIBS)

clean:
	$(RM) $(BINS)
