# Makefile to compile each edge program. Since each program is small and
# executed individually, we can just compile them all without generating any
# object files.

SDIR=src
CC=gcc
BLOCKDIR=blocks
DEPS=include

objects = serverM serverA serverB serverC clientA clientB

all: clean $(objects)

$(objects): %: $(SDIR)/%.c
	$(CC) -o $@ $< -I$(DEPS)
   
.PHONY: clean

clean:
	rm -f $(objects); cp $(BLOCKDIR)/* $(SDIR)/
