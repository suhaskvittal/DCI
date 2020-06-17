IDIR=./include
CC=g++
CFLAGS=-I$(IDIR)

ODIR=./bin
CDIR=./src

_OBJ=main.o command_factory.o command_processing.o help_command.o invalid_command.o msg_command.o auth_command.o file_command.o system_command.o network.o alias_command.o dci_utils.o
OBJ=$(patsubst %,$(ODIR)/%,$(_OBJ))

_DEPS=command.h auth_command.h help_command.h msg_command.h system_command.h file_command.h network.h node.h alias_command.h dci_utils.h
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
