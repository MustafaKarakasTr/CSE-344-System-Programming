all: src/myString.c src/replace_system.c src/main.c 
	gcc -Wall -g -c src/myString.c src/replace_system.c src/main.c
	gcc myString.o replace_system.o main.o -o hw1
