BIN=bin
SRC=src

all: $(BIN)/catcher

$(BIN)/catcher : $(SRC)/catcher.c
	cc -o $@ $(SRC)/catcher.c

clean:
	rm -f $(BIN)/*

.PHONY: all clean
