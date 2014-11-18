BIN=bin
SRC=src

all: $(BIN)/fdcounter $(BIN)/decho

$(BIN)/fdcounter : $(SRC)/fdcounter.c
	cc -o $@ $(SRC)/fdcounter.c

$(BIN)/decho : $(SRC)/decho.c
	cc -o $@ $(SRC)/decho.c

clean:
	rm -f $(BIN)/*

.PHONY: all clean
