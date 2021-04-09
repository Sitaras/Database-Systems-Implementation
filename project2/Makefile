INCLUDE=./include
LIB=./lib
EXAMPLES=./examples
SRC=./src
BUILD=./build

CC=gcc
CFLAGS=-g -Wall -I $(INCLUDE) -static

OBJ1=$(EXAMPLES)/main_SHT.o $(SRC)/HT.o  $(SRC)/SHT.o $(LIB)/BF_64.a

EXEC1=$(BUILD)/demoSHT

all: $(EXEC1) $(EXEC2)

$(EXEC1): $(OBJ1)
	$(CC) $(CFLAGS) $(OBJ1) -o $(EXEC1)

.PHONY: clean

clean:
	rm -f $(EXAMPLES)/main_SHT.o $(EXAMPLES)/main_HT.o $(SRC)/HT.o $(EXEC1)  $(SRC)/SHT.o  $(BUILD)/ask2_HT $(BUILD)/ask2_SHT
