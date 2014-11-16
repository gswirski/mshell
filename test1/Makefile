BIN=bin
SRC=src

all: $(BIN)/splitter

$(BIN)/splitter : $(SRC)/splitter.c
	cc -o $@ $(SRC)/splitter.c

clean:
	rm -f $(BIN)/*

.PHONY: all clean
