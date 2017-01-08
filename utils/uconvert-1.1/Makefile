# User Convert Makefile. Don't change unless you know what you're doing..
# even though I still don't and am somewhat new at Makefile's.. just
# this one works.. so well.. why bother changing it.

EXEC         = uconvert
CC           = gcc
OBJ_FLAG     = -O2 -Wall -c
INCLUDE_FLAG = -I/usr/include/ -I/usr/local/include/ -I./ -I../../src/include/
EXEC_FLAG    = -O2 -Wall -o $(EXEC)
OBJECTS      = formats.o uconvert.o

TALKER_FLAG  = -DRAMTITS113

all: $(EXEC)

$(EXEC):	$(OBJECTS) Makefile
	$(CC) $(EXEC_FLAG) $(OBJECTS)

$(OBJECTS):	Makefile
	$(CC) $(INCLUDE_FLAG) $(TALKER_FLAG) $(OBJ_FLAG) $*.c -o $*.o

clean:
	rm -f *.o core $(EXEC) *.log

re: clean all

strip:
	/usr/bin/strip $(EXEC)
