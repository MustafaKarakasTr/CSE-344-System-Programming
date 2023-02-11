CXX = gcc
LDFLAGS =  -Wall -g -pthread

SRC_servant =  src/servant.c src/Date.c
SRC_server =  src/server.c  src/my_queue.c src/threadHelper.c
SRC_client =  src/client.c src/threadHelper.c src/Date.c

OBJ_servant = $(SRC_servant:.cc=.o)
OBJ_server = $(SRC_server:.cc=.o)
OBJ_client = $(SRC_client:.cc=.o)

EXEC_servant = servant
EXEC_server = server
EXEC_client = client

all: $(EXEC_servant) $(EXEC_server) $(EXEC_client)  

$(EXEC_servant): $(OBJ_servant)
	$(CXX) $(LDFLAGS) -o $@ $(OBJ_servant) $(LBLIBS)
$(EXEC_server): $(OBJ_server)
	$(CXX) $(LDFLAGS) -o $@ $(OBJ_server) $(LBLIBS)
$(EXEC_client): $(OBJ_client)
	$(CXX) $(LDFLAGS) -o $@ $(OBJ_client) $(LBLIBS)

clean:
	rm servant 
	rm server
	rm client