main: main.c multiplicarMatriz.o
	gcc main.c multiplicarMatriz.o -o main

multiplicarMatriz.o: multiplicarMatriz.asm
	nasm -f elf64 multiplicarMatriz.asm -o multiplicarMatriz.o