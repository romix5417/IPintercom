DIR_INC = ./include
DIR_SRC = ./src
DIR_LIB = ./lib
DIR_AUD = ${DIR_SRC}/audio
DIR_CLI = ${DIR_SRC}/client
DIR_TRS = ${DIR_SRC}/transport
DIR_LOG = ${DIR_LIB}/log
DIR_SOCKET = ${DIR_LIB}/socket/
DIR_OBJ = ./obj
DIR_TEST = ./test

SRC = $(wildcard ${DIR_SRC}/*.c ${DIR_LIB}/*.c ${DIR_AUD}/*.c ${DIR_CLI}/*.c ${DIR_TRS}/*.c)
OBJ = $(patsubst %.c,${DIR_OBJ}/%.o,$(notdir ${SRC})) 

TARGET = IPintercom

BIN_TARGET = ${TARGET}

CROSS_TOOLCHAIN = arm-hisiv400-linux-

CC = ${CROSS_TOOLCHAIN}gcc
CFLAGS = -g -Wall -std=c99 -I${DIR_INC}

${BIN_TARGET}:${OBJ}
	$(CC) $(OBJ)  -o $@

${DIR_OBJ}/%.o:${DIR_SRC}/%.c
	$(CC) $(CFLAGS) -c  $< -o $@

${DIR_OBJ}/%.o:${DIR_LIB}/%.c
	$(CC) $(CFLAGS) -c  $< -o $@

${DIR_OBJ}/%.o:${DIR_AUD}/%.c
	$(CC) $(CFLAGS) -c $< -o $@

${DIR_OBJ}/%.o:${DIR_CLI}/%.c
	$(CC) $(CFLAGS) -c $< -o $@

${DIR_OBJ}/%.o:${DIR_TRS}/%.c
	$(CC) $(CFLAGS) -c $< -o $@

${DIR_OBJ}/%.o:${DIR_LOG}/%.c
	$(CC) $(CFLAGS) -c $< -o $@

${DIR_OBJ}/%.o:${DIR_SOCKET}/%.c
	$(CC) $(CFLAGS) -c $< -o $@

${DIR_OBJ}/%.o:${DIR_SOCKET}/%.c
	$(CC) $(CFLAGS) -c $< -o $@

all:
	@echo $(SRC)
	@echo $(OBJ)
	@echo "end"

.PHONY:clean
clean:
	rm -rf IPintercom ${DIR_OBJ}/*.o 
