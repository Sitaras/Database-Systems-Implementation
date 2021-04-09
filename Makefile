INCLUDE=./include
LIB=./lib
EXAMPLES=./examples
SRC=./src
BUILD=./build

CC=gcc
CFLAGS=-g -Wall -I $(INCLUDE) -static

OBJ1=$(EXAMPLES)/main_HP.o $(SRC)/HP.o $(LIB)/BF_64.a
OBJ2=$(EXAMPLES)/main_HT.o $(SRC)/HT.o $(LIB)/BF_64.a

EXEC1=$(BUILD)/demoHP
EXEC2=$(BUILD)/demoHT

all: $(EXEC1) $(EXEC2)

$(EXEC1): $(OBJ1)
	$(CC) $(CFLAGS) $(OBJ1) -o $(EXEC1)
$(EXEC2): $(OBJ2)
	$(CC) $(CFLAGS) $(OBJ2) -o $(EXEC2)


.PHONY: clean

clean:
	rm -f $(EXAMPLES)/main_HT.o $(EXAMPLES)/main_HP.o $(SRC)/HP.o $(SRC)/HT.o $(EXEC1) $(EXEC2)
