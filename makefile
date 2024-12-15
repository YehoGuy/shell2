all: myshell mypipeline

myshell: myshell.o LineParser.o
	gcc -g -Wall -o myshell myshell.o LineParser.o
myshell.o: myshell.c LineParser.h
	gcc -g -Wall -c -o myshell.o myshell.c

LineParser.o: LineParser.c LineParser.h
	gcc -g -Wall -c -o LineParser.o LineParser.c

mypipeline: mypipeline.o
	gcc -g -Wall -o mypipeline mypipeline.o
mypipeline.o: mypipeline.c
	gcc -g -Wall -c -o mypipeline.o mypipeline.c

.PHONY: clean

clean:
	rm -f *.o mypipeline myshell