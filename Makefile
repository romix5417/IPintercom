DIR_INC = ./include
DIR_SRC = ./src
DIR_LIB = ./lib
DIR_OBJ = ./obj
DIR_TEST = ./test

SRC = $(wildcard ${DIR_SRC}/*.c ${DIR_LIB}/*.c)  
OBJ = $(patsubst %.c,${DIR_OBJ}/%.o,$(notdir ${SRC})) 

TARGET = IPintercom

BIN_TARGET = ${TARGET}

CC = gcc
CFLAGS = -g -Wall -std=c99 -I${DIR_INC}

${BIN_TARGET}:${OBJ}
	$(CC) $(OBJ)  -o $@

${DIR_OBJ}/%.o:${DIR_SRC}/%.c
	$(CC) $(CFLAGS) -c  $< -o $@

${DIR_OBJ}/%.o:${DIR_LIB}/%.c
	$(CC) $(CFLAGS) -c  $< -o $@

all:
	@echo $(SRC)
	@echo $(OBJ)
	@echo "end"

.PHONY:clean
clean:
	rm -rf IPintercom ${DIR_OBJ}/*.o 
