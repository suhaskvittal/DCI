IDIR=./include
CC=g++
CFLAGS=-I$(IDIR)

ODIR=./bin
CDIR=./src

_OBJ=main.o command_factory.o command_processing.o command.o file_command.o system_command.o network.o
OBJ=$(patsubst %,$(ODIR)/%,$(_OBJ))

_DEPS=command.h network.h node.h
DEPS=$(patsubst %,$(IDIR)/%,$(_DEPS))

$(ODIR)/%.o: $(CDIR)/%.cpp $(DEPS)
	$(CC) -std=c++11 -c -o $@ $< $(CFLAGS)

main: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
debug: $(OBJ)
	$(CC) -g -o $@ $^ $(CFLAGS)
.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~
