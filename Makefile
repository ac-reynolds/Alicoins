# Makefile to compile each edge program. Since each program is small and
# executed individually, we can just compile them all without generating any
# object files.

SDIR=src
CC=gcc
BLOCKDIR=blocks
DEPS=include

objects = serverM serverA serverB serverC clientA clientB

all: clean $(objects)

blocks: FORCE
	cp $(BLOCKDIR)/* $(SDIR)/

copies: FORCE
	cp $(SDIR)/serverA.c $(SDIR)/serverB.c
	cp $(SDIR)/serverA.c $(SDIR)/serverC.c
	cp $(SDIR)/clientA.c $(SDIR)/clientB.c
	sed -i '1 s/A/B/' $(SDIR)/clientB.c
	sed -i '1 s/A/B/' $(SDIR)/serverB.c
	sed -i '1 s/A/C/' $(SDIR)/serverC.c

$(objects): %: $(SDIR)/%.c
	$(CC) -o $@ $< -I$(DEPS)
   
.PHONY: clean

clean:
	rm -f $(objects) $(SDIR)/alichain.txt

FORCE: ;
