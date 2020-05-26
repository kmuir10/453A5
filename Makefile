CC 	= gcc

CFLAGS  = -Wall -g -I .

LD 	= gcc

LDFLAGS  = -Wall -g -L/home/pn-cs453/Given/Asgn2

PUBFILES =  minget.c minls.c

PROGS	= minget minls

MINGET  = minget.o 

MINLS = minls.o 

OBJS	= $(MINGET) $(MINLS)

all: 	$(PROGS)

allclean: clean
	@rm -f $(EXTRACLEAN)

clean:	
	rm -f $(OBJS) *~ TAGS

minget.o: minget.c
	gcc -Wall -fPIC -o minget minget.c

minls.o: minls.c
	gcc -Wall -fPIC -o minls minls.c
