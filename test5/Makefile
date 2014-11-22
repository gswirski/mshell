BIN=bin
SRC=src

all: $(BIN)/testerOX

$(BIN)/testerOX : $(SRC)/testerOX.c
	cc -o $@ $(SRC)/testerOX.c

clean:
	rm -f $(BIN)/*

.PHONY: all clean
