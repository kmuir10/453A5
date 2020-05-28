CC 	= gcc

CFLAGS  = -Wall -g -I .

LD 	= gcc

LDFLAGS  = -Wall -g -L/home/pn-cs453/Given/Asgn2

PUBFILES =  minget.c minls.c

PROGS	= minget minls

MINGET  = minget.o 

MINLS = minls.o 

OBJS	= $(MINGET) $(MINLS)

SRCS	= minget.c minls.c

EXTRACLEAN = core $(PROGS)

all: 	$(PROGS)

allclean: clean
	@rm -f $(EXTRACLEAN)

clean:	
	rm -f $(OBJS) $(PROGS) *~ TAGS

minget.o: minget.c
	gcc -Wall -fPIC -c minget.c

minls.o: minls.c
	gcc -Wall -fPIC -c minls.c
