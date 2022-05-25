hw3: named/src/*.c unnamed/src/*.c
	gcc -g -c -Wall named/src/shared_memory.c  named/src/pusher.c named/src/create_semaphore.c named/src/helper.c named/src/wholesaler.c named/src/chef.c 
	gcc shared_memory.o pusher.o -o pusher -lrt -pthread
	gcc shared_memory.o create_semaphore.o chef.o -o chef -lrt -pthread
	gcc shared_memory.o helper.o -o helper -lrt -pthread
	gcc shared_memory.o wholesaler.o -o hw3named -lrt -pthread

	gcc -g -c -Wall unnamed/src/shared_memoryU.c  unnamed/src/pusherU.c unnamed/src/helperU.c unnamed/src/wholesalerU.c unnamed/src/chefU.c 
	gcc shared_memoryU.o pusherU.o -o pusherU -lrt -pthread
	gcc shared_memoryU.o chefU.o -o chefU -lrt -pthread
	gcc shared_memoryU.o helperU.o -o helperU -lrt -pthread
	gcc shared_memoryU.o wholesalerU.o -o hw3unnamed -lrt -pthread