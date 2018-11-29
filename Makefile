PROGRAM       := tf
CROSS_COMPILE := $(CROSS_COMPILE)
CC            ?= gcc
CFLAGS        ?= -Wall -Wextra -std=c11
LDFLAGS       ?= -Wall -Wextra -std=c11

OBJS          := dbg.o colors.o utils.o termfill.o

$(PROGRAM): $(OBJS) 
	$(CC) -o $@ $(LDFLAGS) $^

%.o: %.c %.h defs.h
	$(CC) -o $@ -c $(CFLAGS) $<

clean:
	rm -f *~ $(PROGRAM) *.o
