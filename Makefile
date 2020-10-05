CC			=  gcc
AR          =  ar
DEBUGFLAGS	+= -std=c99 -Wall -pedantic -g -fsanitize=address -DMAKE_VALGRIND_HAPPY -D_GNU_SOURCE
CFLAGS		+= -std=c99 -Wall -pedantic -g -DMAKE_VALGRIND_HAPPY -D_GNU_SOURCE
ARFLAGS     =  rvs
INCLUDES	= -I.
LDFLAGS 	= -L.
OPTFLAGS	= -O3
LIB 		= -lpinger

TARGETS = pinger

OBJETCS = ping_list.o \
			chout.o

INCLUDE_FILES = chout.h \
				ping_list.h

.PHONY: clean exec cleanall

%: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) -o $@ $< $(LDFLAGS) 

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) -c -o $@ $<

pinger: main.o libpinger.a $(INCLUDE_FILES)
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

libpinger.a: $(OBJETCS)
	$(AR) $(ARFLAGS) $@ $^

clean:
	rm -f *.a *.o

cleanall:
	rm -f *.a *.o pinger