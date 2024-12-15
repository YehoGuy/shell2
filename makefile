all: myshell

myshell: myshell.o
	gcc -g -Wall -o myshell myshell.o
myshell.o: myshell.c
	gcc -g -Wall -c -o myshell.o myshell.c

.PHONY: clean

clean:
	rm -f *.o myshell looper