PROG = casim
CC = g++
CPPFLAGS = -W -Wall -g -I/usr/local/include
LDFLAGS =  -Wl,--copy-dt-needed-entries -lGLU -lglut -L/usr/local/lib -llua
OBJS = main.o

$(PROG) : $(OBJS)
	$(CC) -o $(PROG) $(OBJS) $(LDFLAGS)
main.o :
	$(CC) $(CPPFLAGS) -c main.cpp

clean:
	rm -f core $(PROG) $(OBJS)
