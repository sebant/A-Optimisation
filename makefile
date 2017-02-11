CC=gcc
OBJECTS = llist readmap astar
LIBS=-include nireheads.h
LLIST=-include llist.h
HEURISTICS=-include equirectangular.h
readmapLIBS=$(LIBS) $(LLIST)
astarLIBS=$(LIBS) $(LLIST) $(HEURISTICS)
CFLAGS= -g -Ofast -std=gnu11

all: $(OBJECTS)

rline: rline.c
	$(CC) $(CFLAGS) rline.c -o rline
llist: llist_test.c llist.c
	$(CC) $(LIBS) $(LLIST) $(CFLAGS)  llist_test.c llist.c -o llist
astar: astar.c llist.c equirectangular.c
	$(CC) $(astarLIBS) $(CFLAGS)   astar.c llist.c equirectangular.c -o astar -lm
readmap: readmap.c llist.c
	$(CC) $(readmapLIBS) $(CFLAGS) readmap.c llist.c -o readmap
binsearch: binsearch.c
	$(CC) $(LIBS) $(CFLAGS) binsearch.c -o binsearch
.PHONY: clean
clean:
	rm $(OBJECTS)
iotest: iotest.c 
	$(CC) $(LIBS) $(CFLAGS) iotest.c -o iotest
ioread: ioreeead.c 
	$(CC) $(LIBS) $(CFLAGS) ioreeead.c -o ioread
