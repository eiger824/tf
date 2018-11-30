PROGRAM       := tf
OUTDIR        := bin
CROSS_COMPILE := $(CROSS_COMPILE)
CC            ?= gcc
CFLAGS        ?= -Wall -Wextra -std=c11 -g
LDFLAGS       ?= -Wall -Wextra -std=c11 -g

OBJS          := \
	$(OUTDIR)/dbg.o \
	$(OUTDIR)/colors.o \
	$(OUTDIR)/utils.o \
	$(OUTDIR)/termfill.o

all: $(OUTDIR) $(OUTDIR)/$(PROGRAM)

$(OUTDIR):
	test -d $(OUTDIR) || mkdir $(OUTDIR)

$(OUTDIR)/$(PROGRAM): $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $^
	@# Remove any symlinks
	rm -f tf
	# Make a symlink
	ln -s ./bin/tf

$(OUTDIR)/termfill.o: termfill.c defs.h
	$(CC) -o $@ -c $(CFLAGS) $<

$(OUTDIR)/%.o: %.c %.h defs.h
	$(CC) -o $@ -c $(CFLAGS) $<

clean:
	rm -rf *~ $(PROGRAM) *.o $(OUTDIR)
