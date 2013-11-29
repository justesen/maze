# Edit prefixes as you prefer (defaults should be alright)
PREFIX = /usr/local
BINPREFIX = ${PREFIX}/bin
MANPREFIX  = ${PREFIX}/share/man

VERSION = 0.2

CFLAGS = -Wall -std=c90 -pedantic -I /usr/include/cairo
LDFLAGS = -L /usr/lib -lcairo
CC = cc

all: maze

maze: maze.o
	cc maze.o ${LDFLAGS} -o maze

maze.o: maze.c

clean:
	@echo cleaning
	@rm -f maze maze.o maze-${VERSION}.tar.gz

devel: all
	@rm -f maze.png solve.png maze.o maze-${VERSION}.tar.gz

dist: clean
	@echo creating distribution tarball
	@mkdir -p maze-${VERSION}
	@cp -r maze.c maze.1 Makefile README LICENSE maze-${VERSION}
	@tar -cf maze-${VERSION}.tar maze-${VERSION}
	@gzip maze-${VERSION}.tar
	@rm -rf maze-${VERSION}

install:
	@echo installing executable file to ${PREFIX}/bin
	@mkdir -p ${BINPREFIX}
	@cp -f maze ${BINPREFIX}
	@chmod 755 ${BINPREFIX}/maze
	@echo installing manual page to ${MANPREFIX}/man1
	@mkdir -p ${MANPREFIX}/man1
	@cp -f maze.1 ${MANPREFIX}/man1/maze.1
	@chmod 644 ${MANPREFIX}/man1/maze.1

uninstall:
	@echo removing executable file from ${BINPREFIX}
	@rm -f ${BINPREFIX}/maze
