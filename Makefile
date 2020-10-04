CC =	gcc
CCFLAGS += -Wall
OPTIONFLAGS = -L. -lpinglist

TARGETS= somma

.PHONY: clean exec cleanall

pinger: main.c libpinglist.a
	$(CC) -c main.c -o pinger.o
	$(CC) $(CCFLAGS) -o pinger pinger.o -L. -lpinglist

libpinglist.a: ping_list.o
	ar rcs libpinglist.a ping_list.o

ping_list.o: ping_list.c ping_list.h
	$(CC) -c ping_list.c -o ping_list.o

clean:
	rm -f libpinglist.a ping_list.o

cleanall:
	rm -f libpinglist.a ping_list.o pinger