CC=gcc
CFLAGS=-Wall

CFILES = compile.c engine.c parser.c cflow.c assembly.c vector.c
OFILES = $(CFILES:.c=.o)

CIBLE  = compile

$(CIBLE): $(OFILES)
	$(CC) $(OFILES) ${LIBS} -o $(CIBLE)

main.o:     compile.c parser.h cflow.h assembly.h
engine.o:   engine.c engine.h
parser.o:   parser.c parser.h grammar.h engine.h
cflow.o:    cflow.c cflow.h parser.h vector.h
assembly.o: assembly.c assembly.h cflow.h vector.h
vector.o:   vector.c vector.h 

clean:
	rm -f compile compile.o engine.o parser.o cflow.o assembly.o vector.o
