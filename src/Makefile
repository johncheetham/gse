#
# gse makefile based on GNU shogi makefile
#

# Installation directory.
prefix  =   /usr/local

# Where the language description, the book, and the 
# persistent hashtable live.
LIBDIR  =   $(prefix)/lib/gnushogi

# For _pow external 
LIBS    = -lm

#
# C compiler and compiler options.
#

CC          = gcc
CWARNINGS   = -Wall -Wno-implicit-int -Wstrict-prototypes -ansi -pedantic
CEXTRAFLAGS = -fsigned-char -funroll-loops $(CEXTFLAGS2) $(CEXTFLAGS3)
CEXTFLAGS2 = $(HASH) $(FCNTL_H) $(BCOPY) $(ERRNO) $(MEMCPY)
CEXTFLAGS3 = $(MEMSET) $(LBUF) $(UNISTD)
CFLAGS      = -O2 $(CEXTRAFLAGS) $(CWARNINGS) -I. -pthread

# The hashfile is a record of positions seen. It is used by
# GNU Shogi to avoid making the same mistakes, a form of learning.

HASH       = -DHASHFILE=\"$(LIBDIR)/gnushogi.hsh\"

FCNTL_H = -DHAVE_FCNTL_H=1
BCOPY = -DHAVE_BCOPY=1
ERRNO = -DHAVE_ERRNO_H=1
MEMCPY = -DHAVE_MEMCPY=1
MEMSET = -DHAVE_MEMSET=1
LBUF = -DHAVE_SETLINEBUF=1
UNISTD = -DHAVE_UNISTD_H=1

# The "book" is a record of the first few moves, for playing good
# moves easily and quickly, saving time, and irritating the human
# opponent.

TEXTBOOK   = -DBOOK=\"$(LIBDIR)/gnushogi.tbk\"
BINBOOK    = -DBINBOOK=\"gse.bbk\"


# The language file describes capabilities of the program.  It may
# be useful for customizing gnushogi for non-English-speaking players.

LANGF      = -DLANGFILE=\"gnushogi.lng\"


# The pattern file contains various opening patterns. The program tries to
# obtain such a pattern in the opening stage. Sequences of opening
# patterns may be described in order to support the correct order of moves.

PATTERN    = -DPATTERNFILE=\"../misc/gnushogi.pat\"

all: gse gnushogi.bbk

	
#
# Source files.
#

COMMONFILES = \
                attacks.o      \
                book.o         \
                commondsp.o    \
                eval.o         \
                genmove.o      \
                globals.o      \
                init.o         \
                pattern.o      \
                rawdsp.o       \
                search.o       \
                sysdeps.o      \
                tcontrl.o      \
                util.o

GSE_FILES  = $(COMMONFILES) gse.o      


PAT2INCFILES    = $(COMMONFILES) makepattern.o pat2inc.o


#
# Primary targets.
#

gse:
	$(CC) $(CFLAGS) -o gse $(GSE_FILES) $(LIBS)

#
# Object files.
#

# Common files.

attacks.o:
	$(CC) $(CFLAGS) -c attacks.c

book.o:
	$(CC) $(CFLAGS) $(TEXTBOOK) $(BINBOOK) -c book.c 

commondsp.o:
	$(CC) $(CFLAGS) -c commondsp.c

genmove.o:
	$(CC) $(CFLAGS) -c genmove.c

globals.o:
	$(CC) $(CFLAGS) -c globals.c

eval.o:
	$(CC) $(CFLAGS) -c eval.c

init.o:
	$(CC)  $(CFLAGS) $(LANGF) -c init.c

gse.o:
	$(CC) $(CFLAGS) $(BINBOOK) -c gse.c

# To create "pattern.inc" with "pat2inc", the external
# pattern textfile must be used.

makepattern.o:
	$(CC) $(CFLAGS) $(PATTERN) -c makepattern.c

pattern.o:
	$(CC)  $(CFLAGS) -c pattern.c

rawdsp.o:
	$(CC) $(CFLAGS) -c rawdsp.c 

search.o:
	$(CC) $(CFLAGS) -c search.c

sysdeps.o:
	$(CC) $(CFLAGS) -c sysdeps.c 

tcontrl.o:
	$(CC) $(CFLAGS) -c tcontrl.c

util.o:
	$(CC) $(CFLAGS) -c util.c

#
# Other targets.
#

gnushogi.bbk:	 
	@if [ -f ./gse.bbk ]; then rm ./gse.bbk; fi	
	cat ../data/gnushogi.tbk > _tmp_t
	./gse -g _tmp_t gse.bbk 8000 40	
	rm _tmp_t
	@echo

#
# Cleanup.
#

CLEANFILES = *.o gse *.bbk

clean: 
	@for file in $(CLEANFILES); do \
	if [ -f $$file ]; then rm $$file; fi; \
	done

#
# Dependencies.
#

gse:           $(GSE_FILES)
pat2inc:       $(PAT2INCFILES)
sizetest:      sizetest.o 
attacks.o:     attacks.c gnushogi.h version.h
book.o:        book.c gnushogi.h version.h 
commondsp.o:   commondsp.c gnushogi.h version.h
genmove.o:     genmove.c gnushogi.h version.h
globals.o:     globals.c gnushogi.h version.h
eval.o:        eval.c eval.h gnushogi.h version.h pattern.h
init.o:        init.c gnushogi.h version.h pattern.h
gse.o:         gse.c gnushogi.h version.h
makepattern.o: pattern.c gnushogi.h pattern.h
pattern.o:     pattern.c gnushogi.h pattern.h pattern.inc
rawdsp.o:      rawdsp.c gnushogi.h version.h
search.o:      search.c gnushogi.h version.h 
sysdeps.o:     sysdeps.c gnushogi.h version.h
tcontrl.o:     tcontrl.c gnushogi.h version.h 
util.o:        util.c gnushogi.h version.h
pat2inc.o:     pat2inc.c pattern.h gnushogi.h 
sizetest.o:    sizetest.c gnushogi.h eval.h
gnushogi.bbk:  gse

