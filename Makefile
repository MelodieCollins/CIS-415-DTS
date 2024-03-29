CFLAGS=-W -Wall -pedantic -I/usr/local/include
LDFLAGS=-L/usr/local/lib
PROGRAMS=dtsv1 dtsv2 dtsv3 dtsv4
LIBRARIES=-lBXP -lpthread -lADTs

all: $(PROGRAMS)

dtsv1: dtsv1.o
	gcc $(LDFLAGS) -o $@ $^ $(LIBRARIES)

dtsv2: dtsv2.o
	gcc $(LDFLAGS) -o $@ $^ $(LIBRARIES)

dtsv3: dtsv3.o
	gcc $(LDFLAGS) -o $@ $^ $(LIBRARIES)

dtsv4: dtsv4.o
	gcc $(LDFLAGS) -o $@ $^ $(LIBRARIES)

dtsv1.o: dtsv1.c
dtsv2.o: dtsv2.c
dtsv3.o: dtsv3.c
dtsv4.o: dtsv4.c

clean:
	rm -f $(PROGRAMS) *.o
