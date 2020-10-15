CC			=  gcc
AR          =  ar
DEBUGFLAGS	+= -std=c99 -Wall -pedantic -g -DMAKE_VALGRIND_HAPPY -D_GNU_SOURCE
CFLAGS		+= -std=c99 -Wall -pedantic -DMAKE_VALGRIND_HAPPY -D_GNU_SOURCE
ARFLAGS     =  rvs
INCLUDES	= -I./lib
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

INCLUDE_FILES = lib/chout.h \
				lib/ping_list.h \
				lib/string_utils.h \
				lib/htable.h \
				lib/configuration.h \
				lib/pscheck.h

.PHONY: clean exec cleanall

#%: %.c
#	echo "Building $@"
#	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) -o $@ $< $(LDFLAGS) 

%.o: lib/%.c
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