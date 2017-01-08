#----------------------------------------------------------------------------#
# DEBUGGING OPTIONS:
#----------------------------------------------------------------------------#
# Uncomment this option if you plan on using gdb to debug the talker.  Note
# uncommenting this line will cause the talker daemon to not fork into the
# background, this is so it'll work with gdb.

#DEBUGIT      = 1


# Uncomment this option if you have electric fence installed on your server.
# Electric fence is a library which when used in conjunction with GDB causes
# a program to crash whenever there is an invalid reference to something in
# memory. This is good for picking up bugs which otherwise might cause one
# to be baffled as to why their talker is crashing.

#USE_EFENCE   = 1


# Uncomment this if you want to experiment around with the ident daemon
# doing all sorts of debugging on it.

#DEBUG_IDENT  = 1


# If you're compiling your talker, and you did something wrong, and when
# it compiles it leaves a whole slew of errors, this will log the errors
# to srcfile.log. This helps track down where the beginning of the errors
# start.

#LOG_ERRORS   = 1


#----------------------------------------------------------------------------#
# MISC:
#----------------------------------------------------------------------------#
# If your server uses older kernel/crypt libraries, this line needs to be
# commented out.  Or if this line is commented out and you receive a reference
# like: "undefined reference to `crypt'"  you'll need to uncomment this line.

USE_LCRYPT   = 1


# If your server is slow on resolving IP addresses, uncomment this, and
# it'll try to quicken up the process by using a system call (/usr/bin/host)
# to find the alphanumeric site.

#USE_RESOLV   = 1

# If you're using solaris, and some flavours of unices, you'll need to
# uncomment the following two lines to add -lnsl -lsocket to the compile
# flag...

#USE_LNSL     = 1
#USE_LSOCKET  = 1

# Comment this if you want debugging information stored in. Note if you have
# this uncommented, and have DEBUGIT uncommented, DEBUGIT takes precedence
# of this.  Basically this just uses the 'strip' command (/usr/bin/strip) and
# gets rid of debugging information that isn't needed for normal use.

STRIPIT      = 1


# Don't touch.
SVERSION     = 2.2

#----------------------------------------------------------------------------#
# DO NOT EDIT BEYOND THIS POINT UNLESS YOU KNOW HOW MAKEFILES WORK.
# I WILL NOT BE RESPONSIBLE IF YOU DO NOT HEED THIS WARNING.
#----------------------------------------------------------------------------#
IDENT_EFLAG = -O2 -Wall ident_server.c -o $(IDENT_EXEC)
ifdef DEBUT_IDENT
  IDENT_EFLAG = -DDEBUG_IDENT -DDEBUG_IDENT_TOO -O2 -Wall ident_server.c -o $(IDENT_EXEC)
endif

OBJ_FLAG    = -O2 -Wall -c
LIBS        = 
ifdef DEBUGIT
  OBJ_FLAG += -g -DTDEBUG -pg
endif
ifdef DEBUG_IDENT
  OBJ_FLAG += -DDEBUG_IDENT -DDEBUG_IDENT_TOO
endif
ifdef USE_RESOLV
  OBJ_FLAG += -DRESOLVEIP
endif
ifdef USE_LCRYPT
  LIBS     += -lcrypt
endif
ifdef USE_EFENCE
  LIBS     += -lefence
endif
ifdef USE_LNSL
  LIBS     += -lnsl
endif
ifdef USE_LSOCKET
  LIBS     += -lsocket
endif

EXEC_FLAG   = -O2 -Wall -o $(EXEC)
ifdef DEBUGIT
  EXEC_FLAG = -O2 -Wall -g -DTDEBUG -pg -o $(EXEC)
endif

ifdef LOG_ERRORS
  LOGIT     = 2>$*.log
  LOGIT2    = 2>$(EXEC).log
else
  LOGIT     = 
  LOGIT2    = 
endif

OBJ_DIR     = objects
BIN_DIR     = ../bin
INC_DIR     = include
CC          = gcc
OBJECTS     = $(OBJ_DIR)/bootup.o $(OBJ_DIR)/casino.o $(OBJ_DIR)/command.o \
              $(OBJ_DIR)/config.o $(OBJ_DIR)/connect.o $(OBJ_DIR)/editor.o \
              $(OBJ_DIR)/figlet.o $(OBJ_DIR)/games.o $(OBJ_DIR)/general.o \
              $(OBJ_DIR)/main.o $(OBJ_DIR)/menus.o $(OBJ_DIR)/messages.o \
              $(OBJ_DIR)/module1.o $(OBJ_DIR)/module2.o $(OBJ_DIR)/module3.o \
              $(OBJ_DIR)/objects.o $(OBJ_DIR)/ramtits.o $(OBJ_DIR)/rooms.o \
              $(OBJ_DIR)/server.o $(OBJ_DIR)/speech.o $(OBJ_DIR)/staff.o \
              $(OBJ_DIR)/string.o $(OBJ_DIR)/checkers.o $(OBJ_DIR)/whohelp.o \
              $(OBJ_DIR)/write.o $(OBJ_DIR)/idclient.o $(OBJ_DIR)/chess.o \
              $(OBJ_DIR)/telnet.o

HEADERS     = $(INC_DIR)/commands.h $(INC_DIR)/custom1.h $(INC_DIR)/custom2.h \
              $(INC_DIR)/defines.h $(INC_DIR)/structs.h

ID_HEADERS  = $(INC_DIR)/ident.h

ifndef DEBUGIT
  ifdef STRIPIT
    all: $(EXEC) $(IDENT_EXEC) strip
  else
    all: $(EXEC) $(IDENT_EXEC)
  endif
else
  all: $(EXEC) $(IDENT_EXEC)
endif

$(IDENT_EXEC):	$(ID_HEADERS) ident_server.c Makefile
	$(CC) $(IDENT_EFLAG)

$(EXEC):	$(OBJECTS) Makefile
	$(CC) $(EXEC_FLAG) $(OBJECTS) $(LIBS) $(LOGIT2)

$(OBJ_DIR)/%.o: $(HEADERS) Makefile
	$(CC) $(OBJ_FLAG) $*.c -o $(OBJ_DIR)/$*.o $(LOGIT)

install: all
	@$(BASH) EXEC=$(EXEC) IDENT_EXEC=$(IDENT_EXEC) ./install

re: clean all

clean:
	rm -f $(OBJ_DIR)/*.o core $(EXEC) $(IDENT_EXEC) *.log $(BIN_DIR)/$(EXEC)* $(BIN_DIR)/$(IDENT_EXEC)*

depend:
	@printf " - Creating Makefile.dep\n"
	@( for i in *.c; do gcc -M $$i; done ) > .rawdep 2>/dev/null
	@sed -e '1,$$s/^[a-z]/$$(OBJ_DIR)\/&/' .rawdep > Makefile.dep
	@-rm -f .rawdep

depend_silent:
	@( for i in *.c; do gcc -M $$i; done ) > .rawdep 2>/dev/null
	@sed -e '1,$$s/^[a-z]/$$(OBJ_DIR)\/&/' .rawdep > Makefile.dep
	@-rm -f .rawdep

strip:
	@/usr/bin/strip $(EXEC)

delerrors:
	@-rm -f *.log

include Makefile.dep
