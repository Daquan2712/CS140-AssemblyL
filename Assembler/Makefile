assembler: main.o Assembler.o map.o
	gcc -o assembler main.o Assembler.o
Assembler.o: Assembler.c
	gcc -c Assembler.c
main.o: main.c
	gcc -c main.c
map.o: map.c
    gcc -c map.c
clean:
	rm -f *~ *.o *.a