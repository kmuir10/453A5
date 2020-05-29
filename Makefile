CC 	= gcc

CFLAGS  = -Wall -g -I .

LD 	= gcc

LDFLAGS  = -Wall -g -L/home/pn-cs453/Given/Asgn2

PUBFILES =  minget.c minls.c

PROGS	= minget minls

MINGETO  = minget.o 

MINLSO = minls.o 

MINTOOLO = mintool.o

OBJS	= $(MINGETO) $(MINLSO) $(MINTOOLO)

SRCS	= minget.c minls.c

EXTRACLEAN = core $(PROGS)

all: 	$(PROGS)

allclean: clean
	@rm -f $(EXTRACLEAN)

clean:	
	rm -f $(OBJS) $(PROGS) *~ TAGS

minget: minget.o mintool.o
	gcc -Wall -fPIC -o minget mintool.o minget.o

minls: minls.o mintool.o
	gcc -Wall -fPIC -o minls mintool.o minls.o

minls.o: minls.c mintool.o
	gcc -Wall -fPIC -c minls.c 

minget.o: minget.c mintool.o
	gcc -Wall -fPIC -c minget.c

mintool.o: mintool.c mintool.h
	gcc -Wall -fPIC -c mintool.c
