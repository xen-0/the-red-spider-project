CC=g++
CFLAGS=-O3 -I../include -std=c++11

IDIR = ../include
ODIR = ../build
BDIR = ../bin

_DEPS = rsTypes.h rsTerminal.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = rsGameOfLife.o rsTerminal.o 
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

gameofspiders: $(OBJ)
	$(CC) -o $(BDIR)/$@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~d
