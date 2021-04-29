all: myprime inter worker

inter: inter.o
	gcc inter.o -o inter

myprime: myprime.o
	gcc myprime.o -o myprime

worker: worker.o
	gcc worker.o -o worker -lm

inter.o: inter.c
	gcc -c inter.c

worker.o: worker.c
	gcc -c worker.c

myprime.o: myprime.c
	gcc -c myprime.c

clean:
	rm *o myprime worker inter