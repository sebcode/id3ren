CFLAGSDEBUG = -g -O2 -Wall -DDEBUG
CFLAGS = -s -O2 -Wall
CC = gcc
RM = rm -f
INSTALL = install -s -m 755
INSTALL_DIR = /usr/bin
INSTALL_NAME = id3ren

SOURCES = id3ren.c id3tag.c id3file.c id3misc.c
INCLUDES = id3ren.h id3tag.h id3file.h id3misc.h id3genre.h

all:	${INSTALL_NAME}

${INSTALL_NAME}: ${SOURCES} ${INCLUDES}
	${CC} ${CFLAGS} -o ${INSTALL_NAME} ${SOURCES}

debug:
	${CC} ${CFLAGSDEBUG} -o ${INSTALL_NAME} ${SOURCES}

install: ${INSTALL_NAME}
	${INSTALL} ${INSTALL_NAME} ${INSTALL_DIR}/${INSTALL_NAME}

clean:
	${RM} ${INSTALL_NAME} *.o *.bak core

