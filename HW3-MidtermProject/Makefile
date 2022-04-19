mid: src/determinant.c src/get_num.c src/error_functions.c src/linkedList/linkedList.c src/client.c src/serverZ.c src/server.c src/workerY.c 
	
	gcc -c -g src/determinant.c src/get_num.c src/error_functions.c src/linkedList/linkedList.c src/client.c src/serverZ.c src/server.c src/workerY.c -lpthread -lrt
	gcc  determinant.o get_num.o error_functions.o serverZ.o -o serverZ -lpthread -lrt
	gcc  get_num.o error_functions.o linkedList.o client.o -o client -lpthread -lrt
	gcc  get_num.o error_functions.o server.o -o serverY -lpthread -lrt
	gcc  determinant.o get_num.o error_functions.o workerY.o -o workerY -lpthread -lrt
	rm *.o