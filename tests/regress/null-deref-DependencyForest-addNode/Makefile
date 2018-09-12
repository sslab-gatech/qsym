all: main

main: asm.o main.o
	gcc $^ -o $@

asm.o: asm.s
	as asm.s -o asm.o

main.o: main.c
	gcc -c main.c -o main.o

clean:
	rm -rf *.o main
