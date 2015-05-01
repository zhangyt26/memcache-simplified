
CC = gcc

FLAGS =  -g -O2

LIBS = -lpthread

server: server.o
	${CC} -o server server.o ${LIBS}

server.o: server.c
	${CC} ${FLAGS} -c server.c

clean:
	rm server