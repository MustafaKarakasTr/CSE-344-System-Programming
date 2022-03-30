all: parentProcess.c childProgram.c linkedList/linkedList.c
	gcc -Wall -g -c linkedList/linkedList.c childProgram.c   parentProcess.c
	gcc childProgram.o -o childProgram
	gcc linkedList.o parentProcess.o -o processP -lm 
