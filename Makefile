# Edit prefixes as you prefer (defaults should be alright)
PREFIX = /usr/local
BINPREFIX = ${PREFIX}/bin
MANPREFIX  = ${PREFIX}/share/man/man1

VERSION = 0.3

CFLAGS = -Wall -std=c90 -pedantic -I /usr/include/cairo -O3
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
	@echo installing manual page to ${MANPREFIX}
	@mkdir -p ${MANPREFIX}
	@cp -f maze.1 ${MANPREFIX}/maze.1
	@chmod 644 ${MANPREFIX}/maze.1

uninstall:
	@echo removing executable file from ${BINPREFIX}
	@rm -f ${BINPREFIX}/maze
	@echo removing man file from ${MANPREFIX}
	@rm -f ${MANPREFIX}/maze.1

