
CC = gcc

FLAGS =  -g -O2

LIBS = -lpthread

server: server.o hashmap.o
	${CC} -o server server.o hashmap.o ${LIBS}

server.o: server.c
	${CC} ${FLAGS} -c server.c

hashmap.o: hashmap.c hashmap.h
	${CC} ${FLAGS} -c hashmap.c

clean:
	rm server