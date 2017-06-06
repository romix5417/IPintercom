DIR_INC = ./include
DIR_SRC = ./src
DIR_LIB = ./lib
DIR_AUD = ${DIR_SRC}/audio
DIR_CARD = ${DIR_AUD}/card
DIR_CLI = ${DIR_SRC}/client
DIR_BUTTON = ${DIR_CLI}/button
DIR_TRS = ${DIR_SRC}/transport
DIR_LOG = ${DIR_LIB}/log
DIR_SOCKET = ${DIR_LIB}/socket
DIR_OBJ = ./obj
DIR_TEST = ./test
INSTALL_PROGRAM = cp


SRC = $(wildcard ${DIR_SRC}/*.c ${DIR_LIB}/*.c ${DIR_AUD}/*.c ${DIR_CLI}/*.c ${DIR_TRS}/*.c ${DIR_LOG}/*.c ${DIR_SOCKET}/*.c ${DIR_BUTTON}/*.c ${DIR_CARD}/*.c)
OBJ = $(patsubst %.c,${DIR_OBJ}/%.o,$(notdir ${SRC}))

TARGET = IPinterCom

BIN_TARGET = ${TARGET}

CROSS_TOOLCHAIN = arm-hisiv400-linux-

ifdef CROSS_TOOLCHAIN
CC = ${CROSS_TOOLCHAIN}gcc
else
CC = gcc
endif

ifndef DESTDIR
	DESTDIR = .
endif

CFLAGS += -g -Wall -std=c99 -I${DIR_INC}

${BIN_TARGET}:${OBJ}
	$(CC) $(OBJ) $(LDFLAGS) -o $@

#${DIR_OBJ}/%.o:${DIR_SRC}/%.c ${DIR_LIB}/%.c ${DIR_AUD}/%.c ${DIR_CLI}/%.c ${DIR_TRS}/%.c ${DIR_LOG}/%.c ${DIR_SOCKET}/%.c ${DIR_SOCKET}/%.c ${DIR_BUTTON}/%.c
#	$(CC) $(CFLAGS) $(LDFLAGS) -c  $< -o $@

${DIR_OBJ}/%.o:${DIR_SRC}/%.c
	$(CC) $(CFLAGS) $(LDFLAGS) -c  $< -o $@

${DIR_OBJ}/%.o:${DIR_LIB}/%.c
	$(CC) $(CFLAGS) $(LDFLAGS) -c  $< -o $@

${DIR_OBJ}/%.o:${DIR_AUD}/%.c
	$(CC) $(CFLAGS) $(LDFLAGS) -c $< -o $@

${DIR_OBJ}/%.o:${DIR_CLI}/%.c
	$(CC) $(CFLAGS) $(LDFLAGS) -c $< -o $@

${DIR_OBJ}/%.o:${DIR_TRS}/%.c
	$(CC) $(CFLAGS) $(LDFLAGS) -c $< -o $@

${DIR_OBJ}/%.o:${DIR_LOG}/%.c
	$(CC) $(CFLAGS) $(LDFLAGS) -c $< -o $@

${DIR_OBJ}/%.o:${DIR_SOCKET}/%.c
	$(CC) $(CFLAGS) $(LDFLAGS) -c $< -o $@

${DIR_OBJ}/%.o:${DIR_SOCKET}/%.c
	$(CC) $(CFLAGS) $(LDFLAGS) -c $< -o $@

${DIR_OBJ}/%.o:${DIR_BUTTON}/%.c
	$(CC) $(CFLAGS) $(LDFLAGS) -c $< -o $@

${DIR_OBJ}/%.o:${DIR_CARD}/%.c
	$(CC) $(CFLAGS) $(LDFLAGS) -c $< -o $@

all:${BIN_TARGET}
	@echo $(SRC)
	@echo $(OBJ)
	@echo "end"


.PHONY:install clean
install:
	@echo "Installing to $(DESTDIR)$(INSTALL_PREFIX)/bin"
	@$(INSTALL_PROGRAM) -rf $(BIN_TARGET) $(DESTDIR)$(INSTALL_PREFIX)/bin
clean:
	rm -rf IPinterCom ${DIR_OBJ}/*.o 
