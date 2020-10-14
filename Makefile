CC			=  gcc
AR          =  ar
DEBUGFLAGS	+= -std=c99 -Wall -pedantic -g -DMAKE_VALGRIND_HAPPY -D_GNU_SOURCE
CFLAGS		+= -std=c99 -Wall -pedantic -DMAKE_VALGRIND_HAPPY -D_GNU_SOURCE
ARFLAGS     =  rvs
INCLUDES	= -I./phdr
LDFLAGS 	= -L.
OPTFLAGS	= -O3
LIBS		= -lpinger -lutils

TARGETS = pinger

OBJETCS = ping_list.o \
			pscheck.o

UTILS = chout.o \
		string_utils.o \
		htable.o \
		configuration.o

INCLUDE_FILES = phdr/chout.h \
				phdr/ping_list.h \
				phdr/string_utils.h \
				phdr/htable.h \
				phdr/configuration.h \
				phdr/pscheck.h

.PHONY: clean exec cleanall

#%: %.c
#	echo "Building $@"
#	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) -o $@ $< $(LDFLAGS) 

%.o: pimpl/%.c
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) -c -o $@ $<


pinger: main.o libpinger.a libutils.a $(INCLUDE_FILES)
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)


pingmock: pingmock.c
	$(CC) $(DEBUGFLAGS) $< -o $@


libpinger.a: $(OBJETCS)
	$(AR) $(ARFLAGS) $@ $^

libutils.a: $(UTILS)
	$(AR) $(ARFLAGS) $@ $^

clean:
	rm -f *.a *.o

cleanall:
	rm -f *.a *.o pinger